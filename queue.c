/* Ella Visconti
 * Starter file for CS 208 assignment: Queues in C
 * Adapted by Aaron Bauer and then Jeff Ondich
 * from a lab developed at CMU by R. E. Bryant, 2017-2018
 */
/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
  queue_t *q = malloc(sizeof(queue_t));
  if (q == NULL)
  {
    fprintf(stderr, "Space could not be allocated");
    return NULL;
  }
  q->head = NULL;
  q->tail = NULL;
  q->node_count = 0;
  return q;
}
/*call in other functions to free nodes*/
void free_node(list_ele_t *node){
  free(node->value);
  free(node);
}
/* Free all storage used by queue by looping through list nodes
starting at the head, freeing each node and its value. */
void q_free(queue_t *q)
{
  if (q == NULL)
  {
    fprintf(stdout,"Queue is null");
    return;
  }
  else {
    list_ele_t *current_node;
    current_node = q->head;
    while (current_node != NULL)
    {
      list_ele_t *next_node;
      next_node=current_node->next;
      free_node(current_node);
      current_node=next_node;
    }
    free(q);
}
}
/*helper function for insert_head insert_tail
function. Copies value into new node */
list_ele_t *new_node(queue_t *q, char *s){
  list_ele_t *new_node = malloc(sizeof(list_ele_t));
  if (new_node == NULL)
  {
    fprintf(stderr, "Error malloc\n");
    return false;
  }
  new_node->value = malloc(sizeof(char) * (strlen(s) + 1));
  if (new_node->value == NULL)
  {
    fprintf(stderr, "Error malloc\n");
    free(new_node);
    return false;
  }
  strcpy(new_node->value, s);
  new_node->next=NULL;
  return new_node;

}
/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
  if (q == NULL)
  {
    fprintf(stderr, "Queue is null");
    return false;
  }
  list_ele_t *new_head;
  new_head=new_node(q,s);
  if (new_head==NULL){
    return false;
  }
  new_head->next = q->head;
  q->head = new_head;

  if (q->tail == NULL)
  {
    q->tail = new_head;
  }
  q->node_count++;
  return true;
}

/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
  if (q == NULL)
  {
    fprintf(stderr, "Queue is null");
    return false;
  }
  list_ele_t *new_tail;
  new_tail=new_node(q,s);
  if (new_tail==NULL){
    fprintf(stderr, "Malloc error");
    return false;
  }
  if (q->head == NULL)
  {
    q->head = new_tail;
  }
  else
  {
    q->tail->next = new_tail;
  }
  q->tail = new_tail;
  q->node_count++;
  return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
  if (q == NULL || q->head == NULL)
  {
    fprintf(stderr, "q is null");
    return false;
  }
  int s_length=0;
  for (int i=0; q->head->value[i]!='\0';i++){
    s_length++;
  }

  if (sp != NULL && s_length>=bufsize&&bufsize!=0)
  {
    strncpy(sp, q->head->value, bufsize - 1);
    sp[bufsize - 1] = '\0';
  }
  else if(bufsize!=0){
    strncpy(sp, q->head->value, bufsize);
  }
  list_ele_t *removed_head;
  removed_head = q->head;
  if (q->head->next==NULL){
    q->tail=NULL;
  }
  q->head = q->head->next;
  free_node(removed_head);
  q->node_count--;
  return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
  if (q == NULL || q->head == NULL)
  {
    fprintf(stderr, "Queue is empty");
    return 0;
  }
  return q->node_count;
}

/*
  Reverse elements in queue by switching
  current node, previous node, and next node
  No effect if q is NULL or empty
 */
void q_reverse(queue_t *q)
{
  if (q == NULL || q->head == NULL)
  {
    return;
  }
  list_ele_t *current_node = q->head;
  list_ele_t *next_node = q->head;
  list_ele_t *previous_node = NULL;
  if (q->node_count == 1)
  {
    return;
  }
  while (next_node != NULL && q->node_count != 1)
  {
    next_node = current_node->next;
    current_node->next = previous_node;
    previous_node = current_node;
    current_node = next_node;
    next_node = next_node->next;
  }
  current_node->next = previous_node;
  q->tail = q->head;
  q->head = current_node;
}
