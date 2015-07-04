#ifndef __CPU_EXCEPTIONS_H__
#define __CPU_EXCEPTIONS_H__
/*
	MC68010 exception-management function and macro declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include "stdio.h"
#include "include/types.h"
#include "cpu/utilities.h"
#include "cpu/exceptionstackframes.h"


/*
	CPU_EXC_VTBL_ENTRY: a vector table entry, i.e. the address in RAM of a particular exception
	vector.
*/
#define CPU_EXC_VTBL(x)	(*((vu32 *) (x)))


/*
	CPU_EXC_VPTR: macro to be used when setting a vector table entry.  For example, to cause
	address	errors to be redirected to the function address_error, do this:

		V_address_error = VTBL_PTR(address_error);
*/
#define CPU_EXC_VPTR(x)	((u32) (x))


/*
	CPU_EXC_VPTR_SET(x, p): set vector number x to point at function p.
*/
#define CPU_EXC_VPTR_SET(x, p)	do { CPU_EXC_VTBL((x) << 2) = CPU_EXC_VPTR(p); } while(0)

/*
    IRQ_HANDLER_DECL(fn): declare fn as an interrupt handler
    IRQ_HANDLER_FN(fn): define fn as an interrupt handler
*/
#define IRQ_HANDLER_DECL(fn)   void fn(void) __attribute__((interrupt_handler))
#define IRQ_HANDLER_FN(fn)   void fn(void)


/*
	Vector table definition
*/

