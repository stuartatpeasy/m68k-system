-- PLL for simple VGA controller
--
-- Stuart Wallace, July 2016.
--

library IEEE;
use IEEE.std_logic_1164.all;


entity pll is
    generic(
        pll_fr          : bit_vector(2 downto 0);
        pll_divr        : bit_vector(3 downto 0);
        pll_divf        : bit_vector(6 downto 0);
        pll_divq        : bit_vector(2 downto 0)
    );

    port(
        REFERENCECLK    : in std_logic;
        RESET           : in std_logic;
        PLLOUTCORE      : out std_logic;
        PLLOUTGLOBAL    : out std_logic
    );
end entity pll;

architecture behaviour of pll is
component SB_PLL40_CORE
    generic(
        -- feedback
        FEEDBACK_PATH                   : string := "SIMPLE";
        DELAY_ADJUSTMENT_MODE_FEEDBACK  : string := "FIXED";
        DELAY_ADJUSTMENT_MODE_RELATIVE  : string := "FIXED";
        SHIFTREG_DIV_MODE               : bit_vector(1 downto 0)    := "00";
        FDA_FEEDBACK                    : bit_vector(3 downto 0)    := "0000";
        FDA_RELATIVE                    : bit_vector(3 downto 0)    := "0000";
        PLLOUT_SELECT                   : string := "GENCLK";

        -- dividers
        DIVF                            : bit_vector(6 downto 0);
        DIVR                            : bit_vector(3 downto 0);
        DIVQ                            : bit_vector(2 downto 0);
        FILTER_RANGE                    : bit_vector(2 downto 0);

        -- additional control bits
        ENABLE_ICEGATE                  : bit := '0';

        -- test mode parameters
        TEST_MODE                       : bit := '0';
        EXTERNAL_DIVIDE_FACTOR          : integer := 1
    );

    port(
        REFERENCECLK    : in std_logic;     -- Driven by core logic
        PLLOUTCORE      : out std_logic;    -- PLL output to core logic
        PLLOUTGLOBAL    : out std_logic;    -- PLL output to global network
        EXTFEEDBACK     : in std_logic;     -- Driven by core logic
        DYNAMICDELAY    : in std_logic_vector (7 downto 0); -- Driven by core logic
        LOCK            : out std_logic;    -- Output of PLL
        BYPASS          : in std_logic;     -- Driven by core logic
        RESETB          : in std_logic;     -- Driven by core logic
        LATCHINPUTVALUE : in std_logic;     -- iCEGate Signal

        -- Test Pins
        SDO             : out std_logic;
        SDI             : in std_logic;
        SCLK            : in std_logic
    );
end component;

begin
    pll_inst: SB_PLL40_CORE
    generic map(
        DIVF                            => pll_divf,
        DIVQ                            => pll_divq,

        -- These are constant for every supported VGA resolution
        DIVR                            => pll_divr,
        FILTER_RANGE                    => pll_fr,

        -- These are constant for all PLL configurations
        FEEDBACK_PATH                   => "SIMPLE",
        DELAY_ADJUSTMENT_MODE_FEEDBACK  => "FIXED",
        FDA_FEEDBACK                    => "0000",
        DELAY_ADJUSTMENT_MODE_RELATIVE  => "FIXED",
        FDA_RELATIVE                    => "0000",
        SHIFTREG_DIV_MODE               => "00",
        PLLOUT_SELECT                   => "GENCLK",
        ENABLE_ICEGATE                  => '0'
    )
    port map(
        REFERENCECLK    => REFERENCECLK,
        PLLOUTCORE      => PLLOUTCORE,
        PLLOUTGLOBAL    => PLLOUTGLOBAL,
        EXTFEEDBACK     => '0',
        DYNAMICDELAY    => "00000000", -- open,
        RESETB          => RESET,
        BYPASS          => '0',
        LATCHINPUTVALUE => '0',
        LOCK            => open,
        SDI             => '0',
        SDO             => open,
        SCLK            => '0'
    );
end behaviour;

