// unpac - pac file extraction tool
// by jjolano

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined _MSC_VER
#include <direct.h>
#endif

#include "bytes.h"
#include "pac.h"
#include "pac2.h"

using namespace std;

void unpac_v1(FILE* fp, char* dirname, pac_hdr* pac_hdr)
{
	// read file count
	fseek(fp, 8, SEEK_SET);
	fread(&pac_hdr->file_count, sizeof pac_hdr->file_count, 1, fp);
	
	int files = pac_hdr->file_count;
	
	cout << "files: " << files << "\n";
	
	// list and extract files
	fseek(fp, PAC_OFFSET, SEEK_SET);
	
	for(int i = 0; i < files; i++)
	{
		pac_file pac_file = {};
	
		// read file info
		fread(&pac_file.file_name, sizeof pac_file.file_name, 1, fp);
		fread(&pac_file.file_size, sizeof pac_file.file_size, 1, fp);
		fread(&pac_file.file_location, sizeof pac_file.file_location, 1, fp);
	
		// extract file data
		printf("extracting \"%s\" (%d/%d, %d bytes) ... ", pac_file.file_name, (i + 1), files, pac_file.file_size);
	
		// store file pointer position
		long position = ftell(fp);
	
		// relative filename
		char filename[64];
		sprintf(filename, "%s/%s", dirname, pac_file.file_name);
	
		// allocate memory for file
		char* buffer = (char*)malloc(pac_file.file_size);
	
		if(buffer == NULL)
		{
			cout << "fail\n";
			cerr << "warning: failed to allocate memory\n";
			continue;
		}
	
		// seek file pointer position to data offset
		fseek(fp, pac_file.file_location, SEEK_SET);
	
		// read data into allocated memory
		fread(buffer, pac_file.file_size, 1, fp);
	
		// create file and write data
		FILE* fpw = fopen(filename, "wb");
		
		if(fpw == NULL)
		{
			cout << "fail\n";
			cerr << "warning: failed to create file\n";
		
			free(buffer);
			continue;
		}
		
		fwrite(buffer, 1, pac_file.file_size, fpw);
		fclose(fpw);
		
		free(buffer);
	
		// restore file pointer position
		fseek(fp, position, SEEK_SET);
		
		cout << "ok\n";
	}
}

void unpac_v2(FILE* fp, char* dirname, pac2_hdr* pac2_hdr)
{
	// read file count
	fseek(fp, 12, SEEK_SET);
	fread(&pac2_hdr->file_count, sizeof pac2_hdr->file_count, 1, fp);
	
	int files = pac2_hdr->file_count;
	
	cout << "files: " << files << "\n";
	
	// extract files
	long fileinfo_offset = 0x10 + (0x20 * files);
	
	for(int i = 0; i < files; i++)
	{
		pac2_file pac2_file = {};
		pac2_fileinfo pac2_fileinfo = {};
	
		// read file name
		fread(&pac2_file.file_name, sizeof pac2_file.file_name, 1, fp);
	
		// store file pointer position
		long position = ftell(fp);
	
		// read file info
		fseek(fp, fileinfo_offset + (i * 8), SEEK_SET);
		fread(&pac2_fileinfo.file_location, sizeof pac2_fileinfo.file_location, 1, fp);
		fread(&pac2_fileinfo.file_size, sizeof pac2_fileinfo.file_size, 1, fp);
	
		pac2_fileinfo.file_location += fileinfo_offset + (files * 8);
	
		// extract file data
		printf("extracting \"%s\" (%d/%d, %d bytes) ... ", pac2_file.file_name, (i + 1), files, pac2_fileinfo.file_size);
	
		// relative filename
		char filename[64];
		sprintf(filename, "%s/%s", dirname, pac2_file.file_name);
	
		// allocate memory for file
		char* buffer = (char*)malloc(pac2_fileinfo.file_size);
	
		if(buffer == NULL)
		{
			cout << "fail\n";
			cerr << "warning: failed to allocate memory\n";
			continue;
		}
	
		// seek file pointer position to data offset
		fseek(fp, pac2_fileinfo.file_location, SEEK_SET);
	
		// read data into allocated memory
		fread(buffer, pac2_fileinfo.file_size, 1, fp);
	
		// create file and write data
		FILE* fpw = fopen(filename, "wb");
		
		if(fpw == NULL)
		{
			cout << "fail\n";
			cerr << "warning: failed to create file\n";
		
			free(buffer);
			continue;
		}
		
		fwrite(buffer, 1, pac2_fileinfo.file_size, fpw);
		fclose(fpw);
		
		free(buffer);
	
		// restore file pointer position
		fseek(fp, position, SEEK_SET);
		
		cout << "ok\n";
	}
}

int main(int argc, char* argv[])
{
	cout << "unpac v0.2 by jjolano\n";
	
	if(argc < 2)
	{
		printf("usage: %s <pac|pac2> [outdir]\n", argv[0]);
		return 1;
	}
	
	// open file
	FILE* fp = fopen(argv[1], "rb");
	
	if(fp == NULL)
	{
		perror("error");
		return 1;
	}
	
	// file headers
	pac_hdr pac_hdr = {};
	pac2_hdr pac2_hdr = {};
	
	int version = -1;
	
	// pac version 1 magic
	fread(&pac_hdr.magic, sizeof pac_hdr.magic, 1, fp);
	
	// pac version 2 magic
	fseek(fp, 0, SEEK_SET);
	fread(&pac2_hdr.magic1, sizeof pac2_hdr.magic1, 1, fp);
	fread(&pac2_hdr.magic2, sizeof pac2_hdr.magic2, 1, fp);
	
	// determine version
	if(pac_hdr.magic == _ES32(PAC_MAGIC))
	{
		version = 1;
	}
	
	if(pac2_hdr.magic1 == _ES64(PAC2_MAGIC1) && pac2_hdr.magic2 == _ES32(PAC2_MAGIC2))
	{
		version = 2;
	}
	
	if(version == -1)
	{
		cerr << "error: invalid pac file\n";
		return 1;
	}
	
	cout << "PAC version " << version << "\n";
	
	// create output directory
	cout << "creating output directory ... ";
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
	
	#if defined _MSC_VER
	int ret = _mkdir(dirname);
	#else
	int ret = mkdir(dirname, 0777);
	#endif
	
	if(ret == 0)
	{
		cout << "ok\n";
	}
	else
	{
		if(errno != EEXIST)
		{
			cout << "fail\n";
			cerr << "error: output directory could not be created\n";
			return 1;
		}
		else
		{
			cout << "ok (exists)\n";
		}
	}
	
	switch(version)
	{
		case 1:
			unpac_v1(fp, dirname, &pac_hdr);
			break;
		case 2:
			unpac_v2(fp, dirname, &pac2_hdr);
			break;
		default:
			cerr << "warning: unhandled PAC version (" << version << ")\n";
	}
	
	fclose(fp);
	return 0;
}
