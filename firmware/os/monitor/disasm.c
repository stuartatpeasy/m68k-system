/*
	MC68000/68010 disassembler

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, 2011.
*/

#include "disasm.h"


int disassemble(unsigned short **p, char *str)
{
	char a1[32], a2[32];
	char *pf = NULL;

	const unsigned short instr = HTOP_SHORT(*(*p)++);
	const unsigned char bit7_6 = (instr >> 6) & 3,
						src_mode = (instr >> 3) & 0x7,
						src_reg = (instr & 0x7),
						dest_mode = (instr >> 6) & 0x7,
						dest_reg = (instr >> 9) & 0x7;

	ea_size_t size = ea_unsized;

	*str = '\0';

	int x;
	for(x = 0; x < 32; a1[x] = a2[x] = 0, ++x) ;

	switch(instr >> 12)
	{
	case 0x0:
		if(BIT(instr, 8) || (((instr >> 8) & 0xf) == 8))		/* static/dynamic bit / movep */
		{
			if(src_mode == 1)	/* movep */
			{
				const unsigned char dir = BIT(instr, 7),
									sz = BIT(instr, 6);

				pf = "movep";
				size = sz ? ea_long : ea_word;

				if(dir)		/* reg -> mem */
				{
					a1[0] = 'd';
					a1[1] = '0' + dest_reg;

					sprintf(a2, "%d(a%c)", (short) HTOP_SHORT(*(*p)++), '0' + src_reg);
				}
				else		/* mem -> reg */
				{
					sprintf(a1, "%d(a%c)", (short) HTOP_SHORT(*(*p)++), '0' + src_reg);

					a2[0] = 'd';
					a2[1] = '0' + dest_reg;
				}
			}
			else				/* static/dynamic bit */
			{
				switch(bit7_6)
				{
					case 0:	pf = "btst";	break;
					case 1:	pf = "bchg";	break;
					case 2:	pf = "bclr";	break;
					case 3:	pf = "bset";	break;
				}

				if(BIT(instr, 8))	/* dynamic bit */
				{
					size = src_mode ? ea_byte : ea_long;

					a1[0] = 'd';
					a1[1] = '0' + dest_reg;
				}
				else				/* static bit */
				{
					sprintf(a1, "#%d", HTOP_SHORT(*(*p)++) & 0xff);
				}
				ea(a2, src_mode, src_reg, p, size);
			}
		}
		else
		{
			switch((instr >> 9) & 0x7)
			{
				case 0:	pf = "ori";		break;
				case 1:	pf = "andi";	break;
				case 2:	pf = "subi";	break;
				case 3:	pf = "addi";	break;
				/* case 4 is "static bit", which is handled above */
				case 5:	pf = "eori";	break;
				case 6:	pf = "cmpi";	break;
				case 7: pf = "moves";	break;
			}

			if((src_mode == 7) && (src_reg == 4))		/* -> ccr/sr */
			{
				/* TODO: only andi/eori/ori are permitted here - validate this */
				if(BIT(instr, 6))
				{
					size = ea_word;
					a2[0] = 's'; a2[1] = 'r';
				}
				else
				{
					size = ea_byte;
					a2[0] = 'c'; a2[1] = 'c'; a2[2] = 'r';
				}
				ea(a1, EAMODE_IMM, EAMODE_IMM_IMM, p, size);
			}
			else
			{
				switch(bit7_6)
				{
					case 0:	size = ea_byte;			break;
					case 1:	size = ea_word;			break;
					case 2:	size = ea_long;			break;
					case 3:	size = ea_unsized;		break;	/* invalid */
				}

				if(size)
				{
					ea(a1, EAMODE_IMM, EAMODE_IMM_IMM, p, size);
					ea(a2, src_mode, src_reg, p, size);
				}
				else
					pf = NULL;
			}
		}
		break;

	case 0x1:
	case 0x2:
	case 0x3:
		switch(instr >> 12)
		{
			case 1:	size = ea_byte;	break;
			case 2:	size = ea_long;	break;
			case 3:	size = ea_word;	break;
		}

		if(dest_mode == 1)		/* movea */
		{
			if(size == ea_byte)
				break;		/* movea.b is not allowed */

			pf = "movea";
		}
		else
			pf = "move";

		ea(a1, src_mode, src_reg, p, size);
		ea(a2, dest_mode, dest_reg, p, size);
		break;

	case 0x4:
		if(BIT(instr, 8))
		{
			if(bit7_6 == 2)		/* chk */
			{
				pf = "chk";
				size = ea_word;
				a2[0] = 'd';
			}
			else				/* lea */
			{
				pf = "lea";
				size = ea_long;
				a2[0] = 'a';
			}
			ea(a1, src_mode, src_reg, p, size);
			a2[1] = '0' + dest_reg;
		}
		else
		{
			if(src_mode && (bit7_6 & 2) && ((dest_reg & 5) == 4))		/* movem */
			{
				pf = "movem";
				size = BIT(instr, 6) ? ea_long : ea_word;
				if(dest_reg == 4)		/* movem regs -> <ea> */
				{
					movem_regs(a1, HTOP_SHORT(*(*p)++), src_mode == 4);
					ea(a2, src_mode, src_reg, p, ea_long);
				}
				else					/* movem <ea> -> regs */
				{
					movem_regs(a2, HTOP_SHORT(*(*p)++), src_mode == 4);
					ea(a1, src_mode, src_reg, p, ea_long);
				}
				break;
			}

			if(bit7_6 == 3)
			{
				switch(dest_reg)
				{
				case 0:		/* move from sr */
					pf = "move";
					size = ea_word;
					a1[0] = 's';
					a1[1] = 'r';
					ea(a2, src_mode, src_reg, p, ea_long);
					break;

				case 1:		/* move from ccr */
					pf = "move";
					size = ea_word;
					a1[0] = 'c';
					a1[1] = 'c';
					a1[2] = 'r';
					ea(a2, src_mode, src_reg, p, ea_long);
					break;

				case 2:		/* move to ccr */
					pf = "move";
					size = ea_word;
					ea(a1, src_mode, src_reg, p, ea_long);
					a2[0] = 'c';
					a2[1] = 'c';
					a2[2] = 'r';
					break;

				case 3:		/* move to sr */
					pf = "move";
					size = ea_word;
					ea(a1, src_mode, src_reg, p, ea_long);
					a2[0] = 's';
					a2[1] = 'r';
					break;

				case 4:		/* ext.l */
					pf = "ext";
					size = ea_long;
					a1[0] = 'd';
					a1[1] = '0' + src_reg;
					break;

				case 5:		/* tas / illegal */
					if((src_mode == 7) && (src_reg == 4))
						pf = "illegal";
					else
					{
						pf = "tas";
						size = ea_byte;
						ea(a1, src_mode, src_reg, p, ea_long);
					}
					break;

				case 7:		/* jmp */
					pf = "jmp";
					ea(a1, src_mode, src_reg, p, ea_long);
					break;
				}
			}
			else
			{
				switch(bit7_6)
				{
					case 0: size = ea_byte;	break;
					case 1: size = ea_word;	break;
					case 2: size = ea_long;	break;
				}

				switch(dest_reg)
				{
				case 0:		/* negx */
					pf = "negx";
					ea(a1, src_mode, src_reg, p, ea_long);
					break;

				case 1:		/* clr */
					pf = "clr";
					ea(a1, src_mode, src_reg, p, ea_long);
					break;

				case 2:		/* neg */
					pf = "neg";
					ea(a1, src_mode, src_reg, p, ea_long);
					break;

				case 3:		/* not */
					pf = "not";
					ea(a1, src_mode, src_reg, p, ea_long);
					break;

				case 4:		/* nbcd / swap / pea / ext.w */
					if(size == ea_byte)			/* nbcd */
					{
						pf = "nbcd";
						break;
					}

					if(!src_mode)
					{
						if(size == ea_word)		/* swap */
						{
							pf = "swap";
							a1[0] = 'd';
							a1[1] = '0' + src_reg;
						}
						else					/* ext.w */
						{
							pf = "ext";
							size = ea_word;
							a1[0] = 'd';
							a1[1] = '0' + src_reg;
						}
					}
					else if(bit7_6 == 1)			/* pea */
					{
						pf = "pea";
						size = ea_long;
						ea(a1, src_mode, src_reg, p, ea_long);
					}
					break;

				case 5:		/* tst */
					pf = "tst";
					ea(a1, src_mode, src_reg, p, ea_long);
					break;

				case 7:		/* trap / link / unlk / move -> usp / move <- usp / reset / nop / stop / rte / rtd / rts / trapv / rtr / movec / jsr */
					if(bit7_6 == 1)
					{
						switch(src_mode)
						{
						case 0:						/* trap */
						case 1:
							pf = "trap";
							size = ea_unsized;
							sprintf(a1, "#%d", instr & 0xf);
							break;

						case 2:						/* link */
							pf = "link";
							size = ea_unsized;
							a1[0] = 'a';
							a1[1] = '0' + src_reg;
							sprintf(a2, "#%d", (short) HTOP_SHORT(*(*p)++));
							break;

						case 3:						/* unlk */
							pf = "unlk";
							size = ea_unsized;
							a1[0] = 'a';
							a1[1] = '0' + src_reg;
							break;

						case 4:						/* move -> usp */
							pf = "move";
							size = ea_long;
							a1[0] = 'a';
							a1[1] = '0' + src_reg;
							a2[0] = 'u';
							a2[1] = 's';
							a2[2] = 'p';
							break;

						case 5:						/* move <- usp */
							pf = "move";
							size = ea_long;
							a1[0] = 'u';
							a1[1] = 's';
							a1[2] = 'p';
							a2[0] = 'a';
							a2[1] = '0' + src_reg;
							break;

						case 6:						/* reset / nop / stop / rte / rtd / rts / trapv / rtr */
							size = ea_unsized;
							switch(src_reg)
							{
							case 0:	pf = "reset";	break;
							case 1:	pf = "nop";		break;
							case 2:
                                pf = "stop";
                                size = ea_unsized;
                                sprintf(a1, "#%d", (short) HTOP_SHORT(*(*p)++));
                                break;
							case 3:	pf = "rte";		break;
							case 4:	pf = "rtd";		break;
							case 5:	pf = "rts";		break;
							case 6:	pf = "trapv";	break;
							case 7:	pf = "rtr";		break;
							}
							break;

						case 7:						/* movec */
							pf = "movec";
							size = ea_long;
							if(BIT(instr, 0))			/* general reg -> control reg */
							{
								a1[0] = BIT(**p, 15) ? 'a' : 'd';
								a1[1] = '0' + (((**p) >> 12) & 0x7);
								switch(**p & 0xfff)
								{
									case 0x000:		a2[0] = 's'; a2[1] = 'f'; a2[2] = 'c';	break;
									case 0x001:		a2[0] = 'd'; a2[1] = 'f'; a2[2] = 'c';	break;
									case 0x800:		a2[0] = 'u'; a2[1] = 's'; a2[2] = 'p';	break;
									case 0x801:		a2[0] = 'v'; a2[1] = 'b'; a2[2] = 'r';	break;
									default:			/* invalid register */
										pf = NULL;
										break;
								}
							}
							else						/* control reg -> general reg */
							{
								switch(**p & 0xfff)
								{
									case 0x000:		a1[0] = 's'; a1[1] = 'f'; a1[2] = 'c';	break;
									case 0x001:		a1[0] = 'd'; a1[1] = 'f'; a1[2] = 'c';	break;
									case 0x800:		a1[0] = 'u'; a1[1] = 's'; a1[2] = 'p';	break;
									case 0x801:		a1[0] = 'v'; a1[1] = 'b'; a1[2] = 'r';	break;
									default:			/* invalid register */
										pf = NULL;
										break;
								}
								a2[0] = BIT(**p, 15) ? 'a' : 'd';
								a2[1] = '0' + (((**p) >> 12) & 0x7);
							}
							(*p)++;
							break;
						}
					}
					else if(bit7_6 == 2)			/* jsr */
					{
						pf = "jsr";
						size = ea_unsized;
						ea(a1, src_mode, src_reg, p, ea_long);
					}
					break;
				}

			}
		}
		break;

	case 0x5:
		if(bit7_6 == 3)			/* scc / dbcc */
		{
			if(src_mode == 1)		/* dbcc */
			{
				switch((instr >> 8) & 0xf)
				{
					case 0x0:	pf = "dbt";		break;
					case 0x1:	pf = "dbf";		break;
					case 0x2:	pf = "dbhi";	break;
					case 0x3:	pf = "dbls";	break;
					case 0x4:	pf = "dbcc";	break;
					case 0x5:	pf = "dbcs";	break;
					case 0x6:	pf = "dbne";	break;
					case 0x7:	pf = "dbeq";	break;
					case 0x8:	pf = "dbvc";	break;
					case 0x9:	pf = "dbvs";	break;
					case 0xa:	pf = "dbpl";	break;
					case 0xb:	pf = "dbmi";	break;
					case 0xc:	pf = "dbge";	break;
					case 0xd:	pf = "dblt";	break;
					case 0xe:	pf = "dbgt";	break;
					case 0xf:	pf = "dble";	break;
				}

				size = ea_word;
				a1[0] = 'd';
				a1[1] = '0' + src_reg;
				sprintf(a2, "%d", (short) HTOP_SHORT(*(*p)++));
			}
			else					/* scc */
			{
				switch((instr >> 8) & 0xf)
				{
					case 0x0:	pf = "st";	break;
					case 0x1:	pf = "sf";	break;
					case 0x2:	pf = "shi";	break;
					case 0x3:	pf = "sls";	break;
					case 0x4:	pf = "scc";	break;
					case 0x5:	pf = "scs";	break;
					case 0x6:	pf = "sne";	break;
					case 0x7:	pf = "seq";	break;
					case 0x8:	pf = "svc";	break;
					case 0x9:	pf = "svs";	break;
					case 0xa:	pf = "spl";	break;
					case 0xb:	pf = "smi";	break;
					case 0xc:	pf = "sge";	break;
					case 0xd:	pf = "slt";	break;
					case 0xe:	pf = "sgt";	break;
					case 0xf:	pf = "sle";	break;
				}

				size = ea_byte;
				ea(a1, src_mode, src_reg, p, ea_long);
			}
		}
		else					/* addq / subq */
		{
			if(BIT(instr, 8))		/* subq */
				pf = "subq";
			else					/* addq */
				pf = "addq";

			switch(bit7_6)
			{
				case 0:	size = ea_byte;	break;
				case 1:	size = ea_word;	break;
				case 2:	size = ea_long;	break;
			}

			a1[0] = '#';
			a1[1] = (dest_reg == 0) ? '8' : '0' + dest_reg;
			ea(a2, src_mode, src_reg, p, ea_long);
		}
		break;

	case 0x6:					/* bcc / bra / bsr */
		switch((instr >> 8) & 0xf)
		{
			case 0x0:	pf = "bra";	break;
			case 0x1:	pf = "bsr";	break;
			case 0x2:	pf = "bhi";	break;
			case 0x3:	pf = "bls";	break;
			case 0x4:	pf = "bcc";	break;
			case 0x5:	pf = "bcs";	break;
			case 0x6:	pf = "bne";	break;
			case 0x7:	pf = "beq";	break;
			case 0x8:	pf = "bvc";	break;
			case 0x9:	pf = "bvs";	break;
			case 0xa:	pf = "bpl";	break;
			case 0xb:	pf = "bmi";	break;
			case 0xc:	pf = "bge";	break;
			case 0xd:	pf = "blt";	break;
			case 0xe:	pf = "bgt";	break;
			case 0xf:	pf = "ble";	break;
		}

		if(!(instr & 0xff))
		{
			size = ea_word;
			sprintf(a1, "#%d", (short) HTOP_SHORT(*(*p)++));
		}
		else
		{
			size = ea_byte;
			sprintf(a1, "#%d", (char) (instr & 0xff));
		}
		break;

	case 0x7:
		if(!BIT(instr, 8))
		{
			pf = "moveq";
			size = ea_long;

			sprintf(a1, "#%d", instr & 0xff);

			a2[0] = 'd';
			a2[1] = '0' + dest_reg;
		}
		break;

	case 0xc:					/* and / mulu / abcd / exg / muls */
		if(dest_mode == 5)
		{
			if(src_mode == 0)			/* exg dx, dy */
			{
				pf = "exg";
				size = ea_long;
				a1[0] = a2[0] = 'd';
				a1[1] = '0' + dest_reg;
				a2[1] = '0' + src_reg;
				break;
			}
			else if(src_mode == 1)		/* exg ax, ay */
			{
				pf = "exg";
				size = ea_long;
				a1[0] = a2[0] = 'a';
				a1[1] = '0' + dest_reg;
				a2[1] = '0' + src_reg;
				break;
			}
		}
		else if((dest_mode == 6) && (src_mode == 1))
		{
			pf = "exg";
			size = ea_long;
			a1[0] = 'd';
			a1[1] = '0' + dest_reg;
			a2[0] = 'a';
			a2[1] = '0' + src_reg;
		}

	case 0x8:					/* or / divu / sbcd / divs */
		if((dest_mode == 3) || (dest_mode == 7))
		{
			pf = ((instr >> 12) == 0x8) ? ((dest_mode == 3) ? "divu" : "divs")
										: ((dest_mode == 3) ? "mulu" : "muls");
			size = ea_word;

			ea(a1, src_mode, src_reg, p, ea_word);
			a2[0] = 'd';
			a2[1] = '0' + dest_reg;
		}
		else
		{
			if((dest_mode == 4) && !(src_mode & 6))	/* abcd / sbcd */
			{
				pf = ((instr >> 12) == 0x8) ? "sbcd" : "abcd";
				size = ea_byte;
				if(BIT(instr, 3))
				{
					a1[0] = a2[0] = '-';
					a1[1] = a2[1] = '(';
					a1[2] = a2[2] = 'a';
					a1[4] = a2[4] = ')';

					a1[3] = '0' + src_reg;
					a2[3] = '0' + dest_reg;
				}
				else
				{
					a1[0] = 'd';
					a1[1] = '0' + src_reg;
					a2[0] = 'd';
					a2[1] = '0' + dest_reg;
				}
			}
			else									/* and / or */
			{
				pf = ((instr >> 12) == 0x8) ? "or" : "and";
				switch(bit7_6)
				{
					case 0:	size = ea_byte;	break;
					case 1:	size = ea_word;	break;
					case 2:	size = ea_long;	break;
				}

				if(BIT(instr, 8))		/* <ea>, Dn */
				{
					ea(a1, src_mode, src_reg, p, size);
					a2[0] = 'd';
					a2[1] = '0' + dest_reg;
				}
				else					/* Dn, <ea> */
				{
					a1[0] = 'd';
					a1[1] = '0' + dest_reg;
					ea(a2, src_mode, src_reg, p, size);
				}
			}
		}
		break;

	case 0x9:		/* sub / suba / subx */
	case 0xd:		/* add / adda / addx */
		if((instr & 0x00c0) == 0x00c0)	/* adda / suba */
		{
			pf = ((instr >> 12) == 0x9) ? "suba" : "adda";
			size = BIT(instr, 8) ? ea_long : ea_word;
			ea(a1, src_mode, src_reg, p, size);
			a2[0] = 'a';
			a2[1] = '0' + dest_reg;
		}
		else
		{
			switch(bit7_6)
			{
				case 0:	size = ea_byte;	break;
				case 1:	size = ea_word;	break;
				case 2:	size = ea_long;	break;
			}

			if((instr & 0x0130) == 0x0100)	/* addx / subx */
			{
				pf = ((instr >> 12) == 0x9) ? "subx" : "addx";
				if(src_mode & 0x1)
				{
					a1[0] = a2[0] = '-';
					a1[1] = a2[1] = '(';
					a1[2] = a2[2] = 'a';
					a1[4] = a2[4] = ')';

					a1[3] = '0' + src_reg;
					a1[3] = '0' + dest_reg;
				}
				else
				{
					a1[0] = a2[0] = 'd';

					a1[1] = '0' + src_reg;
					a2[1] = '0' + dest_reg;
				}
			}
			else							/* add / sub */
			{
				pf = ((instr >> 12) == 0x9) ? "sub" : "add";
				if(BIT(instr, 8))
				{
					a1[0] = 'd';
					a1[1] = '0' + dest_reg;
					ea(a2, src_mode, src_reg, p, size);
				}
				else
				{
					ea(a1, src_mode, src_reg, p, size);
					a2[0] = 'd';
					a2[1] = '0' + dest_reg;
				}
			}
		}
		break;

	case 0xb:		/* cmp / cmpa / cmpm / eor */
		if((instr & 0x00c0) == 0x00c0)		/* cmpa */
		{
			pf = "cmpa";
			size = BIT(instr, 8) ? ea_long : ea_word;
			ea(a1, src_mode, src_reg, p, size);
			a2[0] = 'a';
			a2[1] = '0' + dest_reg;
		}
		else
		{
			switch(bit7_6)
			{
				case 0:	size = ea_byte;	break;
				case 1:	size = ea_word;	break;
				case 2:	size = ea_long;	break;
			}

			if(BIT(instr, 8))
			{
				if(src_mode == 1)			/* cmpm */
				{
					pf = "cmpm";
					a1[0] = a2[0] = '(';
					a1[1] = a2[1] = 'a';
					a1[3] = a2[3] = ')';
					a1[4] = a2[4] = '+';

					a1[2] = '0' + src_reg;
					a2[2] = '0' + dest_reg;
				}
				else						/* eor */
				{
					pf = "eor";
					a1[0] = 'd';
					a1[1] = '0' + dest_reg;
					ea(a2, src_mode, src_reg, p, size);
				}
			}
			else							/* cmp */
			{
				pf = "cmp";
				ea(a1, src_mode, src_reg, p, size);
				a2[0] = 'd';
				a2[1] = '0' + dest_reg;
			}
		}
		break;

	case 0xe:		/* shift/rotate register/memory */
		if(BIT(instr, 8))
			switch(src_mode & 3)
			{
				case 0:	pf = "asl";		break;
				case 1:	pf = "lsl";		break;
				case 2:	pf = "roxl";	break;
				case 3:	pf = "rol";		break;
			}
		else
			switch(src_mode & 3)
			{
				case 0:	pf = "asr";		break;
				case 1:	pf = "lsr";		break;
				case 2:	pf = "roxr";	break;
				case 3:	pf = "ror";		break;
			}

		if(bit7_6 == 3)		/* memory */
		{
			size = ea_word;
			ea(a1, src_mode, src_reg, p, ea_long);
		}
		else				/* register */
		{
			switch(bit7_6)
			{
				case 0:	size = ea_byte;	break;
				case 1: size = ea_word;	break;
				case 2:	size = ea_long;	break;
			}

			/* immediate shift or register shift? */
			a1[0] = (src_mode & 4) ? 'd' : '#';
			a1[1] = '0' + dest_reg;

			a2[0] = 'd';
			a2[1] = '0' + src_reg;
		}
		break;
	}


	/* formulate instruction string */
	if(pf)
	{
		if(size)
			sprintf(str, "%s.%c", pf, size);
		else
			strcat(str, pf);


		if(*a1)
		{
			strcat(str, " ");
			strcat(str, a1);
			if(*a2)
			{
				strcat(str, ", ");
				strcat(str, a2);
			}
		}
		return 0;
	}

	strcat(str, "???");
	return 1;
}


