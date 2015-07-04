#include <stdio.h>
#include <stdlib.h>

#define NWORDS 9

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef unsigned int u32;
typedef const unsigned int ku32;

typedef union
{
	unsigned int i;
	float f;
} uint_float;


void bcd_double()
{
	u32 num = 0x00000001;
	u32 carry = 0;

	int i;
	for(i = 0; i < 27; ++i)
	{
		unsigned int t1, t2, t3, t4, t5, t6;

		if(num >= 0x50000000)
		{
			num -= 0x50000000;
			carry = 1;
		}

		t1 = num + 0x06666666;
		t2 = t1 + num;
		t3 = t1 ^ num;
		t4 = t2 ^ t3;
		t5 = ~t4 & 0x11111110;
		t6 = (t5 >> 2) | (t5 >> 3);

		num = t2 - t6;
		printf("%08x\n", num);
	}
}


/* Packed BCD full adder */
void bcd_add_array(u32 *a, ku32 *b, int end)
{
	unsigned int carry = 0;

	for(; end >= 0; --end)
	{
		unsigned int t1, t2, t3, t4, t5, t6, sum;

		/* Step 1: sum = carry_in ? ++a : a */
		if(carry)
		{
			if(a[end] == 0x99999999)
			{
				sum = 0;
			}
			else
			{
				t1 = a[end] + 0x06666666;
				t2 = t1 + 1;
				t3 = t1 ^ 1;
				t4 = t2 ^ t3;
				t5 = ~t4 & 0x11111110;
				t6 = (t5 >> 2) | (t5 >> 3);

				sum = t2 - t6;
				carry = 0;
			}
		}
		else
		{
			sum = a[end];
		}

		/* Step 2: sum += b */
		t1 = sum + 0x06666666;
		t2 = t1 + b[end];
		t3 = t1 ^ b[end];
		t4 = t2 ^ t3;
		t5 = ~t4 & 0x11111110;
		t6 = (t5 >> 2) | (t5 >> 3);

		sum = t2 - t6;
		if((sum < b[end]) || (sum > 0x99999999))
		{
			carry = 1;
			sum -= 0xa0000000;
		}

		a[end] = sum;
	}
}


/* Divide a 32-bit packed BCD number by 2 */
#define BCD_DIV2(n)							\
		(__extension__ ({					\
			unsigned int t1, t2, t3;		\
			t1 = (n) >> 1;					\
			t2 = t1 & 0x88888888;			\
			t3 = (t2 >> 2) | (t2 >> 3);		\
			t1 - t3;						\
		}))


/* Divide a 32-bit packed BCD number by 2 with borrow */
#define BCD_DIV2_BORROW(n, borrow)			\
		(__extension__ ({					\
			unsigned int t1, t2, t3;		\
			t1 = ((n) >> 1) | (borrow);		\
			t2 = t1 & 0x88888888;			\
			t3 = (t2 >> 2) | (t2 >> 3);		\
			t1 - t3;						\
		}))


unsigned int bcd_div_2(const unsigned int n, unsigned int borrow)
{
	unsigned int t1, t2, t3;
	
	t1 = (n >> 1) | borrow;
	t2 = t1 & 0x88888888;
	t3 = (t2 >> 2) | (t2 >> 3);

	return t1 - t3;
}


void bcd_print(const unsigned int a)
{
	int x;

	for(x = 7; x >= 0; --x)
		printf("%c", '0' + ((a >> (4 * x)) & 0xf));
}


void table()
{
	unsigned int i, j, x[NWORDS], start, end;

	x[0] = 0x50000000;
	for(i = 1; i < NWORDS; ++i)
		x[i] = 0;

	for(start = 0, j = 1; j <= 127; ++j)
	{
		end = j >> 3;

		for(i = end; i > start; --i)
			x[i] = BCD_DIV2_BORROW(x[i], x[i - 1] << 31);
		x[start] = BCD_DIV2(x[start]);

		if(!x[start])
			++start;

		// ----------------------------------
		printf("%3d: (%2d-%2d) ", j + 1, start, end);
		for(i = 0; i < start; ++i)
			printf(" -------");

		for(i = start; i <= end; ++i)
			bcd_print(x[i]);

		for(i = end + 1; i < NWORDS; ++i)
			printf("------- ");

		putchar('\n');
		// ----------------------------------
	}
}


