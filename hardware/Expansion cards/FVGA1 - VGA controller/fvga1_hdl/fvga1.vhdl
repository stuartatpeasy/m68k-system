-- FVGA1 - VGA framebuffer card driver
--
-- Stuart Wallace, 2017.
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
-- -------  ------------------  --f--  --2f-  --4f-
-- clk/MHz  dR    fr   dF        dQ     dQ     dQ
-- -------  ------------------  -----  -----  -----
--  25.175  0100  001  1010000   101    100    011
--  40.000  0100  001  0111111   100    011    010
--  65.000  0100  001  1100111   100    011    010
-- -------  ------------------  -----  -----  -----


library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;


entity fvga1 is
    port(
        SYSCLK      : in std_logic;                             -- 50MHz clock input

        MEM_D       : inout std_logic_vector(15 downto 0);      -- RAM data bus
        MEM_A       : out std_logic_vector(17 downto 0);        -- RAM address bus

        MEM_nCE1,                                               -- RAM chip enable 1
        MEM_nCE0,                                               -- RAM chip enable 0
        MEM_nUB,                                                -- RAM upper byte enable
        MEM_nLB,                                                -- RAM lower byte enable
        MEM_nOE,                                                -- RAM output enable
        MEM_nWE     : out std_logic;                            -- RAM write enable

        HOST_D      : inout std_logic_vector(15 downto 0);      -- Host data bus
        HOST_A      : in std_logic_vector(19 downto 1);         -- Host address bus

        HOST_ID,                                                -- Host ID request
        HOST_nUCS,                                              -- Host upper byte chip select
        HOST_nLCS,                                              -- Host lower byte chip select
        HOST_nW,                                                -- Host write request
        HOST_RESET  : in std_logic;                             -- Host reset input
        HOST_nIRQ   : out std_logic;                            -- Host IRQ

        VGA_R,                                                  -- VGA red channel output
        VGA_G       : out std_logic_vector(2 downto 0);         -- VGA green channel output
        VGA_B       : out std_logic_vector(1 downto 0);         -- VGA blue channel output
        VGA_nVSYNC,                                             -- VGA vertical sync output
        VGA_nHSYNC  : out std_logic                             -- VGA horizontal sync output
    );
end;


architecture behaviour of fvga1 is
    constant mem_d_width    : integer := 16;                    -- width of memory data bus
    constant host_d_width   : integer := 16;

    -- 640x480 timings
--    constant vga_h_fp       : integer := 16;
--    constant vga_h_bp       : integer := 48;
--    constant vga_h_sync     : integer := 96;
--    constant vga_h_pixels   : integer := 640;
--    constant vga_h_sync_pol : std_logic := '0';
--
--    constant vga_v_fp       : integer := 10;
--    constant vga_v_bp       : integer := 33;
--    constant vga_v_sync     : integer := 2;
--    constant vga_v_pixels   : integer := 480;
--    constant vga_v_sync_pol : std_logic := '0';

    -- 1024x768 timings
    constant vga_h_fp               : integer := 24;
    constant vga_h_bp               : integer := 160;
    constant vga_h_sync             : integer := 136;
    constant vga_h_pixels           : integer := 1024;
    constant vga_h_sync_pol         : std_logic := '0';

    constant vga_v_fp               : integer := 3;
    constant vga_v_bp               : integer := 29;
    constant vga_v_sync             : integer := 6;
    constant vga_v_pixels           : integer := 768;
    constant vga_v_sync_pol         : std_logic := '0';

    constant device_id              : std_logic_vector(mem_d_width - 1 downto 0) := x"8400";

    constant h_period               : integer := vga_h_sync + vga_h_bp + vga_h_pixels + vga_h_fp;
    constant v_period               : integer := vga_v_sync + vga_v_bp + vga_v_pixels + vga_v_fp;

    signal mem_d_r                  : std_logic_vector(mem_d_width - 1 downto 0);   -- memory read bus
    signal mem_d_w                  : std_logic_vector(mem_d_width - 1 downto 0);   -- memory write bus
    signal mem_we                   : std_logic := '0';                             -- memory write enable

    signal host_d_r                 : std_logic_vector(host_d_width - 1 downto 0);  -- host data bus read
    signal host_d_w                 : std_logic_vector(host_d_width - 1 downto 0);  -- host data bus write
    signal host_we                  : std_logic := '0';                             -- enable write to host

    signal host_cycle_in_progress   : std_logic := '0';

    signal pixel_clk                : std_logic;
    signal pixel                    : std_logic_vector(15 downto 0);
