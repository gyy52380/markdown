#include <assert.h>
#include <stdio.h>

#include "typedef.h"
#include "string.h"
#include "lk_region.h"
#include "list.h"

#include "parser.h"

#define DebugAssert(test) assert(test)

#ifndef TEMPORARY_MEMORY
#define TEMPORARY_MEMORY
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

void print_string_list(SLList<Labeled_String> &list)
{
	for (auto *node = list.head; node != NULL; node = node->next)
		printf("%.*s\n", StringArgs(node->value.value));
}

struct Parse_Context
{
	

	SLList<Labeled_String> section_list; // consists of <p>, <h>, <ul>, <il>, <li> tags and "other text"
	SLList<String> 			output_list; // used to construct output string


	String input; // input string
	String output; // return
	String input_cursor; // moves along input
	String temp_string; //temporary substring of input


	bool p_tag_open = false;
	bool blockquote_tag_open = false;	
	bool header_tag_open[6] = {};
	bool any_list_tag_open = false;

	u32 indent_level = 0;
};


static const Labeled_String tag_begin_p 			= {ST_HTML_TAG, "<p>"_s};
static const Labeled_String tag_end_p 				= {ST_HTML_TAG, "</p>"_s};
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
		while (ctx->indent_level > 0)
		{
			ctx->indent_level--;
			ctx->section_list.append(tag_end_li);
			ctx->section_list.append(tag_end_ul);
		}

		ctx->any_list_tag_open = false;
		ctx->indent_level = 0;
	}
}

// call whenever a new paragraph or header or blockquote may begin
// this usually gets fed the line after two newlines
void new_section_begin(Parse_Context *ctx, String first_line_of_section)
{
	if (!first_line_of_section) return;

	String trimmed_line = trim(first_line_of_section);

	/////////
	// first try to find a header
	////////

	i8 header_level = 0;	// can be h1 to h6
	while (header_level < trimmed_line.length)
	{
		if (trimmed_line[header_level] == '#')
			header_level++; // this can make header_level into 7
		else
			break;
	}
	header_level = header_level > 6 ? 6 : header_level; // if header_level == 7, make it 6
	if (header_level > 0)
	{
		header_level--; //string tag arrays are 0..5 indexed, offset by -1
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
		// everything else (indent level, opening top <ul> etc.)
		// is done in try_add_list_element
		ctx->any_list_tag_open = true;
		return;
	}

 
	////////
	// no list tag found
	// section must be a paragraph
	////////

	ctx->p_tag_open = true;
	ctx->section_list.append(tag_begin_p);
}

bool try_add_list_element(Parse_Context *ctx, String line)
{
	/*
	 * CLARIFICATIONS~
	 *
	 * 1) exiting list section with newline
	 * 	example:
	 * 		- poop
	 * 		- boop
	 * 		  doop // any indent is fine here
	 *		- loop
	 * 	
	 * 	is converted to:
	 * 		<li> poop </li>
	 * 		<li> poop doop </li>
	 *		<li> loop </li>
	 * 	
	 * 	to exit list section double newline is needed
	 * 	(treated like any p or h block)
	 *
	 *
	 * 2) HTML5 and nested lists
	 *	html nests lists like this
	 * 		<ul>
	 *			<li> // <li> tag isnt closed here!!
	 *			<ul>
	 *				<li> "im nested!" </li>
	 *			</ul>
	 *			</li>
	 *		</ul>
	 *
	 *	every nested <ul> must be inside <li>,
	 * 	all of the fuckery below is because of that
	 * 
	 *
	 * 3) HTML5 and VERY nested lists
	 *	while you can make this:
	 *		* indent level 1
	 *			* indent level 2
	 *
	 *	this wont render correctly:
	 *		* indent level 1
	 *				* indent level 3
	 *	
	 *	because of that, if the last indent level was n,
	 * 	and indent level of 'line' is m, m > n
	 *	n is converted to m + 1
	 */


	if (!ctx->any_list_tag_open)
		return false;


	String trimmed_line = trim(line);

	// check for list beginning, and prepare line of text if list begining found
	// otherwise exit
	Labeled_String text;
	if (prefix_equals(trimmed_line, "* "_s) ||
		prefix_equals(trimmed_line, "- "_s))
	{
		String line_without_list_beginning = trim(substring(trimmed_line, 2, trimmed_line.length - 2));
		text = {ST_TEXT, line_without_list_beginning};
	}
	else // line is continuation of last list element (see CLARIFICATION 1)
	{
		text = {ST_TEXT, trimmed_line};
		ctx->section_list.append(text);
		return true;
	}


	// take care of nested levelness
	const u32 line_indent_level = count_leading_whitespace(line) / 4 + 1;

	if (line_indent_level > ctx->indent_level)
	{
		// see CLARIFICATION 3
		ctx->section_list.append(tag_begin_ul);
		ctx->section_list.append(tag_begin_li);
		ctx->section_list.append(text);

		ctx->indent_level++;
	}
	else if (line_indent_level < ctx->indent_level)
	{
		// close 'indent_difference' <ul> and <il> tags, + leading <li> tag
		ctx->section_list.append(tag_end_li);

		const u32 indent_difference = ctx->indent_level - line_indent_level;
		for (int i = 0; i < indent_difference; i++)
		{
			ctx->section_list.append(tag_end_ul);
			ctx->section_list.append(tag_end_li);
		}
		ctx->section_list.append(tag_begin_li);
		ctx->section_list.append(text);

		ctx->indent_level = line_indent_level;
	}
	else // line_indent_level == ctx->indent level
	{
		// end last open <li>, open new one, add text
		// dont end <li> tag because of possible nested lists following
		// indent level is unchanged
		ctx->section_list.append(tag_end_li);
		ctx->section_list.append(tag_begin_li);
		ctx->section_list.append(text);
	}

	return true;
}


const String EMPTY_STRING = ""_s; // also signifies a newline line

String parse(String input)
{
	Parse_Context ctx;

	String input_cursor = input;


	// TODO:
	// FIX THIS WHILE LOOP v
	// strip leading newlines and whitespaces from input
	// special handle first line of input to determine first section



	// strip top newline lines
	while (input_cursor == EMPTY_STRING)
		consume_line_preserve_whitespace(&input_cursor);



	String line = ""_s; // line starts empty to decide type of section for actual first line
	while (input_cursor)
	{
		if (line == ""_s)
		{
			// consume multiple newline lines if they exist
			// after running the loop sets 'line' to first line of new section
			while (line == ""_s)
				line = consume_line_preserve_whitespace(&input_cursor);

			close_all_open_top_level_tags(&ctx);
			new_section_begin(&ctx, line);
		}
		else
		{
			line = consume_line_preserve_whitespace(&input_cursor);
		}


		bool list_el_added = try_add_list_element(&ctx, line);

		if (!list_el_added)
			ctx.section_list.append({ ST_TEXT, line });
	}
	close_all_open_top_level_tags(&ctx);


	print_string_list(ctx.section_list);
	return ""_s;
}