#define V_ssp					CPU_EXC_VTBL(0x000000) /*   0 000 Supervisor stack pointer		*/
#define V_reset					CPU_EXC_VTBL(0x000004) /*   1 004 Reset initial program counter	*/
#define V_bus_error				CPU_EXC_VTBL(0x000008) /*   2 008 Bus error						*/
#define V_address_error			CPU_EXC_VTBL(0x00000c) /*   3 00c Address error					*/
#define V_illegal_instruction	CPU_EXC_VTBL(0x000010) /*   4 010 Illegal instruction			*/
#define V_int_divide_by_zero	CPU_EXC_VTBL(0x000014) /*   5 014 Integer divide by zero		*/
#define V_chk_instruction		CPU_EXC_VTBL(0x000018) /*   6 018 CHK/CHK2 instruction			*/
#define V_trap					CPU_EXC_VTBL(0x00001c) /*   7 01c FTRAPcc/TRAPcc/TRAPV instr.	*/
#define V_privilege_violation	CPU_EXC_VTBL(0x000020) /*   8 020 Privilege violation			*/
#define V_trace					CPU_EXC_VTBL(0x000024) /*   9 024 Trace							*/
#define V_line_1010_emulator	CPU_EXC_VTBL(0x000028) /*  10 028 Line 1010 emulator			*/
#define V_line_1111_emulator	CPU_EXC_VTBL(0x00002c) /*  11 02c Line 1111 emulator			*/
/*                                                         12 030 [unassigned / reserved]		*/
#define V_coproc_prot_violation	CPU_EXC_VTBL(0x000034) /*  13 034 Coprocessor protocol violation*/
#define V_format_error			CPU_EXC_VTBL(0x000038) /*  14 038 Format error					*/
#define V_uninit_interrupt		CPU_EXC_VTBL(0x00003c) /*  15 03c Unitialised interrupt			*/
/*                                                         16 040 [unassigned / reserved]		*/
/*                                                         17 044 [unassigned / reserved]		*/
/*                                                         18 048 [unassigned / reserved]		*/
/*                                                         19 04c [unassigned / reserved]		*/
/*                                                         20 050 [unassigned / reserved]		*/
/*                                                         21 054 [unassigned / reserved]		*/
/*                                                         22 058 [unassigned / reserved]		*/
/*                                                         23 05c [unassigned / reserved]		*/
#define V_spurious_interrupt	CPU_EXC_VTBL(0x000060) /*  24 060 Spurious interrupt			*/
#define V_level_1_autovector	CPU_EXC_VTBL(0x000064) /*  25 064 Level 1 interrupt autovector	*/
#define V_level_2_autovector	CPU_EXC_VTBL(0x000068) /*  26 068 Level 2 interrupt autovector	*/
#define V_level_3_autovector	CPU_EXC_VTBL(0x00006c) /*  27 06c Level 3 interrupt autovector	*/
#define V_level_4_autovector	CPU_EXC_VTBL(0x000070) /*  28 070 Level 4 interrupt autovector	*/
#define V_level_5_autovector	CPU_EXC_VTBL(0x000074) /*  29 074 Level 5 interrupt autovector	*/
#define V_level_6_autovector	CPU_EXC_VTBL(0x000078) /*  30 078 Level 6 interrupt autovector	*/
#define V_level_7_autovector	CPU_EXC_VTBL(0x00007c) /*  31 07c Level 7 interrupt autovector	*/
#define V_trap_0				CPU_EXC_VTBL(0x000080) /*  32 080 Trap #0						*/
#define V_trap_1				CPU_EXC_VTBL(0x000084) /*  33 084 Trap #1						*/
#define V_trap_2				CPU_EXC_VTBL(0x000088) /*  34 088 Trap #2						*/
#define V_trap_3				CPU_EXC_VTBL(0x00008c) /*  35 08c Trap #3						*/
#define V_trap_4				CPU_EXC_VTBL(0x000090) /*  36 090 Trap #4						*/
#define V_trap_5				CPU_EXC_VTBL(0x000094) /*  37 094 Trap #5						*/
#define V_trap_6				CPU_EXC_VTBL(0x000098) /*  38 098 Trap #6						*/
#define V_trap_7				CPU_EXC_VTBL(0x00009c) /*  39 09c Trap #7						*/
#define V_trap_8				CPU_EXC_VTBL(0x0000a0) /*  40 0a0 Trap #8						*/
#define V_trap_9				CPU_EXC_VTBL(0x0000a4) /*  41 0a4 Trap #9						*/
#define V_trap_10				CPU_EXC_VTBL(0x0000a8) /*  42 0a8 Trap #10						*/
#define V_trap_11				CPU_EXC_VTBL(0x0000ac) /*  43 0ac Trap #11						*/
#define V_trap_12				CPU_EXC_VTBL(0x0000b0) /*  44 0b0 Trap #12						*/
#define V_trap_13				CPU_EXC_VTBL(0x0000b4) /*  45 0b4 Trap #13						*/
#define V_trap_14				CPU_EXC_VTBL(0x0000b8) /*  46 0b8 Trap #14						*/
#define V_trap_15				CPU_EXC_VTBL(0x0000bc) /*  47 0bc Trap #15						*/
#define V_fp_unordered_cond		CPU_EXC_VTBL(0x0000c0) /*  48 0c0 FP bra/set on unordered cond.	*/
#define V_fp_inexact_result		CPU_EXC_VTBL(0x0000c4) /*  49 0c4 FP inexact result				*/
#define V_fp_divide_by_zero		CPU_EXC_VTBL(0x0000c8) /*  50 0c8 FP divide by zero				*/
#define V_fp_underflow			CPU_EXC_VTBL(0x0000cc) /*  51 0cc FP underflow					*/
#define V_fp_operand_error		CPU_EXC_VTBL(0x0000d0) /*  52 0d0 FP operand error				*/
#define V_fp_overflow			CPU_EXC_VTBL(0x0000d4) /*  53 0d4 FP overflow					*/
#define V_fp_signaling_nan		CPU_EXC_VTBL(0x0000d8) /*  54 0d8 FP signaling NaN				*/
#define V_fp_unimpl_data_type	CPU_EXC_VTBL(0x0000dc) /*  55 0dc FP unimplemented data type	*/
#define V_mmu_config_error		CPU_EXC_VTBL(0x0000e0) /*  56 0e0 MMU configuration error		*/
#define V_mmu_illegal_operation	CPU_EXC_VTBL(0x0000e4) /*  57 0e4 MMU illegal operation			*/
#define V_mmu_access_violation	CPU_EXC_VTBL(0x0000e8) /*  58 0e8 MMU access level violation	*/
/*                                                         59 0ec [unassigned / reserved]		*/
/*                                                         60 0f0 [unassigned / reserved]		*/
/*                                                         61 0f4 [unassigned / reserved]		*/
/*                                                         62 0f8 [unassigned / reserved]		*/
/*                                                         63 0fc [unassigned / reserved]		*/
#define V_user_0				CPU_EXC_VTBL(0x000100) /*  64 100 User-defined vector 0 		*/
#define V_user_1				CPU_EXC_VTBL(0x000104) /*  65 104 User-defined vector 1			*/
#define V_user_2				CPU_EXC_VTBL(0x000108) /*  66 108 User-defined vector 2			*/
#define V_user_3				CPU_EXC_VTBL(0x00010c) /*  67 10c User-defined vector 3			*/
#define V_user_4				CPU_EXC_VTBL(0x000110) /*  68 110 User-defined vector 4			*/
#define V_user_5				CPU_EXC_VTBL(0x000114) /*  69 114 User-defined vector 5			*/
#define V_user_6				CPU_EXC_VTBL(0x000118) /*  70 118 User-defined vector 6			*/
#define V_user_7				CPU_EXC_VTBL(0x00011c) /*  71 11c User-defined vector 7			*/
#define V_user_8				CPU_EXC_VTBL(0x000120) /*  72 120 User-defined vector 8			*/
#define V_user_9				CPU_EXC_VTBL(0x000124) /*  73 124 User-defined vector 9			*/
#define V_user_10				CPU_EXC_VTBL(0x000128) /*  74 128 User-defined vector 10		*/
#define V_user_11				CPU_EXC_VTBL(0x00012c) /*  75 12c User-defined vector 11		*/
#define V_user_12				CPU_EXC_VTBL(0x000130) /*  76 130 User-defined vector 12		*/
#define V_user_13				CPU_EXC_VTBL(0x000134) /*  77 134 User-defined vector 13		*/
#define V_user_14				CPU_EXC_VTBL(0x000138) /*  78 138 User-defined vector 14		*/
#define V_user_15				CPU_EXC_VTBL(0x00013c) /*  79 13c User-defined vector 15		*/
#define V_user_16				CPU_EXC_VTBL(0x000140) /*  80 140 User-defined vector 16		*/
#define V_user_17				CPU_EXC_VTBL(0x000144) /*  81 144 User-defined vector 17		*/
#define V_user_18				CPU_EXC_VTBL(0x000148) /*  82 148 User-defined vector 18		*/
#define V_user_19				CPU_EXC_VTBL(0x00014c) /*  83 14c User-defined vector 19		*/
#define V_user_20				CPU_EXC_VTBL(0x000150) /*  84 150 User-defined vector 20		*/
#define V_user_21				CPU_EXC_VTBL(0x000154) /*  85 154 User-defined vector 21		*/
#define V_user_22				CPU_EXC_VTBL(0x000158) /*  86 158 User-defined vector 22		*/
#define V_user_23				CPU_EXC_VTBL(0x00015c) /*  87 15c User-defined vector 23		*/
#define V_user_24				CPU_EXC_VTBL(0x000160) /*  88 160 User-defined vector 24		*/
#define V_user_25				CPU_EXC_VTBL(0x000164) /*  89 164 User-defined vector 25		*/
#define V_user_26				CPU_EXC_VTBL(0x000168) /*  90 168 User-defined vector 26		*/
#define V_user_27				CPU_EXC_VTBL(0x00016c) /*  91 16c User-defined vector 27		*/
#define V_user_28				CPU_EXC_VTBL(0x000170) /*  92 170 User-defined vector 28		*/
#define V_user_29				CPU_EXC_VTBL(0x000174) /*  93 174 User-defined vector 29		*/
#define V_user_30				CPU_EXC_VTBL(0x000178) /*  94 178 User-defined vector 30		*/
#define V_user_31				CPU_EXC_VTBL(0x00017c) /*  95 17c User-defined vector 31		*/
#define V_user_32				CPU_EXC_VTBL(0x000180) /*  96 180 User-defined vector 32		*/
#define V_user_33				CPU_EXC_VTBL(0x000184) /*  97 184 User-defined vector 33		*/
#define V_user_34				CPU_EXC_VTBL(0x000188) /*  98 188 User-defined vector 34		*/
#define V_user_35				CPU_EXC_VTBL(0x00018c) /*  99 18c User-defined vector 35		*/
#define V_user_36				CPU_EXC_VTBL(0x000190) /* 100 190 User-defined vector 36		*/
#define V_user_37				CPU_EXC_VTBL(0x000194) /* 101 194 User-defined vector 37		*/
#define V_user_38				CPU_EXC_VTBL(0x000198) /* 102 198 User-defined vector 38		*/
#define V_user_39				CPU_EXC_VTBL(0x00019c) /* 103 19c User-defined vector 39		*/
#define V_user_40				CPU_EXC_VTBL(0x0001a0) /* 104 1a0 User-defined vector 40		*/
#define V_user_41				CPU_EXC_VTBL(0x0001a4) /* 105 1a4 User-defined vector 41		*/
#define V_user_42				CPU_EXC_VTBL(0x0001a8) /* 106 1a8 User-defined vector 42		*/
#define V_user_43				CPU_EXC_VTBL(0x0001ac) /* 107 1ac User-defined vector 43		*/
#define V_user_44				CPU_EXC_VTBL(0x0001b0) /* 108 1b0 User-defined vector 44		*/
#define V_user_45				CPU_EXC_VTBL(0x0001b4) /* 109 1b4 User-defined vector 45		*/
#define V_user_46				CPU_EXC_VTBL(0x0001b8) /* 110 1b8 User-defined vector 46		*/
#define V_user_47				CPU_EXC_VTBL(0x0001bc) /* 111 1bc User-defined vector 47		*/
#define V_user_48				CPU_EXC_VTBL(0x0001c0) /* 112 1c0 User-defined vector 48		*/
#define V_user_49				CPU_EXC_VTBL(0x0001c4) /* 113 1c4 User-defined vector 49		*/
#define V_user_50				CPU_EXC_VTBL(0x0001c8) /* 114 1c8 User-defined vector 50		*/
#define V_user_51				CPU_EXC_VTBL(0x0001cc) /* 115 1cc User-defined vector 51		*/
#define V_user_52				CPU_EXC_VTBL(0x0001d0) /* 116 1d0 User-defined vector 52		*/
#define V_user_53				CPU_EXC_VTBL(0x0001d4) /* 117 1d4 User-defined vector 53		*/
#define V_user_54				CPU_EXC_VTBL(0x0001d8) /* 118 1d8 User-defined vector 54		*/
#define V_user_55				CPU_EXC_VTBL(0x0001dc) /* 119 1dc User-defined vector 55		*/
#define V_user_56				CPU_EXC_VTBL(0x0001e0) /* 120 1e0 User-defined vector 56		*/
#define V_user_57				CPU_EXC_VTBL(0x0001e4) /* 121 1e4 User-defined vector 57		*/
#define V_user_58				CPU_EXC_VTBL(0x0001e8) /* 122 1e8 User-defined vector 58		*/
#define V_user_59				CPU_EXC_VTBL(0x0001ec) /* 123 1ec User-defined vector 59		*/
#define V_user_60				CPU_EXC_VTBL(0x0001f0) /* 124 1f0 User-defined vector 60		*/
#define V_user_61				CPU_EXC_VTBL(0x0001f4) /* 125 1f4 User-defined vector 61		*/
#define V_user_62				CPU_EXC_VTBL(0x0001f8) /* 126 1f8 User-defined vector 62		*/
#define V_user_63				CPU_EXC_VTBL(0x0001fc) /* 127 1fc User-defined vector 63		*/
#define V_user_64				CPU_EXC_VTBL(0x000200) /* 128 200 User-defined vector 64		*/
#define V_user_65				CPU_EXC_VTBL(0x000204) /* 129 204 User-defined vector 65		*/
#define V_user_66				CPU_EXC_VTBL(0x000208) /* 130 208 User-defined vector 66		*/
#define V_user_67				CPU_EXC_VTBL(0x00020c) /* 131 20c User-defined vector 67		*/
#define V_user_68				CPU_EXC_VTBL(0x000210) /* 132 210 User-defined vector 68		*/
#define V_user_69				CPU_EXC_VTBL(0x000214) /* 133 214 User-defined vector 69		*/
#define V_user_70				CPU_EXC_VTBL(0x000218) /* 134 218 User-defined vector 70		*/
#define V_user_71				CPU_EXC_VTBL(0x00021c) /* 135 21c User-defined vector 71		*/
#define V_user_72				CPU_EXC_VTBL(0x000220) /* 136 220 User-defined vector 72		*/
#define V_user_73				CPU_EXC_VTBL(0x000224) /* 137 224 User-defined vector 73		*/
#define V_user_74				CPU_EXC_VTBL(0x000228) /* 138 228 User-defined vector 74		*/
#define V_user_75				CPU_EXC_VTBL(0x00022c) /* 139 22c User-defined vector 75		*/
#define V_user_76				CPU_EXC_VTBL(0x000230) /* 140 230 User-defined vector 76		*/
#define V_user_77				CPU_EXC_VTBL(0x000234) /* 141 234 User-defined vector 77		*/
#define V_user_78				CPU_EXC_VTBL(0x000238) /* 142 238 User-defined vector 78		*/
#define V_user_79				CPU_EXC_VTBL(0x00023c) /* 143 23c User-defined vector 79		*/
#define V_user_80				CPU_EXC_VTBL(0x000240) /* 144 240 User-defined vector 80		*/
#define V_user_81				CPU_EXC_VTBL(0x000244) /* 145 244 User-defined vector 81		*/
#define V_user_82				CPU_EXC_VTBL(0x000248) /* 146 248 User-defined vector 82		*/
#define V_user_83				CPU_EXC_VTBL(0x00024c) /* 147 24c User-defined vector 83		*/
#define V_user_84				CPU_EXC_VTBL(0x000250) /* 148 250 User-defined vector 84		*/
#define V_user_85				CPU_EXC_VTBL(0x000254) /* 149 254 User-defined vector 85		*/
#define V_user_86				CPU_EXC_VTBL(0x000258) /* 150 258 User-defined vector 86		*/
#define V_user_87				CPU_EXC_VTBL(0x00025c) /* 151 25c User-defined vector 87		*/
#define V_user_88				CPU_EXC_VTBL(0x000260) /* 152 260 User-defined vector 88		*/
#define V_user_89				CPU_EXC_VTBL(0x000264) /* 153 264 User-defined vector 89		*/
#define V_user_90				CPU_EXC_VTBL(0x000268) /* 154 268 User-defined vector 90		*/
#define V_user_91				CPU_EXC_VTBL(0x00026c) /* 155 26c User-defined vector 91		*/
#define V_user_92				CPU_EXC_VTBL(0x000270) /* 156 270 User-defined vector 92		*/
#define V_user_93				CPU_EXC_VTBL(0x000274) /* 157 274 User-defined vector 93		*/
#define V_user_94				CPU_EXC_VTBL(0x000278) /* 158 278 User-defined vector 94		*/
#define V_user_95				CPU_EXC_VTBL(0x00027c) /* 159 27c User-defined vector 95		*/
#define V_user_96				CPU_EXC_VTBL(0x000280) /* 160 280 User-defined vector 96		*/
#define V_user_97				CPU_EXC_VTBL(0x000284) /* 161 284 User-defined vector 97		*/
#define V_user_98				CPU_EXC_VTBL(0x000288) /* 162 288 User-defined vector 98		*/
#define V_user_99				CPU_EXC_VTBL(0x00028c) /* 163 28c User-defined vector 99		*/
#define V_user_100				CPU_EXC_VTBL(0x000290) /* 164 290 User-defined vector 100		*/
#define V_user_101				CPU_EXC_VTBL(0x000294) /* 165 294 User-defined vector 101		*/
#define V_user_102				CPU_EXC_VTBL(0x000298) /* 166 298 User-defined vector 102		*/
#define V_user_103				CPU_EXC_VTBL(0x00029c) /* 167 29c User-defined vector 103		*/
#define V_user_104				CPU_EXC_VTBL(0x0002a0) /* 168 2a0 User-defined vector 104		*/
#define V_user_105				CPU_EXC_VTBL(0x0002a4) /* 169 2a4 User-defined vector 105		*/
#define V_user_106				CPU_EXC_VTBL(0x0002a8) /* 170 2a8 User-defined vector 106		*/
#define V_user_107				CPU_EXC_VTBL(0x0002ac) /* 171 2ac User-defined vector 107		*/
#define V_user_108				CPU_EXC_VTBL(0x0002b0) /* 172 2b0 User-defined vector 108		*/
#define V_user_109				CPU_EXC_VTBL(0x0002b4) /* 173 2b4 User-defined vector 109		*/
#define V_user_110				CPU_EXC_VTBL(0x0002b8) /* 174 2b8 User-defined vector 110		*/
#define V_user_111				CPU_EXC_VTBL(0x0002bc) /* 175 2bc User-defined vector 111		*/
#define V_user_112				CPU_EXC_VTBL(0x0002c0) /* 176 2c0 User-defined vector 112		*/
#define V_user_113				CPU_EXC_VTBL(0x0002c4) /* 177 2c4 User-defined vector 113		*/
#define V_user_114				CPU_EXC_VTBL(0x0002c8) /* 178 2c8 User-defined vector 114		*/
#define V_user_115				CPU_EXC_VTBL(0x0002cc) /* 179 2cc User-defined vector 115		*/
#define V_user_116				CPU_EXC_VTBL(0x0002d0) /* 180 2d0 User-defined vector 116		*/
#define V_user_117				CPU_EXC_VTBL(0x0002d4) /* 181 2d4 User-defined vector 117		*/
#define V_user_118				CPU_EXC_VTBL(0x0002d8) /* 182 2d8 User-defined vector 118		*/
#define V_user_119				CPU_EXC_VTBL(0x0002dc) /* 183 2dc User-defined vector 119		*/
#define V_user_120				CPU_EXC_VTBL(0x0002e0) /* 184 2e0 User-defined vector 120		*/
#define V_user_121				CPU_EXC_VTBL(0x0002e4) /* 185 2e4 User-defined vector 121		*/
#define V_user_122				CPU_EXC_VTBL(0x0002e8) /* 186 2e8 User-defined vector 122		*/
#define V_user_123				CPU_EXC_VTBL(0x0002ec) /* 187 2ec User-defined vector 123		*/
#define V_user_124				CPU_EXC_VTBL(0x0002f0) /* 188 2f0 User-defined vector 124		*/
#define V_user_125				CPU_EXC_VTBL(0x0002f4) /* 189 2f4 User-defined vector 125		*/
#define V_user_126				CPU_EXC_VTBL(0x0002f8) /* 190 2f8 User-defined vector 126		*/
#define V_user_127				CPU_EXC_VTBL(0x0002fc) /* 191 2fc User-defined vector 127		*/
#define V_user_128				CPU_EXC_VTBL(0x000300) /* 192 300 User-defined vector 128		*/
#define V_user_129				CPU_EXC_VTBL(0x000304) /* 193 304 User-defined vector 129		*/
#define V_user_130				CPU_EXC_VTBL(0x000308) /* 194 308 User-defined vector 130		*/
#define V_user_131				CPU_EXC_VTBL(0x00030c) /* 195 30c User-defined vector 131		*/
#define V_user_132				CPU_EXC_VTBL(0x000310) /* 196 310 User-defined vector 132		*/
#define V_user_133				CPU_EXC_VTBL(0x000314) /* 197 314 User-defined vector 133		*/
#define V_user_134				CPU_EXC_VTBL(0x000318) /* 198 318 User-defined vector 134		*/
#define V_user_135				CPU_EXC_VTBL(0x00031c) /* 199 31c User-defined vector 135		*/
#define V_user_136				CPU_EXC_VTBL(0x000320) /* 200 320 User-defined vector 136		*/
#define V_user_137				CPU_EXC_VTBL(0x000324) /* 201 324 User-defined vector 137		*/
#define V_user_138				CPU_EXC_VTBL(0x000328) /* 202 328 User-defined vector 138		*/
#define V_user_139				CPU_EXC_VTBL(0x00032c) /* 203 32c User-defined vector 139		*/
#define V_user_140				CPU_EXC_VTBL(0x000330) /* 204 330 User-defined vector 140		*/
#define V_user_141				CPU_EXC_VTBL(0x000334) /* 205 334 User-defined vector 141		*/
#define V_user_142				CPU_EXC_VTBL(0x000338) /* 206 338 User-defined vector 142		*/
#define V_user_143				CPU_EXC_VTBL(0x00033c) /* 207 33c User-defined vector 143		*/
#define V_user_144				CPU_EXC_VTBL(0x000340) /* 208 340 User-defined vector 144		*/
#define V_user_145				CPU_EXC_VTBL(0x000344) /* 209 344 User-defined vector 145		*/
#define V_user_146				CPU_EXC_VTBL(0x000348) /* 210 348 User-defined vector 146		*/
#define V_user_147				CPU_EXC_VTBL(0x00034c) /* 211 34c User-defined vector 147		*/
#define V_user_148				CPU_EXC_VTBL(0x000350) /* 212 350 User-defined vector 148		*/
#define V_user_149				CPU_EXC_VTBL(0x000354) /* 213 354 User-defined vector 149		*/
#define V_user_150				CPU_EXC_VTBL(0x000358) /* 214 358 User-defined vector 150		*/
#define V_user_151				CPU_EXC_VTBL(0x00035c) /* 215 35c User-defined vector 151		*/
#define V_user_152				CPU_EXC_VTBL(0x000360) /* 216 360 User-defined vector 152		*/
#define V_user_153				CPU_EXC_VTBL(0x000364) /* 217 364 User-defined vector 153		*/
#define V_user_154				CPU_EXC_VTBL(0x000368) /* 218 368 User-defined vector 154		*/
#define V_user_155				CPU_EXC_VTBL(0x00036c) /* 219 36c User-defined vector 155		*/
#define V_user_156				CPU_EXC_VTBL(0x000370) /* 220 370 User-defined vector 156		*/
#define V_user_157				CPU_EXC_VTBL(0x000374) /* 221 374 User-defined vector 157		*/
#define V_user_158				CPU_EXC_VTBL(0x000378) /* 222 378 User-defined vector 158		*/
#define V_user_159				CPU_EXC_VTBL(0x00037c) /* 223 37c User-defined vector 159		*/
#define V_user_160				CPU_EXC_VTBL(0x000380) /* 224 380 User-defined vector 160		*/
#define V_user_161				CPU_EXC_VTBL(0x000384) /* 225 384 User-defined vector 161		*/
#define V_user_162				CPU_EXC_VTBL(0x000388) /* 226 388 User-defined vector 162		*/
#define V_user_163				CPU_EXC_VTBL(0x00038c) /* 227 38c User-defined vector 163		*/
#define V_user_164				CPU_EXC_VTBL(0x000390) /* 228 390 User-defined vector 164		*/
#define V_user_165				CPU_EXC_VTBL(0x000394) /* 229 394 User-defined vector 165		*/
#define V_user_166				CPU_EXC_VTBL(0x000398) /* 230 398 User-defined vector 166		*/
#define V_user_167				CPU_EXC_VTBL(0x00039c) /* 231 39c User-defined vector 167		*/
#define V_user_168				CPU_EXC_VTBL(0x0003a0) /* 232 3a0 User-defined vector 168		*/
#define V_user_169				CPU_EXC_VTBL(0x0003a4) /* 233 3a4 User-defined vector 169		*/
#define V_user_170				CPU_EXC_VTBL(0x0003a8) /* 234 3a8 User-defined vector 170		*/
#define V_user_171				CPU_EXC_VTBL(0x0003ac) /* 235 3ac User-defined vector 171		*/
#define V_user_172				CPU_EXC_VTBL(0x0003b0) /* 236 3b0 User-defined vector 172		*/
#define V_user_173				CPU_EXC_VTBL(0x0003b4) /* 237 3b4 User-defined vector 173		*/
#define V_user_174				CPU_EXC_VTBL(0x0003b8) /* 238 3b8 User-defined vector 174		*/
#define V_user_175				CPU_EXC_VTBL(0x0003bc) /* 239 3bc User-defined vector 175		*/
#define V_user_176				CPU_EXC_VTBL(0x0003c0) /* 240 3c0 User-defined vector 176		*/
#define V_user_177				CPU_EXC_VTBL(0x0003c4) /* 241 3c4 User-defined vector 177		*/
#define V_user_178				CPU_EXC_VTBL(0x0003c8) /* 242 3c8 User-defined vector 178		*/
#define V_user_179				CPU_EXC_VTBL(0x0003cc) /* 243 3cc User-defined vector 179		*/
#define V_user_180				CPU_EXC_VTBL(0x0003d0) /* 244 3d0 User-defined vector 180		*/
#define V_user_181				CPU_EXC_VTBL(0x0003d4) /* 245 3d4 User-defined vector 181		*/
#define V_user_182				CPU_EXC_VTBL(0x0003d8) /* 246 3d8 User-defined vector 182		*/
#define V_user_183				CPU_EXC_VTBL(0x0003dc) /* 247 3dc User-defined vector 183		*/
#define V_user_184				CPU_EXC_VTBL(0x0003e0) /* 248 3e0 User-defined vector 184		*/
#define V_user_185				CPU_EXC_VTBL(0x0003e4) /* 249 3e4 User-defined vector 185		*/
#define V_user_186				CPU_EXC_VTBL(0x0003e8) /* 250 3e8 User-defined vector 186		*/
#define V_user_187				CPU_EXC_VTBL(0x0003ec) /* 251 3ec User-defined vector 187		*/
#define V_user_188				CPU_EXC_VTBL(0x0003f0) /* 252 3f0 User-defined vector 188		*/
#define V_user_189				CPU_EXC_VTBL(0x0003f4) /* 253 3f4 User-defined vector 189		*/
#define V_user_190				CPU_EXC_VTBL(0x0003f8) /* 254 3f8 User-defined vector 190		*/
#define V_user_191				CPU_EXC_VTBL(0x0003fc) /* 255 3fc User-defined vector 191		*/


