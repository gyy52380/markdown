#pragma once

#include "memory.h"
#include "string.h"


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

