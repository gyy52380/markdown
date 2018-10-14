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

u32 count_leading_whitespace(String string)
{
	u32 counter = 0;
	for (int i = 0; i < string.length; i++)
	{
		if (is_whitespace(string[i]))
			counter++;
		else
			break;
	}
	return counter;
}

enum String_Label
	{
		ST_UNKNOWN,
		ST_HTML_TAG,
		ST_META_TAG,
		ST_TEXT
	};

struct Labeled_String
{
	String_Label type;
	String value;
};

struct Parse_Context
{
	

	SList<Labeled_String> section_list; // consists of <p>, <h>, <ul>, <il>, <li> tags and "other text"
	SList<String> 			output_list; // used to construct output string


	String input; // input string
	String output; // return
	String input_cursor; // moves along input
	String temp_string; //temporary substring of input


	bool p_tag_open;
	bool blockquote_tag_open;	
	bool header_tag_open[6];
	bool any_list_tag_open;

	u32 indent_level;
};


static const Labeled_String tag_begin_p 			= {ST_HTML_TAG, "<p>"_s};
static const Labeled_String tag_end_p 				= {ST_HTML_TAG, "</p>"_s};
static const Labeled_String tag_begin_blockquote  	= {ST_HTML_TAG, "<blockquote>"_s};
static const Labeled_String tag_end_blockquote 		= {ST_HTML_TAG, "</blockquote>"_s};
static const Labeled_String tag_begin_ul 			= {ST_HTML_TAG, "<ul>"_s};
static const Labeled_String tag_end_ul 				= {ST_HTML_TAG, "</ul>"_s};
static const Labeled_String tag_begin_li 			= {ST_HTML_TAG, "<li>"_s};
static const Labeled_String tag_end_li 				= {ST_HTML_TAG, "</li>"_s};

static const Labeled_String tag_open_h[6] =
{
													{ ST_HTML_TAG, "<h1>"_s },
													{ ST_HTML_TAG, "<h2>"_s },
													{ ST_HTML_TAG, "<h3>"_s },
													{ ST_HTML_TAG, "<h4>"_s },
													{ ST_HTML_TAG, "<h5>"_s },
													{ ST_HTML_TAG, "<h6>"_s }
};

static const Labeled_String tag_close_h[6] =
{
													{ ST_HTML_TAG, "</h1>"_s },
													{ ST_HTML_TAG, "</h2>"_s },
													{ ST_HTML_TAG, "</h3>"_s },
													{ ST_HTML_TAG, "</h4>"_s },
													{ ST_HTML_TAG, "</h5>"_s },
													{ ST_HTML_TAG, "</h6>"_s }
};

// closes all open <p>, <blockquote>, <h_>, and only the top level <ul>, <il> tags
void close_all_open_top_level_tags(Parse_Context *ctx)
{
	if (ctx->p_tag_open)
	{
		ctx->p_tag_open = false;
		ctx->section_list.append(tag_end_p);
	}

	for (int i = 0; i < 6; i++)
	{
		if (ctx->header_tag_open[i])
		{
			ctx->header_tag_open[i] = false;
			ctx->section_list.append(tag_close_h[i]);
		}
	}

	if (ctx->any_list_tag_open)
	{
		while (ctx->indent_level > 1)
		{
			ctx->indent_level--;
			ctx->section_list.append(tag_end_ul);
			ctx->section_list.append(tag_end_li);
		}
		ctx->section_list.append(tag_end_ul);

		ctx->any_list_tag_open = false;
		ctx->indent_level = 0;
	}
}

// call whenever a new paragraph or header or blockquote may begin
// this usually gets fed the line after two newlines
void new_section_begin(Parse_Context *ctx, String first_line_of_section)
{
	// untrimmed version is used to check indent levels in lists
	String trimmed_line = trim(first_line_of_section);

	close_all_open_top_level_tags(ctx);


	/////////
	// first try to find a header
	////////

	i8 header_level = -1;	// can be h1 to h6 (0 to 5 indexed)
	while (header_level < 6)
	{
		if (trimmed_line[header_level] == '#')
			header_level++;
		else
			break;
	}
	if (header_level >= 0)
	{
		ctx->header_tag_open[header_level] = true;
		ctx->section_list.append(tag_open_h[header_level]);
		return;
	}


	////////
	// no header openers found
	// look for list beginning (TODO ordered lists)
	////////

	if (prefix_equals(trimmed_line, "- "_s) ||
		prefix_equals(trimmed_line, "* "_s))
	{
		ctx->any_list_tag_open = true;
		ctx->indent_level = count_leading_whitespace(first_line_of_section) / 4 + 1;
		ctx->section_list.append(tag_begin_ul);
		return;
	}

 
	////////
	// no list tag found
	// section must be a paragraph
	////////

	ctx->p_tag_open = true;
	ctx->section_list.append(tag_begin_p);
}

void try_add_list_element(Parse_Context *ctx, String line)
{
	// if this function is called a list section is opened

	String trimmed_line = trim(line);

	if (prefix_equals(trimmed_line, "- "_s) ||
		prefix_equals(trimmed_line, "* "_s))
	{
		u32 line_indent_level = count_leading_whitespace(line) / 4 + 1;

		if (line_indent_level > ctx->indent_level)
		{
			for (int i = 0; i < line_indent_level - ctx->indent_level; i++)
			{
				ctx->section_list.append(tag_begin_li);
				ctx->section_list.append(tag_begin_ul);
			}
		}
		else if (line_indent_level < ctx->indent_level)
		{
			for (int i = 0; i < ctx->indent_level - line_indent_level; i++)
			{
				ctx->section_list.append(tag_end_li);
				ctx->section_list.append(tag_end_ul);
			}
		}

		ctx->section_list.append(tag_begin_li);

		String trimmed_text = substring(trimmed_line, 3, trimmed_line.length - 3);
		Labeled_String text = {ST_TEXT, trimmed_text};
		ctx->section_list.append(text);

		ctx->section_list.append(tag_end_li);

		ctx->indent_level = line_indent_level;
	}
}

String parse(String input)
{
	Parse_Context ctx;
	ctx.input = input;

	ctx.input_cursor = ctx.input;

	// decide what kind of section is the first line
	String line1 = consume_line(&ctx.input_cursor);
	String line2 = consume_line(&ctx.input_cursor);
	new_section_begin(&ctx, line1);

	while (ctx.input_cursor)
	{
		line1 = line2;
		line2 = consume_line(&ctx.input_cursor);


		if (trim(line1) == "\n"_s) // new paragraph begins
			new_section_begin(&ctx, line2);

		if (ctx.any_list_tag_open)
			try_add_list_element(&ctx, line1);
	}
}