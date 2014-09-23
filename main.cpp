// unpac - pac file extraction tool
// by jjolano

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "bytes.h"
#include "pac.h"

using namespace std;

int main(int argc, char* argv[])
{
	printf("unpac v0.1 by jjolano\n");
	
	if(argc < 2)
	{
		printf("usage: %s <pac> [outdir]\n", argv[0]);
		return 1;
	}
	
	// open file
	FILE* fp = fopen(argv[1], "rb");
	
	if(fp == NULL)
	{
		perror("error");
		return 1;
	}
	
	// init variables
	pac_hdr pac_hdr = {};
	pac_files pac_files = {};
	
	// read header, verify magic
	fread(&pac_hdr.magic, sizeof pac_hdr.magic, 1, fp);
	fread(&pac_hdr.unused1, sizeof pac_hdr.unused1, 1, fp);
	fread(&pac_hdr.file_count, sizeof pac_hdr.file_count, 1, fp);
	
	if(pac_hdr.magic != _ES32(PAC_MAGIC))
	{
		// not a pac file
		fprintf(stderr, "error: invalid pac file\n");
		return 1;
	}
	
	// create output directory
	printf("creating output directory ...\n");
	char dirname[32];
	
	if(argc > 2)
	{
		// pac filename as directory name
		strcpy(dirname, argv[2]);
	}
	else
	{
		// parameter as directory name
		int len = strlen(argv[1]);
		
		strncpy(dirname, argv[1], len - 4);
		dirname[len - 4] = '\0';
	}
	
	mkdir(dirname, 0777);
	
	// read file list
	printf("files to extract: %d\n", pac_hdr.file_count);
	int i;
	
	fseek(fp, PAC_FOFFSET + 4, SEEK_SET);
	
	for(i = 0; i < pac_hdr.file_count; i++)
	{
		pac_file pac_file = {};
		
		fread(&pac_file.file_name, sizeof pac_file.file_name, 1, fp);
		fread(&pac_file.file_size, sizeof pac_file.file_size, 1, fp);
		fread(&pac_file.file_location, sizeof pac_file.file_location, 1, fp);
		
		printf("%s (%d/%d) ... ", pac_file.file_name, (i + 1), pac_hdr.file_count);
		
		// extract the file
		long ipos = ftell(fp);
		char filename[64];
		
		sprintf(filename, "%s/%s", dirname, pac_file.file_name);
		
		char* buffer = (char*)malloc(pac_file.file_size);
		
		if(buffer == NULL)
		{
			printf("fail\n");
			continue;
		}
		
		fseek(fp, pac_file.file_location, SEEK_SET);
		fread(buffer, pac_file.file_size, 1, fp);
		
		// create file and write
		FILE* fpw = fopen(filename, "wb");
		
		if(fpw == NULL)
		{
			printf("fail\n");
			continue;
		}
		
		fwrite(buffer, 1, pac_file.file_size, fpw);
		fclose(fpw);
		
		free(buffer);
		fseek(fp, ipos, SEEK_SET);
		
		printf("ok\n");
	}
	
	fclose(fp);
	
	return 0;
}
