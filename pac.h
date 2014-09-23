#pragma once

#include <stdint.h>

#define PAC_MAGIC	0x50414320	// PAC
#define PAC_EOF		0x454F4620	// EOF
#define PAC_FOFFSET	2048		// plus 4 byte padding

typedef struct {
	uint32_t magic;
	uint32_t unused1;
	uint32_t file_count;
	char padding[0x210];
	char unknown[0x5e0];
} pac_hdr; // 0

typedef struct {
	char file_name[0x20];
	uint32_t file_size;
	uint32_t file_location;
} pac_file;

typedef struct {
	char padding[0x4];
	pac_file* files;
} pac_files; // 2048

typedef struct {
	uint32_t unknown;
	uint32_t eof;
} pac_eof;
