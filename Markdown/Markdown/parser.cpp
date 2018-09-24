#include <assert.h>
#include <stdio.h>

#include "typedef.h"
#include "string.h"
#include "lk_region.h"
#include "list.h"

#define DebugAssert(test) assert(test)

#if !(temp)
	extern 		LK_Region temporary_memory;
	constexpr 	LK_Region* temp = &temporary_memory;
#endif


struct Parse_Context
{
	bool parsing = true;

	struct
	{
		bool p 		: 1;

		bool h1 	: 1;
		bool h2 	: 1;
		bool h3 	: 1;
		bool h4 	: 1;
		bool h5 	: 1;
		bool h6 	: 1;

		bool bold 	: 1;
		bool italic : 1;
	} open;

	struct
	{
		u8 newline;
		u8 asterix;
		u8 equals;
		u8 hashtag;
	} conescutive;

	String input; // input string
	String input_cursor; // moves along input
	String temp_string; //temporary substring of input

	SList<String> paragraph_list; // ` "<p>"->"text"->"</p>"->... `
	SList<String> output_list; // used to construct output string

	String output; // return
};

String parse(String input)
{
	Parse_Context ctx;
	ctx.input = input;

	ctx.input_cursor = ctx.input;

	// decide if first paragraph is p or h
	String first_line = trim(consume_line(&ctx.input_cursor));

	if (first_line[0] != '#')
	{
		ctx.paragraph_list.append("<p>"_s);
		ctx.open.p = true;
	}
	else if (prefix_equals(first_line, "######"_s))
	{
		ctx.paragraph_list.append("<h6>"_s);
		ctx.open.h6 = true;
	}
	else if (prefix_equals(first_line, "#####"_s))
	{
		ctx.paragraph_list.append("<h5>"_s);
		ctx.open.h5 = true;
	}
	else if (prefix_equals(first_line, "####"_s))
	{
		ctx.paragraph_list.append("<h4>"_s);
		ctx.open.h4 = true;
	}
	else if (prefix_equals(first_line, "###"_s))
	{
		ctx.paragraph_list.append("<h3>"_s);
		ctx.open.h3 = true;
	}
	else if (prefix_equals(first_line, "##"_s))
	{
		ctx.paragraph_list.append("<h2>"_s);
		ctx.open.h2 = true;
	}
	else
	{
		ctx.paragraph_list.append("<h1>"_s);
		ctx.open.h1 = true;
	}



	while (ctx.input_cursor)
	{
		String line = trim(consume_line(&ctx.input_cursor);
	}
}


void add_to_text_block(Parse_Context *ctx)
{
	if (ctx->plaintext_block)
	{
		if (ctx->plaintext_block.length == ctx->input.length)
		{
			printf("EOF.\n");
			return;
		}

		ctx->plaintext_block.length++;
	}
	else
	{
		ctx->plaintext_block.data 	= ctx->input_cursor.data;
		ctx->plaintext_block.length = 1;
	}
}

void finish_current_text_block(Parse_Context *ctx)
{
	if (ctx->plaintext_block)
	{
		list_append(&ctx->output_list, ctx->plaintext_block);
		ctx->plaintext_block.data = NULL;
		ctx->plaintext_block.length = 0;
	}
}


void parse_newline(Parse_Context *ctx)
{

}

void parse_backslash(Parse_Context *ctx)
{

}

void parse_hashtag(Parse_Context *ctx)
{

}

void parse_asterix(Parse_Context *ctx)
{

}

void parse_underscore(Parse_Context *ctx)
{

}

void parse_dash(Parse_Context *ctx)
{

}

void parse_equals(Parse_Context *ctx)
{

}

void parse_brace_l(Parse_Context *ctx)
{

}

void parse_brace_r(Parse_Context *ctx)
{

}

void parse_square_brace_l(Parse_Context *ctx)
{

}

void parse_square_brace_r(Parse_Context *ctx)
{

}
