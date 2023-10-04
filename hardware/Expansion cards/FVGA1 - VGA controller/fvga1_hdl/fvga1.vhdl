-- FVGA1 - VGA framebuffer card driver
--
-- Stuart Wallace, May 2017.
--

-- VGA timings
--
-- ----------------  -------- horizontal -------  -------- vertical ---------  -------
-- resolution  rfsh  sync   fp    bp    pix  pol  sync   fp    bp    pix  pol  clk/MHz
-- ----------------  ---------------------------  ---------------------------  -------
--  640 x  480  @60    96   16    48    640    0     2   10    33    480    0   25.175
--  800 x  600  @60   128   40    88    800    1     4    1    23    600    1   40.000
-- 1024 x  768  @60   136   24   160   1024    0     6    3    29    768    0   65.000
-- ----------------  ---------------------------  ---------------------------  -------

-- PLL configuration
--
-- 50MHz clock
-- ----------------  -------  ------------------  --f--  --2f-  --4f-
-- resolution  rfsh  clk/MHz  dR    fr   dF        dQ     dQ     dQ
-- ----------------  -------  ------------------  -----  -----  -----
--  640 x  480  @60   25.175  0100  001  1010000   101    100    011
--  800 x  600  @60   40.000  0100  001  0111111   100    011    010
-- 1024 x  768  @60   65.000  0100  001  1100111   100    011    010
-- ----------------  -------  ------------------  -----  -----  -----


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity fvga1 is
    generic(
        data_width      : integer := 16;                        -- ram chip data bus width
        addr_width      : integer := 18                         -- ram chip address bus width
    );

    port(
        SYSCLK          : in    std_logic;

        HOST_D          : inout std_logic_vector(data_width - 1 downto 0);
        HOST_A          : in    std_logic_vector(19 downto 1);              -- FIXME - hardwired

        HOST_ID,
        HOST_nUCS,
        HOST_nLCS,
        HOST_nW,
        HOST_RESET      : in    std_logic;
        HOST_nIRQ       : out   std_logic;

        MEM_D           : inout std_logic_vector(data_width - 1 downto 0);
        MEM_A           : out   std_logic_vector(addr_width - 1 downto 0);
        
        MEM_nCE1,
        MEM_nCE0,
        MEM_nUB,
        MEM_nLB,
        MEM_nOE,
        MEM_nWE         : out   std_logic;

        VGA_R,
        VGA_G           : out   std_logic_vector(2 downto 0);
        VGA_B           : out   std_logic_vector(1 downto 0);
        VGA_nVSYNC,
        VGA_nHSYNC      : out   std_logic
    );
end;


architecture behaviour of fvga1 is
    type host_state is (none, read_pending, write_pending, write_latched, done);

    -- 640x480
    constant vga_h_fp       : integer := 16;
    constant vga_h_bp       : integer := 48;
    constant vga_h_sync     : integer := 96;
    constant vga_h_pixels   : integer := 640;
    constant vga_h_sync_pol : std_logic := '0';

    constant vga_v_fp       : integer := 10;
    constant vga_v_bp       : integer := 33;
    constant vga_v_sync     : integer := 2;
    constant vga_v_pixels   : integer := 480;
    constant vga_v_sync_pol : std_logic := '0';

    -- 800x600
    -- constant vga_h_fp       : integer := 40;
    -- constant vga_h_bp       : integer := 88;
    -- constant vga_h_sync     : integer := 128;
    -- constant vga_h_pixels   : integer := 800;
    -- constant vga_h_sync_pol : std_logic := '1';

    -- constant vga_v_fp       : integer := 1;
    -- constant vga_v_bp       : integer := 23;
    -- constant vga_v_sync     : integer := 4;
    -- constant vga_v_pixels   : integer := 600;
    -- constant vga_v_sync_pol : std_logic := '1';

    -- 1024x768
    -- constant vga_h_fp       : integer := 24;
    -- constant vga_h_bp       : integer := 160;
    -- constant vga_h_sync     : integer := 136;
    -- constant vga_h_pixels   : integer := 1024;
    -- constant vga_h_sync_pol : std_logic := '0';

    -- constant vga_v_fp       : integer := 3;
    -- constant vga_v_bp       : integer := 29;
    -- constant vga_v_sync     : integer := 6;
    -- constant vga_v_pixels   : integer := 768;
    -- constant vga_v_sync_pol : std_logic := '0';

    constant device_id  : std_logic_vector(data_width - 1 downto 0) := x"8400";

    constant h_period   : integer := vga_h_sync + vga_h_bp + vga_h_pixels + vga_h_fp;
    constant v_period   : integer := vga_v_sync + vga_v_bp + vga_v_pixels + vga_v_fp;

    signal pixel,
           pixel_data   : std_logic_vector(data_width - 1 downto 0) := (others => '0');

    signal host_oe      : std_logic := '0';
    signal pix_oe       : std_logic := '0';

    signal clk_early    : std_logic := '0';
    signal clk_late     : std_logic := '0';

    signal pixel_addr   : std_logic_vector(18 downto 0) := (others => '0');
    signal pixel_clk    : std_logic := '0';

    signal host_d_r,
           host_d_w_id,
           host_d_w,
           mem_d_r,
           mem_d_w      : std_logic_vector(data_width - 1 downto 0);

    signal host_r_clk,
           host_w_clk,
           host_d_we,
           mem_d_we,
           host_r_oe    : std_logic := '0';
