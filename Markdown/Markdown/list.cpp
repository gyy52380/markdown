#pragma once

#include "typedef.h"
#include "lk_region.h"
#include "string.h"
#include "list.h"



// singly linked list for storing symbols

void append_node(ListNode *last, Symbol value, String extra_data)
{
	ListNode *new_node = LK_RegionValue(temp, ListNode);

	new_node->next 		 = NULL;
	new_node->value 	 = value;
	new_node->extra_data = extra_data ? extra_data : String{};

	last->next = new_node;
}