begin
    pll_inst: entity work.pll
        generic map(
            -- assumes 50MHz sysclk
            pll_divr        => "0100",
            pll_fr          => "001",

            -- 640x480
            -- pll_divf        => "1010000",
            -- pll_divq        => "101"

            -- 800x600
            -- pll_divf        => "0111111",
            -- pll_divq        => "100"

            -- 1024x768
            pll_divf        => "1100111",
            pll_divq        => "010"
        )
        port map(
            REFERENCECLK    => SYSCLK,
            PLLOUTCORE      => pixel_clk,
            PLLOUTGLOBAL    => open,
            RESET           => '1'
        );

    mem_d_inst: entity work.bidir_bus
        generic map(
            width           => mem_d_width
        )
        port map(
            PIN             => MEM_D,
            READ            => mem_d_r,
            WRITE           => mem_d_w,
            ENABLE          => mem_we
        );

    host_d_inst: entity work.bidir_bus
        generic map(
            width           => host_d_width
        )
        port map(
            PIN             => HOST_D,
            READ            => host_d_r,
            WRITE           => host_d_w,
            ENABLE          => host_we
        );

    process(HOST_RESET, pixel_clk, HOST_nUCS, HOST_nLCS, HOST_nW, HOST_A)
        variable state      : integer range 0 to 7 := 0;
        variable pixel_addr : std_logic_vector(18 downto 0) := (others => '0');    -- address of the next pair of pixels
        variable h_count    : integer range 0 to h_period - 1 := 0;
        variable v_count    : integer range 0 to v_period - 1 := 0;
    begin
        if(HOST_RESET = '1') then
            -- Negate host interrupt request
            HOST_nIRQ   <= '1';

            host_we     <= '0';
            host_d_w    <= (others => '0');

            -- Deactivate local memory interface
            MEM_A       <= (others => '0');
            mem_we      <= '0';
            mem_d_w     <= (others => '0');

            MEM_nCE1    <= '1';
            MEM_nCE0    <= '1';
            MEM_nUB     <= '1';
            MEM_nLB     <= '1';
            MEM_nOE     <= '1';
            MEM_nWE     <= '1';

            -- Deactivate VGA outputs
            VGA_R       <= (others => '0');
            VGA_G       <= (others => '0');
            VGA_B       <= (others => '0');
            VGA_nVSYNC  <= not vga_h_sync_pol;
            VGA_nHSYNC  <= not vga_v_sync_pol;

            h_count     := 0;
            v_count     := 0;

            pixel       <= (others => '0');
            pixel_addr  := 0;
            state       := 0;

            host_cycle_in_progress <= '0';
        elsif(rising_edge(pixel_clk)) then
            case state is
                when 0 =>                       -- state 0: begin host cycle; scan out first pixel
                    VGA_R <= pixel(15 downto 13);
                    VGA_G <= pixel(12 downto 10);
                    VGA_B <= pixel(9 downto 8);

                    if((HOST_nUCS = '0') or (HOST_nLCS = '0')) then
                        if(host_cycle_in_progress = '0') then
                            host_cycle_in_progress <= '1';

                            if(HOST_ID = '0') then          -- regular (non-ID) cycle
                                MEM_A(17 downto 0) <= HOST_A(18 downto 1);
                                MEM_nWE <= HOST_nW;
                                MEM_nOE <= not HOST_nW;
                                MEM_nUB <= HOST_nUCS;
                                MEM_nLB <= HOST_nLCS;
    
                                if(HOST_A(19) = '1') then
                                    MEM_nCE1 <= '0';
                                    MEM_nCE0 <= '1';
                                else
                                    MEM_nCE1 <= '1';
                                    MEM_nCE0 <= '0';
                                end if;

                                if(HOST_nW = '0') then
                                    mem_we  <= '1';         -- host write cycle
                                    host_we <= '0';
                                    mem_d_w <= host_d_r;
                                else
                                    mem_we  <= '0';         -- host read cycle
                                    host_we <= '1';
                                end if;
                            else                            -- ID cycle
                                MEM_nCE1    <= '1';
                                MEM_nCE0    <= '1';
                                MEM_nUB     <= '1';
                                MEM_nLB     <= '1';
                                MEM_nWE     <= '1';
                                MEM_nOE     <= '1';

                                MEM_A       <= (others => '0');
                                mem_we      <= '0';
                                mem_d_w     <= (others => '0');
                                host_we     <= '1';
                                host_d_w    <= device_id;
                            end if;
                        end if;
                    else
                        MEM_nCE1    <= '1';
                        MEM_nCE0    <= '1';
                        MEM_nUB     <= '1';
                        MEM_nLB     <= '1';
                        MEM_nWE     <= '1';
                        MEM_nOE     <= '1';
        
                        MEM_A       <= (others => '0');
                        mem_we      <= '0';
                        mem_d_w     <= (others => '0');
                        host_we     <= '0';
                    end if;
                    state := state + 1;

                when 3 =>                       -- state 3: end host cycle
                    if(((HOST_nUCS = '0') or (HOST_nLCS = '0'))
                       and (host_cycle_in_progress = '1')
                       and (HOST_nW = '1') 
                       and (HOST_ID = '0')) then
                        host_d_w <= mem_d_r;        -- host read cycle
                    end if;
                    state := state + 1;

                when 4 =>                       -- state 4: begin pixel read cycle; scan out second pixel
                    VGA_R <= pixel(7 downto 5);
                    VGA_G <= pixel(4 downto 2);
                    VGA_B <= pixel(1 downto 0);

                    mem_we <= '0';

                    if((h_count < vga_h_pixels) and (v_count < vga_v_pixels)) then
                        MEM_A(17 downto 0) <= pixel_addr(17 downto 0);
    
                        if(pixel_addr(18) = '1') then
                            MEM_nCE1 <= '0';
                            MEM_nCE0 <= '1';
                        else
                            MEM_nCE1 <= '1';
                            MEM_nCE0 <= '0';
                        end if;
    
                        MEM_nUB <= '0';
                        MEM_nLB <= '0';
                        MEM_nWE <= '1';
                        MEM_nOE <= '0';
                    end if;
                    state := state + 1;
                
                when 7 =>                      -- state 7: end pixel read cycle
                    if((h_count < vga_h_pixels) and (v_count < vga_v_pixels)) then
                        pixel <= mem_d_r;
                    else
                        pixel <= (others => '0');
                    end if;
                    state := 0;

                when others =>
                    state := state + 1;
            end case;

            if((HOST_nUCS = '1') and (HOST_nLCS = '1')) then
                host_cycle_in_progress <= '0';
            end if;

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
                    if(state = 0) then
                        pixel_addr := pixel_addr + 1;
                    end if;
                elsif(h_count < h_period) then
                    h_count := h_count + 1;
                else
                    h_count := 0;
                    if(v_count < vga_v_pixels) then
                        v_count := v_count + 1;
                        if(state = 0) then
                            pixel_addr := pixel_addr + 1;
                        end if;
                    elsif(v_count < v_period) then
                        v_count := v_count + 1;
                    else
                        v_count := 0;
                        pixel_addr := 0;
                    end if;
                end if;
            end if;
        end if;
    end process;
end behaviour;