/* TODO: fix memory leak: malloc()ed blocks may not all be freed */
int print_float(const float f)
{
	unsigned int *accum, *pow2;
	uint_float x;

	x.f = f;

	int exponent = ((x.i & 0x7f800000) >> 23) - 127;
	unsigned int significand = (x.i & 0x007fffff) | 0x00800000;
	printf("The number is %.32f (0x%08x) mantissa=0x%08x exponent=%d\n", x.f, x.i, significand, exponent);

	if(exponent == 128)
	{
		if(significand)
		{
			/* NaN */
		}
		else
		{
			/* infinity */
		}
	}
	else if(exponent == -127)
	{
		if(significand)
		{
			/* Subnormal number */
		}
		else
		{
			/* +/- zero */
		}
	}
	else
	{
		/* Normalised number */
		int i;

		if(exponent > 0)
		{
			/* INTEGER PART */
			const int int_nbcdwords = 1 + (exponent / 26);
		}

		/*
			FRACTIONAL PART
		*/
		const int frac_nbcdwords = ((22 - exponent) + 7) >> 3;

		pow2 = malloc(frac_nbcdwords * sizeof(unsigned int));
		if(pow2 == NULL)
			return -1;

		accum = malloc(frac_nbcdwords * sizeof(unsigned int));
		if(accum == NULL)
			return -1;

		pow2[0] = 0x50000000;		/* 2^-1 */
		accum[0] = 0;
		for(i = 1; i < NWORDS; ++i)
			pow2[i] = accum[i] = 0;

		/* iterate over bits starting from the first bit of the fractional part */
		unsigned int mask;
		int pow2_start = 0, pow2_end, npower = 0;

		/* initialise pow2 to 2^-exponent */
		for(; exponent < -1; ++exponent)
		{
			/* Calculate the next-lower power of 2 */
			int i;
			pow2_end = ++npower >> 3;

			for(i = pow2_end; i > pow2_start; --i)
				pow2[i] = BCD_DIV2_BORROW(pow2[i], pow2[i - 1] << 31);
			pow2[pow2_start] = BCD_DIV2(pow2[pow2_start]);

			if(!pow2[pow2_start])
				++pow2_start;

			printf("exponent=%4d ", exponent);
			for(i = 0; i < NWORDS; ++i)
				printf(" %08x", pow2[i]);
			putchar('\n');
		}


		for(mask = 1 << (22 - exponent); significand && mask; mask >>= 1)
		{
			printf("mask=%08x  bit=%c ", mask, (significand & mask) ? '1' : '0');

			pow2_end = ++npower >> 3;

			int i;
			for(i = 0; i < NWORDS; ++i)
			{ putchar(' '); bcd_print(pow2[i]); }

			if(significand & mask)
				bcd_add_array(accum, pow2, pow2_end);	/* accumulator += current power of two */

			putchar(' ');
			for(i = 0; i < NWORDS; ++i)
			{ putchar(' '); bcd_print(accum[i]); }
			putchar('\n');

			significand &= ~mask;
			if(significand)
			{
				/* Calculate the next-lower power of 2 */
				int i;
				for(i = pow2_end; i > pow2_start; --i)
					pow2[i] = BCD_DIV2_BORROW(pow2[i], pow2[i - 1] << 31);	
				pow2[pow2_start] = BCD_DIV2(pow2[pow2_start]);

				if(!pow2[pow2_start])
					++pow2_start;
			}
		}

		printf("\nfrac: .");
		for(i = 0; i < NWORDS; ++i)
		{
			putchar(' ');
			bcd_print(accum[i]);
		}
		putchar('\n');
	}

	return 0;
}


int main(int argc, char **argv)
{
	return print_float(1.5);
}

