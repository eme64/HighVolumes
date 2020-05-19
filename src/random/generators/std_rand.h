#include <stdint.h>

#ifndef HEADER_PRNG_STD_H
#define HEADER_PRNG_STD_H

extern int rand_chunk_size;

void std_init(void *seed);

uint32_t std_rand();

// Chunked random values
void std_init_chunked(void *seed);

uint32_t std_rand_chunked();

void refill_chunk();

#endif // HEADER_PRNG_STD_H