/*
	Install the default handlers for every exception.  Default behaviour for every exception is to
	report the exception and dump its stack frame, then halt the system.
*/
void __cpu_exc_install_default_handlers(void);


/*
	Default handler functions for all exceptions. See M68000 PRM page B-2 for information about
	this table and descriptions of each exception.
*/
void __cpu_exc_bus_error(const struct __mc68010_address_exc_frame f);
void __cpu_exc_address_error(const struct __mc68010_address_exc_frame f);

void __cpu_exc_generic(const struct __mc68010_exc_frame f);

IRQ_HANDLER_DECL(__cpu_trap_0);
IRQ_HANDLER_DECL(__cpu_trap_1);
IRQ_HANDLER_DECL(__cpu_trap_2);
IRQ_HANDLER_DECL(__cpu_trap_3);
IRQ_HANDLER_DECL(__cpu_trap_4);
IRQ_HANDLER_DECL(__cpu_trap_5);
IRQ_HANDLER_DECL(__cpu_trap_6);
IRQ_HANDLER_DECL(__cpu_trap_7);
IRQ_HANDLER_DECL(__cpu_trap_8);
IRQ_HANDLER_DECL(__cpu_trap_9);
IRQ_HANDLER_DECL(__cpu_trap_10);
IRQ_HANDLER_DECL(__cpu_trap_11);
IRQ_HANDLER_DECL(__cpu_trap_12);
IRQ_HANDLER_DECL(__cpu_trap_13);
IRQ_HANDLER_DECL(__cpu_trap_14);
IRQ_HANDLER_DECL(__cpu_trap_15);

#endif

