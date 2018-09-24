#pragma once

#include "string.h"


#define DebugAssert(test) assert(test)

#if !(temp)
	extern 		LK_Region temporary_memory;
	constexpr 	LK_Region* temp = &temporary_memory;
#endif

template <typename T>
struct SList
{
	struct SList_Node
	{
		SList_Node *next = NULL;
		T value;
	};

	SList_Node *head = NULL;
	SList_Node *tail = NULL;

	void append(T value)
	{
		SList_Node *new_node = LK_RegionValue(temp, T);
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

