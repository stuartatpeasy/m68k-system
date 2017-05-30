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
    -- 640x480
    -- constant vga_h_fp       : integer := 16;
    -- constant vga_h_bp       : integer := 48;
    -- constant vga_h_sync     : integer := 96;
    -- constant vga_h_pixels   : integer := 640;
    -- constant vga_h_sync_pol : std_logic := '0';

    -- constant vga_v_fp       : integer := 10;
    -- constant vga_v_bp       : integer := 33;
    -- constant vga_v_sync     : integer := 2;
    -- constant vga_v_pixels   : integer := 480;
    -- constant vga_v_sync_pol : std_logic := '0';

    -- 800x600
    constant vga_h_fp       : integer := 40;
    constant vga_h_bp       : integer := 88;
    constant vga_h_sync     : integer := 128;
    constant vga_h_pixels   : integer := 800;
    constant vga_h_sync_pol : std_logic := '1';

    constant vga_v_fp       : integer := 1;
    constant vga_v_bp       : integer := 23;
    constant vga_v_sync     : integer := 4;
    constant vga_v_pixels   : integer := 600;
    constant vga_v_sync_pol : std_logic := '1';

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

    signal pixel        : std_logic_vector(data_width - 1 downto 0) := (others => '0');

    signal host_oe      : std_logic := '0';
    signal pix_oe       : std_logic := '0';

    signal clk_early    : std_logic := '0';
    signal clk_late     : std_logic := '0';

    signal pixel_addr   : std_logic_vector(18 downto 0) := (others => '0');
    signal pixel_clk    : std_logic := '0';

    signal host_d_r,
           host_d_w_id,
           host_d_w     : std_logic_vector(data_width - 1 downto 0);

    signal host_r_clk,
           host_w_clk,
           host_cycle,
           host_d_we,
           host_r_oe    : std_logic := '0';
begin
    host_w_clk <= '1' when ((clk_late = '1') and (HOST_nW = '1') and (HOST_ID = '0')) else '0';
    host_r_clk <= '1' when ((host_cycle = '1') and (clk_early = '1') and (HOST_nW = '0') and (HOST_ID = '0')) else '0';
    host_r_oe  <= '1' when ((host_oe = '1') and (HOST_nW = '0')) else '0';

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
            D           => MEM_D,
            Q           => pixel);

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
            D           => MEM_D,
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
            Q           => MEM_D);

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
            D           => pixel_addr(17 downto 0),
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
            D           => HOST_A(18 downto 1),
            Q           => MEM_A);

    --
    -- Host data bus (read mode) multiplexer
    --
    host_d_r_mux: entity work.mux_2ch_oe
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
    -- PLL
    --
    pll_inst: entity work.pll
        generic map(
            -- assumes 50MHz sysclk
            pll_divr        => "0100",
            pll_fr          => "001",

            -- 640x480
            -- pll_divf        => "1010000",
            -- pll_divq        => "101"

            -- 800x600
            pll_divf        => "0111111",
            pll_divq        => "010"

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
        variable state      : integer range 0 to 7 := 0;
        variable h_count    : integer range 0 to h_period - 1 := 0;
        variable v_count    : integer range 0 to v_period - 1 := 0;
    begin
        if(HOST_RESET = '1') then
            state       := 0;

            pixel_addr  <= (others => '0');

            host_d_we   <= '0';
            host_cycle  <= '0';

            h_count     := 0;
            v_count     := 0;

            MEM_nCE1    <= '1';
            MEM_nCE0    <= '1';
            MEM_nOE     <= '1';
            MEM_nWE     <= '1';
            MEM_nUB     <= '1';
            MEM_nLB     <= '1';

            VGA_R       <= (others => '0');
            VGA_G       <= (others => '0');
            VGA_B       <= (others => '0');
            VGA_nVSYNC  <= not vga_v_sync_pol;
            VGA_nHSYNC  <= not vga_h_sync_pol;

            HOST_nIRQ   <= '1';
        elsif(rising_edge(pixel_clk)) then
            state := state + 1;

            if(state = 2) then
                clk_early <= '1';
            else
                clk_early <= '0';
            end if;

            if(state = 6) then
                clk_late <= '1';
            else
                clk_late <= '0';
            end if;

            if(state <= 3) then
                host_oe <= '0';
                pix_oe  <= '1';
            else
                host_oe <= '1';
                pix_oe  <= '0';
            end if;

            if(state <= 3) then
                -- pixel cycle
                if(pixel_addr(18) = '0') then
                    MEM_nCE0 <= '0';
                    MEM_nCE1 <= '1';
                else
                    MEM_nCE0 <= '1';
                    MEM_nCE1 <= '0';
                end if;

                MEM_nWE  <= '1';
                MEM_nOE  <= '0';
                MEM_nUB  <= '0';
                MEM_nLB  <= '0';
            else
                -- host cycle
                if(host_cycle = '1') then
                    MEM_nCE0 <= HOST_A(19);
                    MEM_nCE1 <= not HOST_A(19);

                    -- this is not good
                    if(state < 7) then
                        MEM_nWE <= HOST_nW;
                    else
                        MEM_nWE <= '1';
                    end if;
                    MEM_nOE  <= not HOST_nW;

                    MEM_nUB  <= HOST_nUCS;
                    MEM_nLB  <= HOST_nLCS;
                else
                    MEM_nCE0 <= '1';
                    MEM_nCE1 <= '1';

                    MEM_nWE  <= '1';
                    MEM_nOE  <= '1';

                    MEM_nUB  <= '1';
                    MEM_nLB  <= '1';
                end if;
            end if;

            -- Handle host cycle
            if(state = 0) then
                if(host_cycle = '0') then
                    if((HOST_nUCS = '0') or (HOST_nLCS = '0')) then
                        host_cycle <= '1';
                        host_d_we <= HOST_nW;
                    end if;
                else
                    if((HOST_nUCS = '1') and (HOST_nLCS = '1')) then
                        host_cycle <= '0';
                    end if;
                end if;
            end if;

            -- Scan out pixel
            if(state = 0) then
                VGA_R <= pixel(15 downto 13);
                VGA_G <= pixel(12 downto 10);
                VGA_B <= pixel(9 downto 8);
            elsif(state = 4) then
                VGA_R <= pixel(7 downto 5);
                VGA_G <= pixel(4 downto 2);
                VGA_B <= pixel(1 downto 0);
            end if;

            -- Generate horizontal sync
            if((h_count >= (vga_h_pixels + vga_h_fp)) and
               (h_count <= (vga_h_pixels + vga_h_fp + vga_h_sync))) then
                VGA_nHSYNC <= vga_h_sync_pol;
            else
                VGA_nHSYNC <= not vga_h_sync_pol;
            end if;

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
                        if(state = 0) then
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

