-- VGA output demonstration for Lattice ICEstick
--
-- Stuart Wallace, July 2016.
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
--  65.000  0100  001  1100111   100    010    010
-- -------  ------------------  -----  -----  -----

-- 75MHz clock
-- -------  ------------------  --f--  --2f-  --4f-
-- clk/MHz  dR    fr   dF         dQ     dQ     dQ
-- -------  ------------------  -----  -----  -----
--  25.175  0011  010  0101010   101    100    011
--  40.000  0001  011  0010000   100    011    010
--  65.000  0110  001  1100000   100    011    010
-- -------  ------------------  -----  -----  -----

-- 80MHz clock
-- -------  ------------------  --f--  --2f-  --4f-
-- clk/MHz  dR    fr   dF        dQ     dQ     dQ
-- -------  ------------------  -----  -----  -----
--  25.175  0111  001  1010000   101    100    011
--  40.000  0000  101  0000111   100    011    010
--  65.000  0000  101  0001100   100    011    010
-- -------  ------------------  -----  -----  -----

-- 100MHz clock
-- -------  ------------------  --f--  --2f-  --4f-
-- clk/MHz  dR    fr   dF        dQ     dQ     dQ
-- -------  ------------------  -----  -----  -----
--  25.175  1001  001  1010000   101    100    011
--  40.000  0100  010  0011111   100    011    010
--  65.000  0100  010  0110011   100    011    010
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

        HOST_nID,                                               -- Host ID request
        HOST_nUCS,                                              -- Host upper byte chip select
        HOST_nLCS,                                              -- Host lower byte chip select
        HOST_nW,                                                -- Host write request
        HOST_nRESET : in std_logic;                             -- Host reset input
        HOST_nIRQ   : out std_logic;                            -- Host IRQ

        VGA_R,                                                  -- VGA red channel output
        VGA_G       : out std_logic_vector(2 downto 0);         -- VGA green channel output
        VGA_B       : out std_logic_vector(1 downto 0);         -- VGA blue channel output
        VGA_VSYNC,                                              -- VGA vertical sync output
        VGA_HSYNC   : out std_logic                             -- VGA horizontal sync output
    );
end;


architecture behaviour of fvga1 is
    signal pixel_clk        : std_logic;
    signal pixel_data: std_logic_vector(15 downto 0);
begin
    pll_inst: entity work.pll
        generic map(
            -- assumes 50MHz sysclk
            pll_divr        => "0100",
            pll_fr          => "001",

            -- 640x480
            pll_divf        => "1010000",
            pll_divq        => "101"

            -- 800x600
            -- pll_divf        => "0111111",
            -- pll_divq        => "100"

            -- 1024x768
            -- pll_divf        => "1100111",
            -- pll_divq        => "100"
        )
        port map(
            REFERENCECLK    => SYSCLK,
            PLLOUTCORE      => pixel_clk,
            PLLOUTGLOBAL    => open,
            RESET           => '1'
        );

    process(HOST_nRESET, pixel_clk, HOST_nUCS, HOST_nLCS, HOST_nW, HOST_A, HOST_D)
        variable cycle: integer range 0 to 1 := 0;
    begin
        if(HOST_nRESET = '0') then
            -- Negate host interrupt request
            HOST_nIRQ <= '1';

            -- Deactivate local memory interface
            MEM_A       <= (others => '0');
            MEM_D       <= (others => 'Z');

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
            VGA_VSYNC   <= '0';
            VGA_HSYNC   <= '0';

            HOST_D      <= (others => 'Z');

            cycle := 0;
        elsif(rising_edge(pixel_clk)) then
            if(cycle = 0) then
                -- Cycle 0: latch host read data, if a read cycle was requested; set the bus up to
                -- read pixel data.
                if(HOST_nUCS = '0') then
                    HOST_D(15 downto 8) <= MEM_D(15 downto 8);
                end if;

                if(HOST_nLCS = '0') then
                    HOST_D(7 downto 0) <= MEM_D(7 downto 0);
                end if;

                -- Start a 16-bit read cycle to obtain pixel data
                MEM_nUB <= '0';
                MEM_nLB <= '0';
                MEM_nWE <= '1';
                MEM_nOE <= '0';

                MEM_A <= (others => '0');       -- temporary

                cycle := 1;
            else
                -- Cycle 1: latch pixel data; run a host memory cycle, if one has been requested.
                pixel_data <= MEM_D;

                if((HOST_nUCS = '0') or (HOST_nLCS = '0')) then
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

                    if(HOST_nW = '1') then
                        -- Host read cycle
                    else
                        -- Host write cycle
                    end if;
                else
                    MEM_nCE1    <= '1';
                    MEM_nCE0    <= '1';
                    MEM_nUB     <= '1';
                    MEM_nLB     <= '1';
                    MEM_nWE     <= '1';
                    MEM_nOE     <= '1';
    
                    MEM_A       <= (others => '0');
                    MEM_D       <= (others => '0');
                end if;

                cycle := 0;
            end if;
        end if;
    end process;
end behaviour;

