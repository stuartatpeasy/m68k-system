#include <stdio.h>

#include "include/stdlib.h"

int strtoultest(const char *const num, const int base, const long expected)
{
	const long val = strtoul(num, NULL, base);
	printf("[%2d] '%s' => %lu %s\n", base, num, val, (val == expected) ? "" : "[FAILED]");

	return val == expected;
}


int main()
{
	int ok = 1;
/*
	TODO: -ve numbers should return the negation of the result of the conversion represented as an unsigned val
*/

	ok &= strtoultest("-123", 10, -123);
	ok &= strtoultest("-123", 0, -123);
	ok &= strtoultest("-123", 16, -0x123);
	ok &= strtoultest("0xc0ffee", 0, 12648430);
	ok &= strtoultest("c0ffee", 0, 0);
	ok &= strtoultest("-0123", 0, -0123);
	ok &= strtoultest("9999999999", 0, 4294967295);
	ok &= strtoultest("123d456", 0, 123);
	ok &= strtoultest("1bfz5j", 36, 79687351);
	ok &= strtoultest("    +964", 10, 964);
	ok &= strtoultest("0xfedcba98", 0, 0xfedcba98);

	puts(ok ? "\nAll tests passed." : "\nSome tests failed");

	return 0;
}

