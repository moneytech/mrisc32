library ieee;
use ieee.std_logic_1164.all;

package consts is
  -- ALU operations.
  constant OP_CPUID  : std_logic_vector(8 downto 0) := "000000000";

  constant OP_LDHI   : std_logic_vector(8 downto 0) := "000000001";
  constant OP_LDHIO  : std_logic_vector(8 downto 0) := "000000010";

  constant OP_OR     : std_logic_vector(8 downto 0) := "000010000";
  constant OP_NOR    : std_logic_vector(8 downto 0) := "000010001";
  constant OP_AND    : std_logic_vector(8 downto 0) := "000010010";
  constant OP_BIC    : std_logic_vector(8 downto 0) := "000010011";
  constant OP_XOR    : std_logic_vector(8 downto 0) := "000010100";
  constant OP_ADD    : std_logic_vector(8 downto 0) := "000010101";
  constant OP_SUB    : std_logic_vector(8 downto 0) := "000010110";
  constant OP_SLT    : std_logic_vector(8 downto 0) := "000010111";
  constant OP_SLTU   : std_logic_vector(8 downto 0) := "000011000";
  constant OP_CEQ    : std_logic_vector(8 downto 0) := "000011001";
  constant OP_CLT    : std_logic_vector(8 downto 0) := "000011010";
  constant OP_CLTU   : std_logic_vector(8 downto 0) := "000011011";
  constant OP_CLE    : std_logic_vector(8 downto 0) := "000011100";
  constant OP_CLEU   : std_logic_vector(8 downto 0) := "000011101";

  constant OP_LSR    : std_logic_vector(8 downto 0) := "000011110";
  constant OP_ASR    : std_logic_vector(8 downto 0) := "000011111";
  constant OP_LSL    : std_logic_vector(8 downto 0) := "000100000";
  
  constant OP_SHUF   : std_logic_vector(8 downto 0) := "000100001";

  constant OP_SEL    : std_logic_vector(8 downto 0) := "001000000";
  constant OP_CLZ    : std_logic_vector(8 downto 0) := "001000001";
  constant OP_REV    : std_logic_vector(8 downto 0) := "001000010";
  constant OP_EXTB   : std_logic_vector(8 downto 0) := "001000011";
  constant OP_EXTH   : std_logic_vector(8 downto 0) := "001000100";

  -- MUL/DIV operations.
  constant OP_MUL    : std_logic_vector(8 downto 0) := "000110000";
  constant OP_MULHI  : std_logic_vector(8 downto 0) := "000110010";
  constant OP_MULHIU : std_logic_vector(8 downto 0) := "000110011";
  constant OP_DIV    : std_logic_vector(8 downto 0) := "000110100";
  constant OP_DIVU   : std_logic_vector(8 downto 0) := "000110101";
  constant OP_REM    : std_logic_vector(8 downto 0) := "000110110";
  constant OP_REMU   : std_logic_vector(8 downto 0) := "000110111";

  -- FPU operations.
  constant OP_ITOF   : std_logic_vector(8 downto 0) := "000111000";
  constant OP_FTOI   : std_logic_vector(8 downto 0) := "000111001";
  constant OP_FADD   : std_logic_vector(8 downto 0) := "000111010";
  constant OP_FSUB   : std_logic_vector(8 downto 0) := "000111011";
  constant OP_FMUL   : std_logic_vector(8 downto 0) := "000111100";
  constant OP_FDIV   : std_logic_vector(8 downto 0) := "000111101";
end package;