#include <stdio.h>

#include "typedef.h"
#include "string.h"
#include "parser.h"
#include "file_io.h"

#define TEMP_MEMORY_IMPLEMENTATION
#include "memory.h"


int main(int argc, char* argv[])
{
	String path = "C:\\Users\\gabri\\source\\repos\\markdown\\Markdown\\Debug\\test.txt"_s;

	if (argc == 2)
	{
		path = make_string(argv[1]);
	}
	else
	{
		printf("Usage: markdown.exe filename.md\nUsing default path: %.*s\n", StringArgs(path));
	}

	String file;
	bool read_success = read_entire_file(&file, path);
	if (!read_success)
	{
		printf("Failed to read file: %.*s", StringArgs(path));
		return 0;
	}

	String html = parse(file);


    return 0;
}