char *ea(char *str, unsigned char mode, unsigned char reg, unsigned short **p, const ea_size_t sz)
{
	mode &= 0x7;
	reg &= 0x7;

	switch(mode)
	{
		case 0:		/* data register direct */
			str[0] = 'd';
			str[1] = '0' + reg;
			break;

		case 1:		/* address register direct */
			str[0] = 'a';
			str[1] = '0' + reg;
			break;

        case 3:		/* address register indirect with postincrement */
			str[4] = '+';
		case 2:		/* address register indirect */
			str[0] = '(';
			str[1] = 'a';
			str[2] = '0' + reg;
			str[3] = ')';
			break;

		case 4:		/* address register indirect with predecrement */
			str[0] = '-';
			str[1] = '(';
			str[2] = 'a';
			str[3] = '0' + reg;
			str[4] = ')';
			break;

		case 5:		/* address register indirect with displacement */
			sprintf(str, "%d(a%d)", (short) HTOP_SHORT(*((*p)++)), reg);
			break;

		case 6:		/* address register indirect with index */
			sprintf(str, "%d(a%d, %c%d)", BEW_DISPLACEMENT(**p), reg, BEW_DA(**p), BEW_REGISTER(**p));
			(*p)++;
			break;

		case 7:		/* absolute / program counter with displacement / immediate or status register */
			switch(reg)
			{
				case 0:		/* absolute short */
					sprintf(str, "%x", HTOP_SHORT(**p));
					(*p)++;
					break;

				case 1:		/* absolute long */
					sprintf(str, "%x", HTOP_INT(*((unsigned int *) *p)));
					*p += 2;
					break;

				case 2:		/* program counter with displacement */
					sprintf(str, "%d(pc)", (short) HTOP_SHORT(**p));
					(*p)++;
					break;

				case 3:		/* program counter with index */
					sprintf(str, "%d(pc, %c%d)", BEW_DISPLACEMENT(**p), BEW_DA(**p), BEW_REGISTER(**p));
					(*p)++;
					break;

				default:		/* immediate or status register */
					switch(sz)
					{
						case ea_byte:
							sprintf(str, "#%d", (char) (HTOP_SHORT(*((*p)++)) & 0xff));
							break;

						case ea_word:
							sprintf(str, "#%d", (short) HTOP_SHORT(*((*p)++)));
							break;

						case ea_long:
							sprintf(str, "#%d", (int) HTOP_INT(*((int *) *p)));
							*p += 2;
							break;

						case ea_sr:
							str[0] = 's';
							str[1] = 'r';
							break;

						default:
							strcat(str, "???");
							break;
					}
					break;
			}
			break;
	}

	return str;
}


