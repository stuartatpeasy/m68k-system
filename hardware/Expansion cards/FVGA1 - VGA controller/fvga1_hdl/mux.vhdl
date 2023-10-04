-- Multiplexer array
--
-- Stuart Wallace, May 2017.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity mux_2ch_oe is
    generic(
        width       : integer := 1
    );

    port(
        A           : in  std_logic_vector(width - 1 downto 0);
        B           : in  std_logic_vector(width - 1 downto 0);
        Q           : out std_logic_vector(width - 1 downto 0);
        SEL         : in  std_logic;
        OE          : in  std_logic
    );
end mux_2ch_oe;


architecture behaviour of mux_2ch_oe is
begin
    Q <= A when ((SEL = '0') and (OE = '1')) else
         B when ((SEL = '1') and (OE = '1')) else
         (others => 'Z');
end behaviour;

