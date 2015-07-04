#ifndef _HARNESS_H_
#define _HARNESS_H_

#include "include/types.h"

void *kmalloc(u32 size);
void *kcalloc(ku32 nmemb, ku32 size);
void *krealloc(void *ptr, u32 size);
void kfree(void *ptr);
void *kmemcpy(void *dest, const void *src, u32 n);

#endif

