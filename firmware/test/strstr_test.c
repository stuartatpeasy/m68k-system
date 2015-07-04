#include <stdio.h>
#include "include/string.h"

void strstrtest(const char *haystack, const char *needle)
{
	char *r = strstr(haystack, needle);
	printf("(%p) n = '%s'\n(%p) h = '%s'\n", needle, needle, haystack, haystack);
	printf("result = %p\n\n", r);
}


int main()
{
	strstrtest("Wonderful!", "rful");
	strstrtest("Wonderful!", "blah");
	strstrtest("Wonderfuerferuererful!", "rful");
	strstrtest("Wonderfuerferuererfu", "rful");

	return 0;
}