begin
    --
    -- Memory data bus registers
    --
    pix_r_ff: entity work.dff_oe_areset
        generic map(
            width       => data_width,
            pol_clk     => '0',
            pol_oe      => '1',
            pol_reset   => '1')
        port map(
            CLK         => clk_early,
            RESET       => HOST_RESET,
            OE          => '1',
            D           => mem_d_r,
            Q           => pixel_data);

    host_w_ff: entity work.dff_oe_areset
        generic map(
            width       => data_width,
            pol_clk     => '0',
            pol_oe      => '1',
            pol_reset   => '1')
        port map(
            CLK         => host_w_clk,
            RESET       => HOST_RESET,
            OE          => '1',
            D           => mem_d_r,
            Q           => host_d_w);

    host_r_ff: entity work.dff_oe_areset
        generic map(
            width       => data_width,
            pol_clk     => '0',
            pol_oe      => '1',
            pol_reset   => '1')
        port map(
            CLK         => host_r_clk,
            RESET       => HOST_RESET,
            OE          => host_r_oe,
            D           => host_d_r,
            Q           => mem_d_w);

    --
    -- Memory address bus registers
    --
    pix_a_ff: entity work.dff_oe_areset
        generic map(
            width       => addr_width,
            pol_clk     => '0',
            pol_oe      => '1',
            pol_reset   => '1')
        port map(
            CLK         => clk_late,
            RESET       => HOST_RESET,
            OE          => pix_oe,
            D           => pixel_addr(addr_width - 1 downto 0),
            Q           => MEM_A);

    host_a_ff: entity work.dff_oe_areset
        generic map(
            width       => addr_width,
            pol_clk     => '0',
            pol_oe      => '1',
            pol_reset   => '1')
        port map(
            CLK         => clk_early,
            RESET       => HOST_RESET,
            OE          => host_oe,
            D           => HOST_A(addr_width downto 1),
            Q           => MEM_A);

    --
    -- Host data bus (read mode) multiplexer
    --
    host_d_w_mux: entity work.mux_2ch_oe
        generic map(
            width       => data_width)
        port map(
            A           => host_d_w,
            B           => device_id,
            Q           => host_d_w_id,
            OE          => '1',
            SEL         => HOST_ID);

    --
    -- Host data bus - bidirectional
    --
    host_d_bidir: entity work.bidir_bus
        generic map(
            width       => data_width)
        port map(
            PIN         => HOST_D,
            READ        => host_d_r,
            WRITE       => host_d_w_id,
            ENABLE      => host_d_we);

    --
    -- Memory data bus - bidirectional
    --
    mem_d_bidir: entity work.bidir_bus
        generic map(
            width       => data_width)
        port map(
            PIN         => MEM_D,
            READ        => mem_d_r,
            WRITE       => mem_d_w,
            ENABLE      => mem_d_we);

    --
    -- PLL
    --
    pll_inst: entity work.pll
        generic map(
            -- assumes 50MHz sysclk
            pll_divr        => "0100",
            pll_fr          => "001",

            -- 640x480
            pll_divf        => "1010000",
            pll_divq        => "011"

            -- 800x600
            -- pll_divf        => "0111111",
            -- pll_divq        => "010"

            -- 1024x768
            -- pll_divf        => "1100111",
            -- pll_divq        => "010"
        )
        port map(
            REFERENCECLK    => SYSCLK,
            PLLOUTCORE      => pixel_clk,
            PLLOUTGLOBAL    => open,
            RESET           => '1');

    process(pixel_clk, HOST_RESET)
        variable state          : integer range 0 to 7 := 0;
        variable h_count        : integer range 0 to h_period - 1 := 0;
        variable v_count        : integer range 0 to v_period - 1 := 0;
        variable host_rq        : boolean := false;
        variable host_rq_done   : boolean := false;
    begin
        if(HOST_RESET = '1') then
            state           := 0;
            host_rq         := false;
            host_rq_done    := false;

            pixel_addr      <= (others => '0');
            pixel           <= (others => '0');

            mem_d_we        <= '0';
            host_d_we       <= '0';

            host_oe         <= '0';
            pix_oe          <= '0';
            host_r_oe       <= '0';
            host_r_clk      <= '0';
            host_w_clk      <= '0';

            clk_early       <= '0';
            clk_late        <= '0';

            h_count         := 0;
            v_count         := 0;

            MEM_nCE1        <= '1';
            MEM_nCE0        <= '1';
            MEM_nOE         <= '1';
            MEM_nWE         <= '1';
            MEM_nUB         <= '1';
            MEM_nLB         <= '1';

            VGA_R           <= (others => '0');
            VGA_G           <= (others => '0');
            VGA_B           <= (others => '0');
            VGA_nVSYNC      <= not vga_v_sync_pol;
            VGA_nHSYNC      <= not vga_h_sync_pol;

            HOST_nIRQ       <= '1';
        elsif(rising_edge(pixel_clk)) then
            state := state + 1;

            case state is
                when 2 =>   -- STATE 2: latch host->mem data, if any; trigger early clock
                    clk_early <= '1';

                    -- latch data written from host->mem, if a host cycle is in progress
                    if((host_rq = true) and (host_rq_done = false)) then
                        if((HOST_nW = '0') and (HOST_ID = '0')) then
                            host_r_clk <= '1';
                        end if;
                    end if;

                when 6 =>   -- STATE 6: complete host read, if any; trigger late clock
                    clk_late <= '1';

                    -- latch data read from mem->host, if a host read cycle is in progress;
                    -- mark any in-progress cycle as complete.
                    if((host_rq = true) and (host_rq_done = false)) then
                        if((HOST_nW = '1') and (HOST_ID = '0')) then
                            host_w_clk <= '1';
                        end if;
                        host_rq_done := true;
                    end if;

                when others =>
                    clk_early   <= '0';
                    clk_late    <= '0';
                    host_r_clk  <= '0';
                    host_w_clk  <= '0';
            end case;

            case state is
                when 0 =>   -- STATE 0: begin pixel read cycle; start/end host cycle; scan out upper pixel
                    host_oe     <= '0';         -- disable host->mem addr output
                    pix_oe      <= '1';         -- enable pix->mem addr output
                    host_r_oe   <= '0';         -- disable host->mem data output  [??? needed]
                    mem_d_we    <= '0';         -- set memory data bus to read mode

                    MEM_nCE0    <= pixel_addr(18);     -- } enable the appropriate RAM ic
                    MEM_nCE1    <= not pixel_addr(18); -- } 

                    MEM_nWE     <= '1';         -- } this is a read cycle
                    MEM_nOE     <= '0';         -- }
                    MEM_nUB     <= '0';         -- } this is a 16-bit cycle
                    MEM_nLB     <= '0';         -- }

                    -- handle start / end of host cycle
                    if((host_rq = false) and (host_rq_done = false) and ((HOST_nUCS = '0') or (HOST_nLCS = '0'))) then
                        host_d_we   <= HOST_nW;
                        host_rq     := true;
                    end if;
                    
                    if((HOST_nUCS = '1') and (HOST_nLCS = '1')) then
                        host_rq      := false;
                        host_rq_done := false;
                    end if;

                    -- scan out upper pixel
                    VGA_R <= pixel(15 downto 13);
                    VGA_G <= pixel(12 downto 10);
                    VGA_B <= pixel(9 downto 8);

                when 4 =>   -- STATE 4: begin host cycle, if requested; scan out lower pixel
                    host_oe     <= '1';         -- enable host->mem addr output
                    pix_oe      <= '0';         -- disable pix->mem addr output

                    if((host_rq = true) and (host_rq_done = false)) then
                        -- start host memory transaction
                        MEM_nCE0    <= HOST_A(19);
                        MEM_nCE1    <= not HOST_A(19);

                        MEM_nWE     <= HOST_nW;         -- set memory write strobe
                        MEM_nOE     <= not HOST_nW;     -- set memory read strobe

                        mem_d_we    <= not HOST_nW;     -- set memory data bus direction
                        host_r_oe   <= not HOST_nW;     -- enable/disable host->mem data output

                        MEM_nUB     <= HOST_nUCS;       -- enable/disable upper lane
                        MEM_nLB     <= HOST_nLCS;       -- enable/disable lower lane
                    else
                        -- no host memory transaction pending
                        MEM_nCE0    <= '1';             -- deselect RAM 0
                        MEM_nCE1    <= '1';             -- deselect RAM 1

                        MEM_nWE     <= '1';             -- de-assert memory write strobe
                        MEM_nOE     <= '1';             -- de-assert memory read strobe

                        mem_d_we    <= '0';             -- set memory data bus to read
                        host_r_oe   <= '0';             -- disable host->mem data output

                        MEM_nUB     <= '1';             -- disable upper lane
                        MEM_nLB     <= '1';             -- disable lower lane
                    end if;

                    -- scan out lower pixel
                    VGA_R <= pixel(7 downto 5);
                    VGA_G <= pixel(4 downto 2);
                    VGA_B <= pixel(1 downto 0);

                when 6 =>   -- STATE 6: transfer pixel data to scan-out register
                    pixel <= pixel_data;        -- THIS NEEDS TO BE REVIEWED <<<<<<<<<<<<<<<<<<------

                when 7 =>
                    MEM_nWE <= '1';     -- TODO this is really bad...

                when others =>
                    null;
            end case;

            -- Generate horizontal sync
            if((h_count >= (vga_h_pixels + vga_h_fp)) and
               (h_count <= (vga_h_pixels + vga_h_fp + vga_h_sync))) then
                VGA_nHSYNC <= vga_h_sync_pol;
            else
                VGA_nHSYNC <= not vga_h_sync_pol;
            end if;

            -- Generate vertical sync
            if((v_count >= (vga_v_pixels + vga_v_fp)) and
               (v_count <= (vga_v_pixels + vga_v_fp + vga_v_sync))) then
                VGA_nVSYNC <= vga_v_sync_pol;
            else
                VGA_nVSYNC <= not vga_v_sync_pol;
            end if;

            -- Update h/v counts and pixel address
            if((state = 0) or (state = 4)) then
                if(h_count < vga_h_pixels) then
                    h_count := h_count + 1;
                    if(state = 4) then
                        pixel_addr <= std_logic_vector(unsigned(pixel_addr) + 1);
                    end if;
                elsif(h_count < h_period) then
                    h_count := h_count + 1;
                else
                    h_count := 0;
                    if(v_count < vga_v_pixels) then
                        v_count := v_count + 1;
                        if(state = 4) then
                            pixel_addr <= std_logic_vector(unsigned(pixel_addr) + 1);
                        end if;
                    elsif(v_count < v_period) then
                        v_count := v_count + 1;
                    else
                        v_count := 0;
                        pixel_addr <= 0;
                    end if;
                end if;
            end if;
        end if;
    end process;
end behaviour;

