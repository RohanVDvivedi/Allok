#ifndef MEM_MAN_H
#define MEM_MAN_H

#include<stdlib.h>

void allok_init(int debugL);

void* allok(size_t size);

void freek(void* mptr);

#endif