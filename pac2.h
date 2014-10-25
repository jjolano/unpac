#pragma once

#include <stdint.h>

#define PAC2_MAGIC1	0x47414D4544415420	// GAMEDAT
#define PAC2_MAGIC2	0x50414332			// PAC2

typedef struct {
	uint64_t magic1;
	uint32_t magic2;
	uint32_t file_count;
} pac2_hdr;

typedef struct {
	char file_name[0x20];
} pac2_file;

typedef struct {
	uint32_t file_location;
	uint32_t file_size;
} pac2_fileinfo;
