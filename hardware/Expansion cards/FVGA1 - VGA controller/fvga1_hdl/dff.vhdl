-- D flip-flop array
--
-- Stuart Wallace, May 2017.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--
-- dff_oe_areset: D flip-flop array with output enable and asynchronous reset input
--
entity dff_oe_areset is
    generic(
        width       : integer   := 8;
        pol_clk     : std_logic := '1';
        pol_oe      : std_logic := '1';
        pol_reset   : std_logic := '1'
    );

    port(
        D           : in  std_logic_vector(width - 1 downto 0);
        Q           : out std_logic_vector(width - 1 downto 0);
        CLK         : in  std_logic;
        OE          : in  std_logic;
        RESET       : in  std_logic
    );
end dff_oe_areset;


architecture behaviour of dff_oe_areset is
    signal q_int: std_logic_vector(width - 1 downto 0);
begin
    process(CLK, OE, RESET)
    begin
        if(((pol_reset = '1') and (RESET = '1')) or
           ((pol_reset = '0') and (RESET = '0'))) then
            q_int <= (others => '0');
        elsif(((pol_clk = '1') and rising_edge(CLK)) or
              ((pol_clk = '0') and falling_edge(CLK))) then
            q_int <= D;
        end if;

        if(((pol_oe = '1') and (OE = '1')) or
           ((pol_oe = '0') and (OE = '0'))) then
            Q <= q_int;
        else
            Q <= (others => 'Z');
        end if;
    end process;
end behaviour;

