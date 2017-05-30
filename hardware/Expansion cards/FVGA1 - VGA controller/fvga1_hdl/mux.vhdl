-- Multiplexer array
--
-- Stuart Wallace, May 2017.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity mux_2ch_oe is
    generic(
        width       : integer := 8
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
    process(SEL, OE, A, B)
    begin
        if(OE = '1') then
            if(SEL = '1') then
                Q <= B;
            else
                Q <= A;
            end if;
        else
            Q <= (others => 'Z');
        end if;
    end process;
end behaviour;

