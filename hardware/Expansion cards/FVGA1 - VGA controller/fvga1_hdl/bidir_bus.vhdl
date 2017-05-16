-- Bidirectional bus
--
-- Stuart Wallace, May 2017.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity bidir_bus is
    generic(
        width   : integer := 1                                  -- bus width in bits
    );

    port(
        PIN     : inout std_logic_vector(width - 1 downto 0);   -- port "pins" (bidirectional)

        READ    : out   std_logic_vector(width - 1 downto 0);   -- read port (input from pins)
        WRITE   : in    std_logic_vector(width - 1 downto 0);   -- write port (output to pins)

        ENABLE  : in    std_logic                               -- output enable
    );
end bidir_bus;


architecture behaviour of bidir_bus is
begin
    process(ENABLE, WRITE)
    begin
        if(ENABLE = '1') then
            PIN <= WRITE;
        else
            PIN <= (others => 'Z');
        end if;
        READ <= PIN;
    end process;
end behaviour;