void movem_regs(char *str, unsigned short regs, char mode)
{
	char c, type, any = 0;
	char current_bit, prev_bit;

	if(mode)
	{
		unsigned short r2 = 0;
		for(c = 16; c; --c, regs >>= 1)
			r2 = (r2 << 1) | (regs & 1);
		regs = r2;
	}

	for(type = 'd'; type >= 'a'; type -= 'd' - 'a')
	{
		char hyphen = 0;
		if(regs & 0xff)
		{
			if(any)
				*str++ = '/';

			for(c = prev_bit = any = 0; c < 8; regs >>= 1, ++c)
			{
				current_bit = regs & 1;
				if(current_bit && !prev_bit)
				{
					if(any)
						*str++ = ',';
					*str++ = type;
					*str++ = '0' + c;
					any = 1;
				}
				else if(!current_bit && prev_bit)
				{
					if(hyphen)
					{
						*str++ = type;
						*str++ = '0' + c - 1;
					}
					any = 1;
				}
				else if(current_bit && prev_bit)
				{
					if(!hyphen)
						*str++ = '-';
					hyphen = 1;
				}
				prev_bit = current_bit;
			}

			if(current_bit && hyphen)
			{
				*str++ = type;
				*str++ = '7';
			}
		}
		else regs >>= 8;
	}
}
