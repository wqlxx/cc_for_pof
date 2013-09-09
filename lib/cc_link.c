/*
 * Copyright (C) 2008-2012 NEC Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "cc_link.h"


/**
 * Initializes a new list by passing the head of the list. Note that
 * the head element should be allocated on caller's stack or heap.
 *
 * @param list the head of the list.
 * @return true on success; false otherwise.
 */
int
create_list( list_element_t **list ) {

  if ( list != NULL ) {
    CC_ERROR_PRINT_HEAD;
    CC_PRINT("list is existing");
    return CC_ERROR;
  }

  *list = malloc(sizeof(list_element_t));
  (*list)->next = NULL;
  (*list)->data = NULL;

  return CC_NO_ERROR;
}


/**
 * Inserts a new element at the head of the list.
 *
 * @param head the head of the list. This will be updated to the new element.
 * @param data the data for the new element.
 * @return true on success; false otherwise.
 */
int
insert_in_front( list_element_t **head, void *data ) {
  if ( head == NULL ) {
    CC_ERROR_PRINT_HEAD;
    CC_PRINT( "head must not be NULL" );
  }

  list_element_t *old_head = *head;
  list_element_t *new_head = malloc( sizeof( list_element_t ) );

  new_head->data = data;
  *head = new_head;
  ( *head )->next = old_head;
  return CC_NO_ERROR;
}


/**
 * Inserts a node before \e sibling containing \e data.
 *
 * @param head the head of the list.
 * @param sibling node to insert \e data before.
 * @param data data to put in the newly-inserted node.
 * @return true on success; false otherwise.
 */
int
insert_before( list_element_t **head, const void *sibling, void *data ) {
  if ( head == NULL ) {
    CC_ERROR_PRINT_HEAD;
    CC_PRINT("head must not be NULL");
    return CC_ERROR;
  }

  for ( list_element_t *e = *head; e->next != NULL; e = e->next ) {
    if ( e->next->data == sibling ) {
      list_element_t *new_element = malloc( sizeof( list_element_t ) );
      new_element->next = e->next;
      new_element->data = data;
      e->next = new_element;
      return CC_NO_ERROR;
    }
  }

    return CC_ERROR;    
}


/**
 * Adds a new element on to the end of the list.
 *
 * @param head the head of the list.
 * @param data the data for the new element.
 * @return true on success; false otherwise.
 */
int
insert_tail( list_element_t **head, void *data ) {
  if ( head == NULL ) {
    CC_ERROR_PRINT_HEAD;
    CC_PRINT( "head must not be NULL" );
    return CC_ERROR;
  }

  list_element_t *new_tail = malloc( sizeof( list_element_t ) );
  new_tail->data = data;
  new_tail->next = NULL;

  if ( *head == NULL ) {
    *head = new_tail;
    return CC_NO_ERROR;
  }

  list_element_t *e;
  for ( e = *head; e->next != NULL; e = e->next );

  e->next = new_tail;
    
  return CC_NO_ERROR;
}


/**
 * Gets the number of elements in a list.
 *
 * @param head the head of the list.
 * @return the number of elements in the list.
 */
unsigned int
list_length_of( const list_element_t *head ) {
  if ( head == NULL ) {
    return CC_ERROR;
  }
  unsigned int length = 1;
  
  for ( list_element_t *e = head->next; e; e = e->next, length++ );
  return length;
}


/**
 * Calls a function for each element of a list.
 *
 * @param head the head of the list.
 * @param function the function to call with each element's data.
 * @param user_data user-data to pass to the function.
 */
void
iterate_list( list_element_t *head, void (*function)( void *data, void *user_data ), void *user_data ) {
  if ( head == NULL ) {
    CC_ERROR_PRINT_HEAD;
    CC_PRINT"head must not be NULL");
    return CC_ERROR;
  }
  if ( function != NULL ) {
    for ( list_element_t *e = head; e != NULL; e = e->next ) 
	{
      (*function)( e->data, user_data );
    }
  }
}


/**
 * Finds an element in a list, using a supplied function to find the
 * desired element. It iterates over the list, calling the given
 * function which should return true when the desired element is found.
 *
 * @param head the head of the list.
 * @param function the function to call for each element. It should return true when the desired element is found.
 * @param user_data user-data passed to the function.
 * @return the found list element, or NULL if it is not found.
 */
void *
find_list_custom( list_element_t *head, bool function( void *data, void *user_data ), void *user_data ) {

  if ( head == NULL ) {
    CC_ERROR_PRINT_HEAD;
    CC_PRINT( "head must not be NULL");
    return NULL;
  }
  if ( function == NULL ) {
    CC_ERROR_PRINT_HEAD;
    CC_PRINT("function must not be NULL");
    return NULL;
  }

  void *data_found = NULL;
  
  for ( list_element_t *e = head; e != NULL; e = e->next ) {
    if ( function( e->data, user_data ) ) {
      data_found = e->data;
      break;
    }
  }
  return data_found;
}


/**
 * Removes an element from a list. If two elements contain the same
 * data, only the first is removed. If none of the elements contain
 * the data, the list is unchanged.
 *
 * @param head the head of the list.
 * @param data the data of the element to remove.
 * @return true on success; false otherwise.
 */
int
delete_element( list_element_t **head, const void *data ) {
  if ( head == NULL ) {
    CC_ERROR_PRINT_HEAD;
    CC_PRINT("head must not be NULL" ;
    return CC_ERROR;
  }

  list_element_t *e = *head;

  if ( e->data == data ) {
    *head = e->next;
    free( e );
    return CC_NO_ERROR;
  }

  for ( ; e->next != NULL; e = e->next ) {
    if ( e->next->data == data ) {
      list_element_t *delete_me = e->next;
      e->next = e->next->next;
      free( delete_me );
      return CC_NO_ERROR;
    }
  }

  return CC_ERROR;
}


/**
 * Removes all elements from a list.
 *
 * @param head the head of the list.
 * @return true on success; false otherwise.
 */
int
delete_list( list_element_t *head ) {
  for ( list_element_t *e = head; e != NULL; ) {
    list_element_t *delete_me = e;
    e = e->next;
    free( delete_me );
  }
  return CC_NO_ERROR;
}


