#include <stdio.h>

#include "include/stdlib.h"
#include "include/limits.h"

int strtoltest(const char *const num, const int base, const long expected)
{
	const long val = strtol(num, NULL, base);
	printf("[%2d] '%s' => %ld %s\n", base, num, val, (val == expected) ? "" : "[FAILED]");

	return val == expected;
}


int main()
{
	int ok = 1;

	ok &= strtoltest("-123", 10, -123);
	ok &= strtoltest("-123", 0, -123);
	ok &= strtoltest("-123", 16, -291);
	ok &= strtoltest("0xc0ffee", 0, 12648430);
	ok &= strtoltest("c0ffee", 0, 0);
	ok &= strtoltest("-0123", 0, -83);
	ok &= strtoltest("9999999999", 0, 2147483647);
	ok &= strtoltest("-9999999999", 0, LONG_MIN);
	ok &= strtoltest("123d456", 0, 123);
	ok &= strtoltest("1bfz5j", 36, 79687351);
	ok &= strtoltest("    +964", 10, 964);

	puts(ok ? "\nAll tests passed." : "\nSome tests failed");

	return 0;
}

