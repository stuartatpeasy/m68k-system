#include <string.h>
#include <stdio.h>
#include <math.h>

/*
	TODO
		rounding
		[done] remove 64-bit types
		add support for precision
		test denormals	- FAIL!
		[done] test +/-inf & nan
		[done] fix bug displaying 0
		[done] use four-byte writes when zeroing coeff[] & digit[]
		use size-specific local vars
*/


#define NDIGITS		(512)		/* must be a multiple of four */

void printdouble(const double d)
{
	int binary_point, coeff_lastdigit, coeff_firstdigit;
	char digit[NDIGITS], coeff[NDIGITS];

	/* Mantissa is in the bottom 52 bits of the 64-bit word */
	unsigned int mantissa_h = ((unsigned int *) &d)[1],
				 mantissa_l = ((unsigned int *) &d)[0];

	const unsigned char negative = mantissa_h >> 31;

	/*
		The exponent is stored in bits 52 - 62 of the double; it is biased by 1023.  The position
		of the binary point can therefore be calculated.
	*/
	binary_point = 52 - (((mantissa_h >> 20) & 0x7ff) - 1023);

	mantissa_h &= 0x0fffff;

	/* exp = 0x7ff (=1024 after removing bias) represents infinities and NaNs */
	if(binary_point == -972)
	{
		if(negative)
			putchar('-');

		if(mantissa_l || mantissa_h)
			puts("nan");
		else
			puts("inf");
		return;
	}

	/*
		((exp == -1023) && (mantissa == 0)) represents positive or negative zero. In this case,
		don't insert the implicit 1 into the mantissa.
	*/
	else if((binary_point != 1075) || mantissa_h || mantissa_l)
	{
		if(negative)
			putchar('-');			/* XXX */
		mantissa_h |= 0x100000;		/* add the implicit 1 at start of mantissa */
	}

	if(binary_point < 53)
	{
		/* An integer part exists */
		int i, bp = binary_point;
		unsigned int int_part_h = mantissa_h,
					 int_part_l = mantissa_l;

		/*
			The decimal representation of the integer part of the number is generated using a BCD
			shift-and-add algorithm.  Digits of the integer part accumulate in char digit[], which
			is sized to hold the largest representable integer (308 digits) plus a terminating \0.
			Another array, char coeff[], holds a BCD representation of a power of two.  The value
			in this array is doubled with every iteration of the shift-and-add loop.

			Both arrays are initialised to zeroes; the least-significant digit of char coeff[] is
			then set to 1.
		*/

		for(i = NDIGITS >> 2; i--;) ((unsigned int *) digit)[i] = ((unsigned int *) coeff)[i] = 0;
		coeff[NDIGITS - 1] = 1;

		/*
			If binary_point < 0; the mantissa does not fit into 52 bits.  This means that there is
			no representable fractional part of the number, and that the mantissa is followed by
			some number of implicit binary zeroes.  The situation is this:

				if(binary_point < 0)
					integer_part = mantissa x (2 ^ -binary_point)

			The first step in the process of generating a decimal representation of the integer
			part of such numbers involves setting the digits in char coeff[] to represent the value
			(2 ^ -binary_point).  This number will then be used in place of 1 as the starting
			coefficient in the shift-and-add loop.

			When generating the coefficient, the variable int coeff_firstdigit keeps track of the
			position of the first digit.  This value is used to avoid carrying out any unnecessary
			calculations in the main loop.
		*/
		if(binary_point > 0)
		{
			if(binary_point < 32)
			{
				const unsigned int mask = (1U << binary_point) - 1;
				int_part_l >>= binary_point;
				int_part_l |= (int_part_h & mask) << (32 - binary_point);
				int_part_h >>= binary_point;
			}
			else
			{
				int_part_l = int_part_h >> (binary_point - 32);
				int_part_h = 0;
			}
		}

		/*
			This is the shift-and-add loop.  In each iteration, if the lsb of the mantissa is set,
			the current coefficient is added to the accumulator in char digit[].  The coefficient
			is then doubled and the lsb of the mantissa is discarded.
		*/
		for(coeff_firstdigit = NDIGITS - 1; bp < 53; bp++)
		{
			if(bp >= 0)
			{
				if(int_part_l & 1)
				{
					/* add coefficient to accumulator */
					for(i = NDIGITS; i-- >= coeff_firstdigit;)
					{
						int i_;
						digit[i] += coeff[i];

						/* Resolve carry */
						for(i_ = i; i_; i_--)
							if(digit[i_] > 9)
							{
								digit[i_] -= 10;
								++digit[i_ - 1];
							}
							else break;
					}
				}

				/* Shift the 64-bit qty in int_part_h|l right one bit */
				int_part_l >>= 1;
				if(int_part_h & 1)
					int_part_l |= (1U << 31);
				int_part_h >>= 1;
			}

			coeff[NDIGITS - 1] <<= 1;
			for(i = NDIGITS - 1; i-- >= coeff_firstdigit;)
			{
				coeff[i] <<= 1;

				/* Resolve carry */
				if(coeff[i + 1] > 9)
				{
					coeff[i + 1] -= 10;
					++coeff[i];
				}
			}

			if(coeff[i + 1])
				--coeff_firstdigit;
		}

		while(!digit[coeff_firstdigit])
			++coeff_firstdigit;

{
char buf[1024];
for(i = coeff_firstdigit; i < NDIGITS; i++) buf[i - coeff_firstdigit] = '0' + digit[i];
buf[i - coeff_firstdigit] = 0;
printf(buf);
}

	} else putchar('0');

	if(binary_point > 0)
	{
		/* A fractional part exists */
		int i;

		for(i = NDIGITS >> 2; i--;) ((unsigned int *) digit)[i] = ((unsigned int *) coeff)[i] = 0;
		coeff[0] = 5;			/* i.e. 2^-1 = 0.5 */
		coeff_firstdigit = 0;
		coeff_lastdigit = 1;

		/* Left-align the fractional part in the 64-bit word */
		if(binary_point <= 32)
		{
			mantissa_h = mantissa_l << (32 - binary_point);
			mantissa_l = 0;
		}
		else if(binary_point < 64)
		{
			const unsigned int mask = ~((1U << (binary_point - 32)) - 1);
			mantissa_h <<= (64 - binary_point);
			mantissa_h |= (mantissa_l & mask) >> (binary_point - 32);
			mantissa_l <<= (64 - binary_point);
		}

		for(; (mantissa_h || mantissa_l) && binary_point; binary_point--)
		{
			if(binary_point < 65)
			{
				if(mantissa_h & (1U << 31))	/* is the top bit set? */
				{
					/* add coefficient to accumulator */
					digit[coeff_firstdigit] += coeff[coeff_firstdigit];
					for(i = coeff_firstdigit + 1; i < coeff_lastdigit; ++i)
					{
						int i_;
						digit[i] += coeff[i];
						/* Resolve carry */
						for(i_ = i; i_; i_--)
							if(digit[i_] > 9)
							{
								digit[i_] -= 10;
								++digit[i_ - 1];
							}
					}
				}

				/* Shift the 64-bit qty in mantissa_h|l left one bit */
				mantissa_h <<= 1;
				if(mantissa_l & (1U << 31))
					mantissa_h |= 1;
				mantissa_l <<= 1;
			}

			for(i = coeff_firstdigit; i < coeff_lastdigit; ++i)
			{
				if(coeff[i] & 1)
					coeff[i + 1] += 10;
				coeff[i] >>= 1;
			}
			coeff[i] = 5;	/* New coeff. will always end with a 5 */

			if(!coeff[coeff_firstdigit])
				++coeff_firstdigit;

			if(coeff_lastdigit < NDIGITS - 1)
				++coeff_lastdigit;
		}
	
		while(!digit[coeff_lastdigit - 1])
			--coeff_lastdigit;

		putchar('.');

{
char buf[1024];
for(i = 0; i < coeff_lastdigit; i++) buf[i] = '0' + digit[i];
buf[i] = 0;
printf(buf);
}
//		for(i = 0; i < coeff_lastdigit; i++) putchar('0' + digit[i]);
	}
}


void test(const double d)
{
	printf("system: %.512f\n    me: ", d);
	printdouble(d);
	puts("\n");
}


int main(int argc, char ** argv)
{
	double d = 1.0/256.0;
	while(d < 1e20)
	{
		test(d);
		d *= M_PI;
	}

	test(123456789012.0);
	test(9007199254740992.0);
	test(-M_PI * 1e-100);
	test(M_PI * 1e15);
	test(0.0);
	test(4.940656e-300);
	test(4.940656e-320);

	return 0;
}

