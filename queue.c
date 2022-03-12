#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "list.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q = (struct list_head *) malloc(sizeof(struct list_head));
    if (!q)
        return NULL;
    INIT_LIST_HEAD(q);

    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    if (list_empty(l)) {
        free(l);
        return;
    }

    struct list_head *cur = NULL, *safe = NULL;
    // Need modify list in list so use safe iteration macro function.
    list_for_each_safe (cur, safe, l) {
        element_t *e = container_of(cur, element_t, list);
        q_release_element(e);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = (element_t *) malloc(sizeof(element_t));

    if (!node)
        return false;

    // Need to add 1 to cover the '\0'
    size_t length = strlen(s) + 1;
    node->value = (char *) malloc(sizeof(length));


    if (node->value) {
        strncpy(node->value, s, length);
    } else {
        free(node);
        return false;
    }

    list_add(&node->list, head);

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = (element_t *) malloc(sizeof(element_t));

    if (!node)
        return false;

    size_t len = strlen(s) + 1;
    node->value = (char *) malloc(sizeof(len));
    if (node->value) {
        strncpy(node->value, s, len);
    } else {
        free(node);
        return false;
    }

    list_add_tail(&node->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *e = list_first_entry(head, element_t, list);
    list_del_init(head->next);

    // Need to check sp is already been allocate, and element is not removed..
    if (sp) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return e;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *e = list_last_entry(head, element_t, list);
    list_del_init(head->prev);

    if (sp) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }


    return e;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    size_t len = 0;

    struct list_head *node = NULL;
    list_for_each (node, head)
        len++;


    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (!head || list_empty(head))
        return false;

    struct list_head *front = head->next;
    struct list_head *back = head->prev;

    // Find middle node from both side of list, and front node would be the
    // target which need to be removed.
    while (front != back && back->prev != front) {
        front = front->next;
        back = back->prev;
    }

    list_del(front);
    q_release_element(container_of(front, element_t, list));

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    if (!list_empty(head)) {
        struct list_head *ptr = NULL, *next = NULL;
        bool last_dup = false;

        list_for_each_safe (ptr, next, head) {
            element_t *cur_element = list_entry(ptr, element_t, list);
            bool match =
                (next != head &&
                 strcmp(cur_element->value,
                        list_entry(next, element_t, list)->value) == 0);
            if (match || last_dup) {
                list_del(ptr);
                q_release_element(cur_element);
            }

            last_dup = match;
        }
    }

    return true;
}

/*
 * Exchange two adjacent nodes and reture pointer a
 */

// struct list_head *exchange(struct list_head *a, struct list_head *b)
//{
//    struct list_head *prev = a->prev;
//    struct list_head *next = b->next;
//    a->next = next;
//    next->prev = a;
//    b->prev = prev;
//    prev->next = b;
//    b->next = a;
//    a->prev = b;
//
//    return a;
//}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    // struct list_head *left = head->next, *right = left->next;
    // while(left != head && right != head) {
    //     left->next = right->next;
    //     right->next->prev = left;
    //     right->prev = left->prev;
    //     left->prev->next = right;
    //     left->prev = right;
    //     right->next = left;

    //     left = left->next;
    //     right = left->next;
    // }

    struct list_head *node;
    for (node = head->next; (node->next != head) && (node != head);
         node = node->next) {
        struct list_head *next = node->next;
        list_del(node);
        list_add(node, next);
    }
}


/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node = NULL;
    struct list_head *safe = NULL;
    list_for_each_safe (node, safe, head) {
        list_move(node, head);
    }

    // struct list_head *h = head->next;
    // struct list_head *t = head->prev;

    // while(h != t && h->next != t) {
    //     element_t *he = list_entry(h, element_t, list);
    //     element_t *ht = list_entry(t, element_t, list);

    //     char *tmp = he->value;
    //     he->value = ht->value;
    //     ht->value = tmp;

    //     h = h->next;
    //     t = t->prev;
    // }
}

struct list_head *sorted_merge(struct list_head *front, struct list_head *back)
{
    if (!front || !back)
        return front ? front : back;

    struct list_head *result = NULL;

    if (strcmp(list_entry(front, element_t, list)->value,
               list_entry(back, element_t, list)->value) > 0) {
        result = back;
        result->next = sorted_merge(front, back->next);
    } else {
        result = front;
        result->next = sorted_merge(front->next, back);
    }

    return result;
}


void merge_sort(struct list_head **head)
{
    if (!*head || !(*head)->next)
        return;

    struct list_head *slow = *head, *fast = slow->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    struct list_head *front = *head;
    struct list_head *back = slow->next;
    slow->next = NULL;

    merge_sort(&front);
    merge_sort(&back);

    *head = sorted_merge(front, back);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    // Let linked list to be singly linked list.
    head->prev->next = NULL;
    merge_sort(&head->next);

    struct list_head *tmp = head;
    while (tmp->next) {
        tmp->next->prev = tmp;
        tmp = tmp->next;
    }

    tmp->next = head;
    head->prev = tmp;
}
