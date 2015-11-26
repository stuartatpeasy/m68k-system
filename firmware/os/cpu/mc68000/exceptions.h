#ifndef CPU_MC68000_EXCEPTIONS_H_INC
#define CPU_MC68000_EXCEPTIONS_H_INC
/*
	MC68010 exception-management function and macro declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.

	Note: this code assumes that the m68k vector table is based at address 0, i.e. the CPU VBR is
	always set to 0x00000000.
*/

#include <kernel/include/types.h>
#include <kernel/util/kutil.h>


/* CPU_EXC_VTBL: a vector table entry, i.e. the address in RAM of a particular exception vector. */
#define CPU_EXC_VTBL(x)	(*((vu32 *) ((x) << 2)))


/*
	CPU_EXC_VPTR: macro to be used when setting a vector table entry.  For example, to cause
	address	errors to be redirected to the function address_error, do this:

		V_address_error = VTBL_PTR(address_error);
*/
#define CPU_EXC_VPTR(x)	((u32) (x))


/*
	CPU_EXC_VPTR_SET(x, p): set vector number x to point at function p.
*/
#define CPU_EXC_VPTR_SET(x, p)	do { CPU_EXC_VTBL(x) = CPU_EXC_VPTR(p); } while(0)



/*
	Vector table definition
*/
typedef enum m68k_excvec
{
	V_ssp                       = 0,        /*   0 000 Supervisor stack pointer         */
	V_reset                     = 1,        /*   1 004 Reset initial program counter	*/
	V_bus_error                 = 2,        /*   2 008 Bus error						*/
	V_address_error             = 3,        /*   3 00c Address error					*/
	V_illegal_instruction       = 4,        /*   4 010 Illegal instruction			    */
	V_int_divide_by_zero        = 5,        /*   5 014 Integer divide by zero		    */
	V_chk_instruction           = 6,        /*   6 018 CHK/CHK2 instruction			    */
	V_trap                      = 7,        /*   7 01c FTRAPcc/TRAPcc/TRAPV instr.	    */
	V_privilege_violation       = 8,        /*   8 020 Privilege violation			    */
	V_trace                     = 9,        /*   9 024 Trace							*/
	V_line_1010_emulator        = 10,       /*  10 028 Line 1010 emulator			    */
	V_line_1111_emulator        = 11,       /*  11 02c Line 1111 emulator			    */
/*                                              12 030 [unassigned / reserved]		    */
	V_coproc_prot_violation     = 13,       /*  13 034 Coprocessor protocol violation   */
	V_format_error              = 14,       /*  14 038 Format error					    */
	V_uninit_interrupt          = 15,       /*  15 03c Unitialised interrupt			*/
/*                                              16 040 [unassigned / reserved]		    */
/*                                              17 044 [unassigned / reserved]		    */
/*                                              18 048 [unassigned / reserved]		    */
/*                                              19 04c [unassigned / reserved]		    */
/*                                              20 050 [unassigned / reserved]		    */
/*                                              21 054 [unassigned / reserved]		    */
/*                                              22 058 [unassigned / reserved]		    */
/*                                              23 05c [unassigned / reserved]		    */
	V_spurious_interrupt        = 24,       /*  24 060 Spurious interrupt			    */
	V_level_1_autovector        = 25,       /*  25 064 Level 1 interrupt autovector	    */
	V_level_2_autovector        = 26,       /*  26 068 Level 2 interrupt autovector	    */
	V_level_3_autovector        = 27,       /*  27 06c Level 3 interrupt autovector	    */
	V_level_4_autovector        = 28,       /*  28 070 Level 4 interrupt autovector	    */
	V_level_5_autovector        = 29,       /*  29 074 Level 5 interrupt autovector	    */
	V_level_6_autovector        = 30,       /*  30 078 Level 6 interrupt autovector	    */
	V_level_7_autovector        = 31,       /*  31 07c Level 7 interrupt autovector	    */
	V_trap_0                    = 32,       /*  32 080 Trap #0						    */
	V_trap_1                    = 33,       /*  33 084 Trap #1						    */
	V_trap_2                    = 34,       /*  34 088 Trap #2						    */
	V_trap_3                    = 35,       /*  35 08c Trap #3						    */
	V_trap_4                    = 36,       /*  36 090 Trap #4						    */
	V_trap_5                    = 37,       /*  37 094 Trap #5						    */
	V_trap_6                    = 38,       /*  38 098 Trap #6						    */
	V_trap_7                    = 39,       /*  39 09c Trap #7						    */
	V_trap_8                    = 40,       /*  40 0a0 Trap #8						    */
	V_trap_9                    = 41,       /*  41 0a4 Trap #9						    */
	V_trap_10                   = 42,       /*  42 0a8 Trap #10						    */
	V_trap_11                   = 43,       /*  43 0ac Trap #11						    */
	V_trap_12                   = 44,       /*  44 0b0 Trap #12						    */
	V_trap_13                   = 45,       /*  45 0b4 Trap #13						    */
	V_trap_14                   = 46,       /*  46 0b8 Trap #14						    */
	V_trap_15                   = 47,       /*  47 0bc Trap #15						    */
	V_fp_unordered_cond         = 48,       /*  48 0c0 FP bra/set on unordered cond.	*/
	V_fp_inexact_result         = 49,       /*  49 0c4 FP inexact result				*/
	V_fp_divide_by_zero         = 50,       /*  50 0c8 FP divide by zero				*/
	V_fp_underflow              = 51,       /*  51 0cc FP underflow					    */
	V_fp_operand_error          = 52,       /*  52 0d0 FP operand error				    */
	V_fp_overflow               = 53,       /*  53 0d4 FP overflow					    */
	V_fp_signaling_nan          = 54,       /*  54 0d8 FP signaling NaN				    */
	V_fp_unimpl_data_type       = 55,       /*  55 0dc FP unimplemented data type	    */
	V_mmu_config_error          = 56,       /*  56 0e0 MMU configuration error		    */
	V_mmu_illegal_operation     = 57,       /*  57 0e4 MMU illegal operation			*/
	V_mmu_access_violation      = 58,       /*  58 0e8 MMU access level violation	    */
/*                                              59 0ec [unassigned / reserved]		    */
/*                                              60 0f0 [unassigned / reserved]	        */
/*                                              61 0f4 [unassigned / reserved]	        */
/*                                              62 0f8 [unassigned / reserved]	        */
/*                                              63 0fc [unassigned / reserved]	        */
	V_user_0                    = 64,       /*  64 100 User-defined vector 0 		    */
	V_user_1                    = 65,       /*  65 104 User-defined vector 1			*/
	V_user_2                    = 66,       /*  66 108 User-defined vector 2			*/
	V_user_3                    = 67,       /*  67 10c User-defined vector 3			*/
	V_user_4                    = 68,       /*  68 110 User-defined vector 4			*/
	V_user_5                    = 69,       /*  69 114 User-defined vector 5			*/
	V_user_6                    = 70,       /*  70 118 User-defined vector 6			*/
	V_user_7                    = 71,       /*  71 11c User-defined vector 7			*/
	V_user_8                    = 72,       /*  72 120 User-defined vector 8			*/
	V_user_9                    = 73,       /*  73 124 User-defined vector 9			*/
	V_user_10                   = 74,       /*  74 128 User-defined vector 10		    */
	V_user_11                   = 75,       /*  75 12c User-defined vector 11		    */
	V_user_12                   = 76,       /*  76 130 User-defined vector 12		    */
	V_user_13                   = 77,       /*  77 134 User-defined vector 13		    */
	V_user_14                   = 78,       /*  78 138 User-defined vector 14		    */
	V_user_15                   = 79,       /*  79 13c User-defined vector 15		    */
	V_user_16                   = 80,       /*  80 140 User-defined vector 16		    */
	V_user_17                   = 81,       /*  81 144 User-defined vector 17		    */
	V_user_18                   = 82,       /*  82 148 User-defined vector 18		    */
	V_user_19                   = 83,       /*  83 14c User-defined vector 19		    */
	V_user_20                   = 84,       /*  84 150 User-defined vector 20		    */
	V_user_21                   = 85,       /*  85 154 User-defined vector 21		    */
	V_user_22                   = 86,       /*  86 158 User-defined vector 22		    */
	V_user_23                   = 87,       /*  87 15c User-defined vector 23		    */
	V_user_24                   = 88,       /*  88 160 User-defined vector 24		    */
	V_user_25                   = 89,       /*  89 164 User-defined vector 25		    */
	V_user_26                   = 90,       /*  90 168 User-defined vector 26		    */
	V_user_27                   = 91,       /*  91 16c User-defined vector 27		    */
	V_user_28                   = 92,       /*  92 170 User-defined vector 28		    */
	V_user_29                   = 93,       /*  93 174 User-defined vector 29		    */
	V_user_30                   = 94,       /*  94 178 User-defined vector 30		    */
	V_user_31                   = 95,       /*  95 17c User-defined vector 31		    */
	V_user_32                   = 96,       /*  96 180 User-defined vector 32		    */
	V_user_33                   = 97,       /*  97 184 User-defined vector 33		    */
	V_user_34                   = 98,       /*  98 188 User-defined vector 34		    */
	V_user_35                   = 99,       /*  99 18c User-defined vector 35		    */
	V_user_36                   = 100,      /* 100 190 User-defined vector 36		    */
	V_user_37                   = 101,      /* 101 194 User-defined vector 37		    */
	V_user_38                   = 102,      /* 102 198 User-defined vector 38		    */
	V_user_39                   = 103,      /* 103 19c User-defined vector 39		    */
	V_user_40                   = 104,      /* 104 1a0 User-defined vector 40		    */
	V_user_41                   = 105,      /* 105 1a4 User-defined vector 41		    */
	V_user_42                   = 106,      /* 106 1a8 User-defined vector 42		    */
	V_user_43                   = 107,      /* 107 1ac User-defined vector 43		    */
	V_user_44                   = 108,      /* 108 1b0 User-defined vector 44		    */
	V_user_45                   = 109,      /* 109 1b4 User-defined vector 45		    */
	V_user_46                   = 110,      /* 110 1b8 User-defined vector 46		    */
	V_user_47                   = 111,      /* 111 1bc User-defined vector 47		    */
	V_user_48                   = 112,      /* 112 1c0 User-defined vector 48		    */
	V_user_49                   = 113,      /* 113 1c4 User-defined vector 49		    */
	V_user_50                   = 114,      /* 114 1c8 User-defined vector 50		    */
	V_user_51                   = 115,      /* 115 1cc User-defined vector 51		    */
	V_user_52                   = 116,      /* 116 1d0 User-defined vector 52		    */
	V_user_53                   = 117,      /* 117 1d4 User-defined vector 53		    */
	V_user_54                   = 118,      /* 118 1d8 User-defined vector 54		    */
	V_user_55                   = 119,      /* 119 1dc User-defined vector 55		    */
	V_user_56                   = 120,      /* 120 1e0 User-defined vector 56		    */
	V_user_57                   = 121,      /* 121 1e4 User-defined vector 57		    */
	V_user_58                   = 122,      /* 122 1e8 User-defined vector 58		    */
	V_user_59                   = 123,      /* 123 1ec User-defined vector 59		    */
	V_user_60                   = 124,      /* 124 1f0 User-defined vector 60		    */
	V_user_61                   = 125,      /* 125 1f4 User-defined vector 61		    */
	V_user_62                   = 126,      /* 126 1f8 User-defined vector 62		    */
	V_user_63                   = 127,      /* 127 1fc User-defined vector 63		    */
	V_user_64                   = 128,      /* 128 200 User-defined vector 64		    */
	V_user_65                   = 129,      /* 129 204 User-defined vector 65		    */
	V_user_66                   = 130,      /* 130 208 User-defined vector 66		    */
	V_user_67                   = 131,      /* 131 20c User-defined vector 67		    */
	V_user_68                   = 132,      /* 132 210 User-defined vector 68		    */
	V_user_69                   = 133,      /* 133 214 User-defined vector 69		    */
	V_user_70                   = 134,      /* 134 218 User-defined vector 70		    */
	V_user_71                   = 135,      /* 135 21c User-defined vector 71		    */
	V_user_72                   = 136,      /* 136 220 User-defined vector 72		    */
	V_user_73                   = 137,      /* 137 224 User-defined vector 73		    */
	V_user_74                   = 138,      /* 138 228 User-defined vector 74		    */
	V_user_75                   = 139,      /* 139 22c User-defined vector 75		    */
	V_user_76                   = 140,      /* 140 230 User-defined vector 76		    */
	V_user_77                   = 141,      /* 141 234 User-defined vector 77		    */
	V_user_78                   = 142,      /* 142 238 User-defined vector 78		    */
	V_user_79                   = 143,      /* 143 23c User-defined vector 79		    */
	V_user_80                   = 144,      /* 144 240 User-defined vector 80		    */
	V_user_81                   = 145,      /* 145 244 User-defined vector 81		    */
	V_user_82                   = 146,      /* 146 248 User-defined vector 82		    */
	V_user_83                   = 147,      /* 147 24c User-defined vector 83		    */
	V_user_84                   = 148,      /* 148 250 User-defined vector 84		    */
	V_user_85                   = 149,      /* 149 254 User-defined vector 85		    */
	V_user_86                   = 150,      /* 150 258 User-defined vector 86		    */
	V_user_87                   = 151,      /* 151 25c User-defined vector 87		    */
	V_user_88                   = 152,      /* 152 260 User-defined vector 88		    */
	V_user_89                   = 153,      /* 153 264 User-defined vector 89		    */
	V_user_90                   = 154,      /* 154 268 User-defined vector 90		    */
	V_user_91                   = 155,      /* 155 26c User-defined vector 91		    */
	V_user_92                   = 156,      /* 156 270 User-defined vector 92		    */
	V_user_93                   = 157,      /* 157 274 User-defined vector 93		    */
	V_user_94                   = 158,      /* 158 278 User-defined vector 94		    */
	V_user_95                   = 159,      /* 159 27c User-defined vector 95		    */
	V_user_96                   = 160,      /* 160 280 User-defined vector 96		    */
	V_user_97                   = 161,      /* 161 284 User-defined vector 97		    */
	V_user_98                   = 162,      /* 162 288 User-defined vector 98	    	*/
	V_user_99                   = 163,      /* 163 28c User-defined vector 99		    */
	V_user_100                  = 164,      /* 164 290 User-defined vector 100		    */
	V_user_101                  = 165,      /* 165 294 User-defined vector 101		    */
	V_user_102                  = 166,      /* 166 298 User-defined vector 102		    */
	V_user_103                  = 167,      /* 167 29c User-defined vector 103		    */
	V_user_104                  = 168,      /* 168 2a0 User-defined vector 104		    */
	V_user_105                  = 169,      /* 169 2a4 User-defined vector 105		    */
	V_user_106                  = 170,      /* 170 2a8 User-defined vector 106		    */
	V_user_107                  = 171,      /* 171 2ac User-defined vector 107		    */
	V_user_108                  = 172,      /* 172 2b0 User-defined vector 108		    */
	V_user_109                  = 173,      /* 173 2b4 User-defined vector 109		    */
	V_user_110                  = 174,      /* 174 2b8 User-defined vector 110		    */
	V_user_111                  = 175,      /* 175 2bc User-defined vector 111		    */
	V_user_112                  = 176,      /* 176 2c0 User-defined vector 112		    */
	V_user_113                  = 177,      /* 177 2c4 User-defined vector 113		    */
	V_user_114                  = 178,      /* 178 2c8 User-defined vector 114		    */
	V_user_115                  = 179,      /* 179 2cc User-defined vector 115		    */
	V_user_116                  = 180,      /* 180 2d0 User-defined vector 116		    */
	V_user_117                  = 181,      /* 181 2d4 User-defined vector 117		    */
	V_user_118                  = 182,      /* 182 2d8 User-defined vector 118		    */
	V_user_119                  = 183,      /* 183 2dc User-defined vector 119		    */
	V_user_120                  = 184,      /* 184 2e0 User-defined vector 120		    */
	V_user_121                  = 185,      /* 185 2e4 User-defined vector 121		    */
	V_user_122                  = 186,      /* 186 2e8 User-defined vector 122		    */
	V_user_123                  = 187,      /* 187 2ec User-defined vector 123		    */
	V_user_124                  = 188,      /* 188 2f0 User-defined vector 124		    */
	V_user_125                  = 189,      /* 189 2f4 User-defined vector 125		    */
	V_user_126                  = 190,      /* 190 2f8 User-defined vector 126		    */
	V_user_127                  = 191,      /* 191 2fc User-defined vector 127		    */
	V_user_128                  = 192,      /* 192 300 User-defined vector 128		    */
	V_user_129                  = 193,      /* 193 304 User-defined vector 129		    */
	V_user_130                  = 194,      /* 194 308 User-defined vector 130		    */
	V_user_131                  = 195,      /* 195 30c User-defined vector 131		    */
	V_user_132                  = 196,      /* 196 310 User-defined vector 132		    */
	V_user_133                  = 197,      /* 197 314 User-defined vector 133		    */
	V_user_134                  = 198,      /* 198 318 User-defined vector 134		    */
	V_user_135                  = 199,      /* 199 31c User-defined vector 135		    */
	V_user_136                  = 200,      /* 200 320 User-defined vector 136		    */
	V_user_137                  = 201,      /* 201 324 User-defined vector 137		    */
	V_user_138                  = 202,      /* 202 328 User-defined vector 138		    */
	V_user_139                  = 203,      /* 203 32c User-defined vector 139		    */
	V_user_140                  = 204,      /* 204 330 User-defined vector 140		    */
	V_user_141                  = 205,      /* 205 334 User-defined vector 141		    */
	V_user_142                  = 206,      /* 206 338 User-defined vector 142		    */
	V_user_143                  = 207,      /* 207 33c User-defined vector 143		    */
	V_user_144                  = 208,      /* 208 340 User-defined vector 144		    */
	V_user_145                  = 209,      /* 209 344 User-defined vector 145		    */
	V_user_146                  = 210,      /* 210 348 User-defined vector 146		    */
	V_user_147                  = 211,      /* 211 34c User-defined vector 147		    */
	V_user_148                  = 212,      /* 212 350 User-defined vector 148		    */
	V_user_149                  = 213,      /* 213 354 User-defined vector 149		    */
	V_user_150                  = 214,      /* 214 358 User-defined vector 150		    */
	V_user_151                  = 215,      /* 215 35c User-defined vector 151		    */
	V_user_152                  = 216,      /* 216 360 User-defined vector 152		    */
	V_user_153                  = 217,      /* 217 364 User-defined vector 153		    */
	V_user_154                  = 218,      /* 218 368 User-defined vector 154		    */
	V_user_155                  = 219,      /* 219 36c User-defined vector 155		    */
	V_user_156                  = 220,      /* 220 370 User-defined vector 156		    */
	V_user_157                  = 221,      /* 221 374 User-defined vector 157		    */
	V_user_158                  = 222,      /* 222 378 User-defined vector 158		    */
	V_user_159                  = 223,      /* 223 37c User-defined vector 159		    */
	V_user_160                  = 224,      /* 224 380 User-defined vector 160		    */
	V_user_161                  = 225,      /* 225 384 User-defined vector 161		    */
	V_user_162                  = 226,      /* 226 388 User-defined vector 162		    */
	V_user_163                  = 227,      /* 227 38c User-defined vector 163		    */
	V_user_164                  = 228,      /* 228 390 User-defined vector 164		    */
	V_user_165                  = 229,      /* 229 394 User-defined vector 165		    */
	V_user_166                  = 230,      /* 230 398 User-defined vector 166		    */
	V_user_167                  = 231,      /* 231 39c User-defined vector 167		    */
	V_user_168                  = 232,      /* 232 3a0 User-defined vector 168		    */
	V_user_169                  = 233,      /* 233 3a4 User-defined vector 169		    */
	V_user_170                  = 234,      /* 234 3a8 User-defined vector 170		    */
	V_user_171                  = 235,      /* 235 3ac User-defined vector 171		    */
	V_user_172                  = 236,      /* 236 3b0 User-defined vector 172		    */
	V_user_173                  = 237,      /* 237 3b4 User-defined vector 173		    */
	V_user_174                  = 238,      /* 238 3b8 User-defined vector 174		    */
	V_user_175                  = 239,      /* 239 3bc User-defined vector 175		    */
	V_user_176                  = 240,      /* 240 3c0 User-defined vector 176		    */
	V_user_177                  = 241,      /* 241 3c4 User-defined vector 177		    */
	V_user_178                  = 242,      /* 242 3c8 User-defined vector 178		    */
	V_user_179                  = 243,      /* 243 3cc User-defined vector 179		    */
	V_user_180                  = 244,      /* 244 3d0 User-defined vector 180		    */
	V_user_181                  = 245,      /* 245 3d4 User-defined vector 181		    */
	V_user_182                  = 246,      /* 246 3d8 User-defined vector 182		    */
	V_user_183                  = 247,      /* 247 3dc User-defined vector 183		    */
	V_user_184                  = 248,      /* 248 3e0 User-defined vector 184		    */
	V_user_185                  = 249,      /* 249 3e4 User-defined vector 185		    */
	V_user_186                  = 250,      /* 250 3e8 User-defined vector 186		    */
	V_user_187                  = 251,      /* 251 3ec User-defined vector 187		    */
	V_user_188                  = 252,      /* 252 3f0 User-defined vector 188		    */
	V_user_189                  = 253,      /* 253 3f4 User-defined vector 189		    */
	V_user_190                  = 254,      /* 254 3f8 User-defined vector 190		    */
	V_user_191                  = 255,      /* 255 3fc User-defined vector 191		    */
} m68k_excvec_t;

#endif
