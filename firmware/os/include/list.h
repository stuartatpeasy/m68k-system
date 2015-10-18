#ifndef INCLUDE_LIST_H_INC
#define INCLUDE_LIST_H_INC
/*
	Simple linked-list implementation.  Owes everything to <linux/list.h>.

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, December 2011.
*/

#include "include/defs.h"

struct llitem
{
    struct llitem *next;
    struct llitem *prev;
};

typedef struct llitem list_t;

#define LIST_INVALID_ITEM      ((list_t *) (0xc01dc0de))

#define LIST_INIT(name) { &(name), &(name) }

#define LINKED_LIST(name) \
    list_t name = LIST_INIT(name)

/**
 * list_entry - get the containing struct for the current entry
 * @ptr: ptr to a list_t referring to the item
 * @type: the type of the containing struct
 * @member: name of the member containing the list_t within the struct
 */
#define list_entry(ptr, type, member) \
    containerof(ptr, type, member)


/**
 * list_first_entry - get the first element in the list
 * @ptr: ptr to a list_t referring to the item
 * @type: the type of the containing struct
 * @member: name of the member containing the list_t within the struct
 */
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)


/**
 * list_last_entry - get the last element in the list
 * @ptr: ptr to a list_t referring to the item
 * @type: the type of the containing struct
 * @member: name of the member containing the list_t within the struct
 */
#define list_last_entry(ptr, type, member) \
    list_entry((ptr)->prev, type, member)


/**
 * list_first_entry_or_null - get the first element in the list, or NULL if the list is empty
 * @ptr: ptr to a list_t referring to the item
 * @type: the type of the containing struct
 * @member: name of the member containing the list_t within the struct
 */
#define list_first_entry_or_null(ptr, type, member) \
    (!list_is_empty_list(ptr) ? list_first_entry(ptr, type, member) : NULL)


/**
 * list_next_entry - get the next element in the list
 * @pos: ptr to list_t to use as a cursor
 * @member: name of the member containing the list_t within the struct
 */
#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)


/**
 * list_prev_entry - get the previous element in the list
 * @pos: ptr to list_t to use as a cursor
 * @member: name of the member containing the list_t within the struct
 */
#define list_prev_entry(pos, member) \
    list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * list_for_each - iterate over a list
 * @pos: ptr to list_t to use as a cursor
 * @head: head of the list
 */
#define list_for_each(pos, head) \
    for(pos = (head)->next; pos != (head); pos = pos->next)


/**
 * list_for_each_safe - iterate over a list safely (in case of item removal)
 * @pos: ptr to list_t to use as a cursor
 * @n: another ptr, like @pos, to use as temporary storage
 * @head: head of the list
 */
#define list_for_each_safe(pos, n, head) \
    for(pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)


/**
 * list_for_each_reverse - iterate over a list backwards
 * @pos: ptr to a list_t to use as a cursor
 * @head: head of the list
 */
#define list_for_each_reverse(pos, head) \
    for(pos = (head)->prev; pos != (head); pos = pos->prev)


/**
 * list_for_each_reverse_safe - iterate over a list backwards safely (in case of item removal)
 * @pos: ptr to list_t to use as a cursor
 * @n: another ptr, like @pos, to use as temporary storage
 * @head: head of the list
 */
#define list_for_each_reverse_safe(pos, n, item) \
    for(pos = (item)->prev, n = pos->prev; pos != (item); pos = n, n = pos->prev)


/**
 * list_for_each_entry - iterate over a list of the specified type
 * @pos: var of ptr type to use as a cursor
 * @head: head of the list
 * @member: name of the member containing the list_t within the struct
 */
#define list_for_each_entry(pos, head, member)                                              \
    for(pos = list_first_entry(head, typeof(*pos), member);                               \
        &pos->member != (head);                                                             \
        pos = list_next_entry(pos, member))


/**
 * list_for_each_entry_safe - iterate over a list of the specified type; safe for entry removal.
 * @pos: var of ptr type to use as cursor
 * @n: another var, same type as ptr, to use as temporary storage
 * @head: head of the list
 * @member: name of the member containing the list_t within the struct
 */
