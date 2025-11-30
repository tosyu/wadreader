#ifndef __WADUTILS_H__
#define __WADUTILS_H__

enum wad_error
{
	NO_ERROR,
	FILE_ERROR,
	ALLOCATION,
	MALFORMED_HEADER,
	MALFORMED_FILE,
	MALFORMED_ENTRY,
	FILE_DOES_NOT_EXIST,
	EXTRACT_ERROR
};

typedef struct
{
	char* identification; /* IWAD or PWAD */
	unsigned long numlumps;  /* number of lumps inf the file */
} wad_header;

typedef struct
{
	char* name;
	unsigned long size;
	unsigned long offset;
} wad_lump;

typedef struct
{
	char* file_name;
	wad_header* header;
	wad_lump** lumps;
} wad_file;

wad_file* wad_load(char* file);
void wad_close(wad_file* wad_ptr);
enum wad_error wad_extract_lump(wad_file* wad_ptr, unsigned long lump_offset);
enum wad_error wad_get_error();

#endif
