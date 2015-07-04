#include "tache.h"


int __main()
{
	os_free(12345678);

	if(os_malloc(16384))
		return 0;
	
	return 1;
}