#define list_for_each_entry_safe(pos, n, head, member)                                      \
    for(pos = list_entry((head)->next, typeof(*pos), member),                               \
            n = list_entry(pos->member.next, typeof(*pos), member);                         \
        &pos->member != (head);                                                             \
        pos = n, n = list_entry(n->member.next, typeof(*n), member))


/**
 * list_for_each_entry_reverse - iterate backwards over a list of the specified type
 * @pos: var of ptr type to use as a cursor
 * @head: head of the list
 * @member: name of the member containing the list_t within the struct
 */
#define list_for_each_entry_reverse(pos, head, member)                                      \
    for(pos = list_last_entry(head, typeof(*pos), member);                               \
        &pos->member != (head);                                                             \
        pos = list_prev_entry(posm, member))


/**
 * list_for_each_entry_safe reverse - iterate over a list of the specified type; safe for entry
 * removal.
 * @pos: var of ptr type to use as cursor
 * @n: another var, same type as ptr, to use as temporary storage
 * @head: head of the list
 * @member: name of the member containing the list_t within the struct
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member)                              \
    for(pos = list_entry((head)->prev, typeof(*pos), member),                               \
            n = list_entry(pos->member.prev, typeof(*pos), member);                         \
        &pos->member != (head);                                                             \
        pos = n, n = list_entry(n->member.prev, typeof(*n), member))


/**
 * list_init - initialise a linked list
 * @l: list to initialise
 *
 * Must be called before the list is used.
 */
static inline void list_init(list_t *l)
{
    l->next = l;
    l->prev = l;
}


/**
 * do_list_append - add a new item to a linked list
 * @new: new item to be added
 * @prev: ptr to (existing) previous item in list
 * @next: ptr to (existing) next item in list
 *
 * Used by other functions in this module.  Should not be called from outside this module.
 */
static inline void do_list_append(list_t *new, list_t *prev, list_t *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}


/**
 * do_list_delete - delete an item from a linked list
 * @prev: item before the item to be deleted
 * @next: item after the item to be deleted
 *
 * Used by other functions in this module.  Should not be called from outside this module.
 */
static inline void do_list_delete(list_t *prev, list_t *next)
{
    next->prev = prev;
    prev->next = next;
}


/**
 * list_append - add a new item after the specified item in a linked list
 * @new: new item to be added
 * @item: existing item
 *
 * Insert a new linked list entry after the specified item.
 */
static inline void list_append(list_t *new, list_t *item)
{
    do_list_append(new, item, item->next);
}


/**
 * list_insert - insert a new item before the specified item in a linked list
 * @new: new item to be added
 * @item: existing item
 */
static inline void list_insert(list_t *new, list_t *item)
{
    do_list_append(new, item->prev, item);
}


/**
 * list_delete - delete an item from a linked list
 * @item: item to be deleted
 *
 * Note: this leaves item in an undefined state.
 */
static inline void list_delete(list_t *item)
{
    do_list_delete(item->prev, item->next);
    item->next = LIST_INVALID_ITEM;
    item->prev = LIST_INVALID_ITEM;
}


/**
 * list_replace - replace an item in a list
 * @old: item to be replaced
 * @new: replacement item
 */
static inline void list_replace(list_t *old, list_t *new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}


/**
 * list_move_append - remove an item from a list and append it to another list
 * @item: item to remove
 * @dest: item after which the removed item should be appended
 */
static inline void list_move_append(list_t *item, list_t *dest)
{
    do_list_delete(item->prev, item->next);
    list_append(item, dest);
}


/**
 * list_move_insert - remove an item from a list and append it to another list
 * @item: item to remove
 * @dest: item after which the removed item should be appended
 */
static inline void list_move_insert(list_t *item, list_t *dest)
{
    do_list_delete(item->prev, item->next);
    list_insert(item, dest);
}


/**
 * list_is_last - finds whether @item is the last entry before @first
 * @item: item to test
 * @first: first item in the list
 */
static inline s32 list_is_last(const list_t *item, const list_t *first)
{
    return item->next == first;
}


/**
 * list_is_empty - find whether item refers to an empty list
 * @item: item to test
 */
static inline s32 list_is_empty(const list_t *item)
{
    return item->next == item;
}

#endif
