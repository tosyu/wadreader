#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wadutils.h"

static enum wad_error current_error = NO_ERROR;

enum wad_error wad_get_error()
{
	return current_error;
}

wad_file* wad_load(char* path)
{
	FILE* file = fopen(path, "rb");
	
	if (file == NULL)
	{
		current_error = FILE_ERROR;
		return NULL;
	}

	wad_file* wad_ptr = (wad_file*)malloc(sizeof(wad_file));

	if (wad_ptr == NULL) {
		current_error = ALLOCATION; 
		fclose(file);
		return NULL;
	}

	int file_name_length = strlen(path);
	wad_ptr->file_name = (char*)malloc(sizeof(char)*file_name_length + 1);
	wad_ptr->header = NULL;
	wad_ptr->lumps = NULL;

	if (wad_ptr->file_name == NULL)
	{
		current_error = ALLOCATION;
		wad_close(wad_ptr);
		fclose(file);
		return NULL;
	}

	memcpy(wad_ptr->file_name, path, file_name_length);

	wad_ptr->file_name[file_name_length] = '\0';

	wad_header* header = (wad_header*)malloc(sizeof(wad_header));

	if (header == NULL) {
		current_error = ALLOCATION; 
		wad_close(wad_ptr);
		fclose(file);
		return NULL;
	}

	header->numlumps = 0;
	header->identification = (char*)malloc(sizeof(char)*5);

	if (header->identification == NULL)
	{
		current_error = ALLOCATION;
		free(header);
		wad_close(wad_ptr);
		fclose(file);
		return NULL;
	}

	unsigned long directory_position = 0;
	char header_buffer[12];
	int bytes_read = 0;

	bytes_read = fread(header_buffer, sizeof(char), 12, file);

	if (bytes_read < 12)
	{
		current_error = MALFORMED_HEADER;
		free(header);
		wad_close(wad_ptr);
		fclose(file);
		return NULL;
	}

	memcpy(header->identification, header_buffer, 4);
	memcpy(&(header->numlumps), header_buffer + 4, 4);
	memcpy(&directory_position, header_buffer + 8, 4);

	header->identification[4] = '\0';
	wad_ptr->header = header;
	
	if (header->numlumps == 0)
	{
		fclose(file);
		return wad_ptr;
	}
	
	if (fseek(file, directory_position, SEEK_SET) != 0)
	{
		current_error = MALFORMED_FILE;
		wad_close(wad_ptr);
		fclose(file);
		return NULL;
	}

	wad_lump** lumps = malloc(sizeof(wad_lump*)*header->numlumps);

	if (lumps == NULL) {
		current_error = ALLOCATION; 
		wad_close(wad_ptr);
		fclose(file);
		return NULL;
	}

	unsigned long i;
	for (i = 0; i < header->numlumps; i++)
	{
		lumps[i] = (wad_lump*)malloc(sizeof(wad_lump));

		if (lumps[i] == NULL)
		{
			current_error = ALLOCATION;
			continue;
		}

		char lump_buffer[16];

		bytes_read = fread(lump_buffer, sizeof(char), 16, file);
		
		if (bytes_read < 16)
		{
			current_error = MALFORMED_ENTRY;
			continue;
		}

		memcpy(&(lumps[i]->offset), lump_buffer, 4);
		memcpy(&(lumps[i]->size), lump_buffer + 4, 4);

		int name_length = strlen(lump_buffer + 8);
		lumps[i]->name = (char*)malloc(sizeof(char) * name_length + 1);
		
		if (lumps[i]->name == NULL)
		{
			current_error = ALLOCATION;
			continue;
		}

		memcpy(lumps[i]->name, lump_buffer + 8, name_length);
		lumps[i]->name[name_length] = '\0';
	}

	fclose(file);
	
	wad_ptr->lumps = lumps;

	return wad_ptr;
}

enum wad_error wad_extract_lump(wad_file* wad_ptr, unsigned long lump_offset)
{
	if (wad_ptr->header->numlumps <= lump_offset)
	{
		return FILE_DOES_NOT_EXIST;
	}

	wad_lump* lump = wad_ptr->lumps[lump_offset];
	char* buffer = NULL;

	buffer = malloc(sizeof(char) * lump->size);

	if (buffer == NULL)
	{
		return ALLOCATION;
	}
	
	FILE* file = fopen(wad_ptr->file_name, "rb");

	if (file == NULL)
	{
		free(buffer);
		return FILE_ERROR;
	}

	if (fseek(file, lump->offset, SEEK_SET) != 0)
	{
		fclose(file);
		free(buffer);
		return FILE_ERROR;
	}

	unsigned long bytes_read = 0;
	bytes_read = fread(buffer, sizeof(char), lump->size, file);

	if (bytes_read != lump->size)
	{
		free(buffer);
		fclose(file);
		return EXTRACT_ERROR;
	}

	fclose(file);

	char* out_name = NULL;
	
	int name_length = strlen(lump->name);
	out_name = malloc(sizeof(char)*5 + name_length);

	if (out_name == NULL)
	{
		free(buffer);
		return ALLOCATION;
	}

	memcpy(out_name, lump->name, name_length);
	memcpy(out_name + name_length, ".bin\0", 5);

	FILE* out = fopen(out_name, "wb+");

	if (out == NULL)
	{
		free(buffer);
		free(out_name);
		return FILE_ERROR;
	}

	unsigned long bytes_written = fwrite(buffer, sizeof(char), lump->size, out);

	free(buffer);
	free(out_name);
	fclose(out);

	if (bytes_written != lump->size)
	{
		return FILE_ERROR;
	}

	return NO_ERROR;
}

void wad_close(wad_file* wad_ptr)
{
	if (wad_ptr == NULL)
	{
		return;
	}

	unsigned long numlumps = 0;
	if (wad_ptr->header != NULL)
	{
		numlumps = wad_ptr->header->numlumps;
		if (wad_ptr->header->identification != NULL)
		{
			free(wad_ptr->header->identification);
		}

		free(wad_ptr->header);
	}

	if (wad_ptr->lumps != NULL)
	{
		unsigned long i;
		for (i = 0; i < numlumps; i++)
		{
			if (wad_ptr->lumps[i]->name != NULL)
			{
				free(wad_ptr->lumps[i]->name);
			}

			free(wad_ptr->lumps[i]);
		}

		free(wad_ptr->lumps);
	}

	if (wad_ptr->file_name != NULL)
	{
		free(wad_ptr->file_name);
	}

	free(wad_ptr);
}
