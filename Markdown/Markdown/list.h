#pragma once

#include "string.h"


#define DebugAssert(test) assert(test)

#ifndef TEMPORARY_MEMORY
#define TEMPORARY_MEMORY
	extern 		LK_Region temporary_memory;
	constexpr 	LK_Region* temp = &temporary_memory;
#endif

template <typename T>
struct SLList
{
	struct SLList_Node
	{
		SLList_Node *next;
		T value;
	};

	SLList_Node *head = NULL;
	SLList_Node *tail = NULL;

	void append(T value)
	{
		SLList_Node *new_node = LK_RegionValue(temp, SLList_Node);
		new_node->value = value;

		if (!head) // first element in list
		{
			head = new_node;
			tail = new_node;
			return;
		}

		tail->next = new_node;
		tail = new_node;
	}
};

