#include <stdio.h>

#include "typedef.h"
#include "string.h"
#include "list.cpp"

LK_Region temporary_memory = {};

int main(int argc, char* argv[])
{
	String path = "test.txt"_s;

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

	ListNode sym_list;

	String temp_text;

	

	String file_cursor;
	while (file_cursor)
	{
		u8 character = consume(&file_cursor, 1);

		if (is_whitespace(character))
			append_node(&sym_list, WHITESPACE);
		else
		{
			#define EndTextBlock 		if (temp_text) append_node(&sym_list, TEXT, temp_text); temp_text = {NULL, NULL};
			#define ContinueTextBlock 	if (!temp_text) temp_text.data = file_cursor.data; temp_text.length++;

			switch(character)
			{
				case '\n':	EndTextBlock; append_node(&sym_list, NEWLINE);			break;
				case '\\':	EndTextBlock; append_node(&sym_list, BACKSLASH);		break;
				case '#': 	EndTextBlock; append_node(&sym_list, HASHTAG);			break;
				case '*': 	EndTextBlock; append_node(&sym_list, ASTERIX);			break;
				case '_': 	EndTextBlock; append_node(&sym_list, UNDERSCORE);		break;
				case '-': 	EndTextBlock; append_node(&sym_list, DASH);				break;
				case '=': 	EndTextBlock; append_node(&sym_list, EQUALS);			break;
				case '(': 	EndTextBlock; append_node(&sym_list, BRACE_L);			break;
				case ')': 	EndTextBlock; append_node(&sym_list, BRACE_R);			break;
				case '[': 	EndTextBlock; append_node(&sym_list, SQUARE_BRACE_L);	break;
				case ']': 	EndTextBlock; append_node(&sym_list, SQUARE_BRACE_R);	break;

				default: ContinueTextBlock;
			}

			#undef EndTextBlock
			#undef ContinueTextBlock
		}
	}

    return 0;
}

