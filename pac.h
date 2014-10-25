#pragma once

#include <stdint.h>

#define PAC_MAGIC	0x50414320	// PAC
#define PAC_OFFSET	2052

typedef struct {
	uint32_t magic;
	uint32_t file_count;
} pac_hdr;

typedef struct {
	char file_name[0x20];
	uint32_t file_size;
	uint32_t file_location;
} pac_file;
