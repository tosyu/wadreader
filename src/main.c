#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "wadutils.h"

void print_help()
{
	printf("usage: ./wadreader [PATH_TO_WAD]\n");
}

void print_keys()
{
	printf("Keys: \nq - quit\nh - list header\nl - list lumps\ne - extract lump\n? - print this help\n");
}

char* wad_error_to_string(enum wad_error error_code)
{
	switch (error_code)
	{
		case FILE_ERROR:
			return "file error";
		case ALLOCATION:
			return "allocation error";
		case MALFORMED_HEADER:
			return "malformed header";
		case MALFORMED_FILE:
			return "malformed file";
		case MALFORMED_ENTRY:
			return "malformed entry";
		case FILE_DOES_NOT_EXIST:
			return "file does not exist";
		case EXTRACT_ERROR:
			return "general extraction error";
		case NO_ERROR:
		default:
				return "";
	}
}

void print_header(wad_file* wad_ptr)
{
	printf("identification: %s\nlumps count: %lu\n", wad_ptr->header->identification, wad_ptr->header->numlumps);
}

void print_lumps(wad_file* wad_ptr)
{
	if (wad_ptr->header->numlumps == 0)
	{
		printf("No lumps found in file: %s\n", wad_ptr->file_name);
		return;
	}

	unsigned long lumps_per_page = 50;
	int quit = wad_ptr->header->numlumps < lumps_per_page;
	unsigned long offset = 0;
	while (!quit)
	{
		unsigned long i;
		for (i = offset; i < (offset+lumps_per_page) &&  i < wad_ptr->header->numlumps; i++)
		{
			printf("%lu: %s (%lu bytes)\n", (i+1), wad_ptr->lumps[i]->name, wad_ptr->lumps[i]->size);
		}

		quit = 1;
		if ((offset+1) < wad_ptr->header->numlumps)
		{
			printf("any key = next page, quit = q\n");
			quit = getc(stdin) == 'q';
			offset += lumps_per_page;
		}
	}
}

void extract_lump(wad_file* wad_ptr)
{
	if (wad_ptr->header->numlumps == 0)
	{
		printf("No lumps found in file: %s\n", wad_ptr->file_name);
		return;
	}

	printf("Type lump number (q to quit): ");
	char input_buffer[32];
	if (fscanf(stdin, "%s", input_buffer) != 1)
	{
		printf("error\n");
		return;
	}

	if (strcmp(input_buffer, "q") == 0)
	{
		return;
	}

	unsigned long number = (unsigned long)atol(input_buffer);
	unsigned long offset = number - 1;

	enum wad_error result = wad_extract_lump(wad_ptr, offset);

	if (result == NO_ERROR)
	{
		printf("file: %s.bin saved\n", wad_ptr->lumps[offset]->name);
		return;
	}

	printf("error: %s\n", wad_error_to_string(result));
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		print_help();
		exit(1);
	}
	
	char* wad_file_path = *(argv + 1);

	printf("reading: %s\n", wad_file_path);

	wad_file* wad_ptr = wad_load(wad_file_path);

	if (wad_ptr == NULL)
	{
		enum wad_error error_code = wad_get_error();
		printf("error: %s\n", wad_error_to_string(error_code));
		exit(1);
	}

	print_keys();
	
	int quit = 0;
	while(!quit)
	{
		char key = getc(stdin);

		switch (key)
		{
			case 'q':
				quit = 1;
				break;
			case 'e':
				extract_lump(wad_ptr);
				print_keys();
				break;				
			case 'h':
				print_header(wad_ptr);
				print_keys();
				break;
			case 'l':
				print_lumps(wad_ptr);
				print_keys();
				break;
			case '?':
				print_keys();
				break;
		}
	}

	wad_close(wad_ptr);

	return 0;
}
