#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"


bit_array_t *
bit_array_init(int n)
{
	bit_array_t *bit_array;

	assert(n > 0);

	if ((bit_array = malloc(sizeof(bit_array_t))) == NULL) {
		fprintf(stderr, "malloc bit_array has failed\n");
		exit(EXIT_FAILURE);
	}

	if ((bit_array->data = calloc(n, sizeof(char))) == NULL) {
		fprintf(stderr, "calloc bit_array data has failed\n");
		exit(EXIT_FAILURE);
	}
	
	bit_array->n = n;

	return (bit_array);
}

bit_array_t *
bit_array_clone(bit_array_t *bit_array)
{
	bit_array_t *clone;

	clone = bit_array_init(bit_array->n);
	memcpy(clone->data, bit_array->data, bit_array->n);

	return (clone);
}

void
bit_array_copy(bit_array_t *destination, bit_array_t *source)
{
	assert(destination != NULL);
	assert(source != NULL);
	assert(destination->n == source->n);

	memcpy(destination->data, source->data, destination->n);
}

void
bit_array_print(bit_array_t *bit_array)
{
	int i;

	for (i = 0; i < bit_array->n; i++)
		printf("%c", bit_array->data[i] == 1 ? '1' : '0');

	printf("\n");
}

void
bit_array_free(bit_array_t *bit_array)
{
	assert(bit_array != NULL);

	free(bit_array->data);
	free(bit_array);
}

int
bit_array_count_nodes(bit_array_t *bit_array)
{
	int i;
	int counter = 0;

	for (i = 0; i < bit_array->n; i++){
		if(bit_array->data[i] == 1){
			counter++;
		}
	}

	return (counter);
}

/******************************************************************************/
/* stack_item functions */

stack_item_t *
stack_item_init(int n)
{
	stack_item_t *stack_item;

	if ((stack_item = malloc(sizeof(stack_item_t))) == NULL) {
		fprintf(stderr, "malloc stack_item has failed\n");
		exit(EXIT_FAILURE);
	}

	stack_item->solution = bit_array_init(n);
	stack_item->dominated_nodes = bit_array_init(n);
	stack_item->level = 0;

	return (stack_item);
}

stack_item_t *
stack_item_clone(stack_item_t *stack_item)
{

	stack_item_t *clone;

	if ((clone = malloc(sizeof(stack_item_t))) == NULL) {
		fprintf(stderr, "malloc clone of stack_item has failed\n");
		exit(EXIT_FAILURE);
	}

	clone->solution = bit_array_clone(stack_item->solution);
	clone->dominated_nodes = bit_array_clone(stack_item->dominated_nodes);
	clone->level = stack_item->level;

	return (clone);
}

void
stack_item_free(stack_item_t *stack_item)
{
	if (stack_item == NULL)
		return;

	bit_array_free(stack_item->solution);
	bit_array_free(stack_item->dominated_nodes);
	free(stack_item);
}

/******************************************************************************/
/* stack functions */

stack_t *
stack_init(void)
{
	stack_t *stack;

	if ((stack = malloc(sizeof(stack_t))) == NULL) {
		fprintf(stderr, "malloc stack has failed\n");
		exit(EXIT_FAILURE);
	}

	if ((stack->items = malloc(DEFAULT_STACK_SIZE * sizeof(stack_item_t *)))
	    == NULL) {
		fprintf(stderr, "malloc stack items has failed\n");
		exit(EXIT_FAILURE);
	}

	stack->size = DEFAULT_STACK_SIZE;
	stack->top = 0;
	stack->bottom = 0;

	return (stack);
}

int
stack_items(stack_t *stack)
{
	return (stack->top - stack->bottom);
}

int
stack_is_empty(stack_t *stack)
{
	return (stack_items(stack) == 0);
}

void
stack_delete_items(stack_t *stack)
{
	while (!stack_is_empty(stack))
		stack_item_free(stack_pop(stack));
}

void
stack_free(stack_t *stack)
{
	stack_delete_items(stack);

	free(stack->items);
	free(stack);
}

void
stack_push(stack_t *stack, stack_item_t *item)
{
	stack_item_t **tmp_items;

	if(stack->top >= stack->size) {
		stack->size *= 2;

		if ((tmp_items = realloc(stack->items, stack->size *
		    sizeof(stack_item_t *))) == NULL) {
			stack_free(stack);
			fprintf(stderr, "stack realloc has failed\n");
			exit(EXIT_FAILURE);
		}

		stack->items = tmp_items;
	}

	stack->items[stack->top] = item;
	stack->top++;
}

stack_item_t *
stack_pop(stack_t *stack)
{
	assert(stack != NULL);

	if (stack_is_empty(stack))
		return (NULL);

	stack->top--;
	return (stack->items[stack->top]);
}

stack_item_t *
stack_divide(stack_t *stack)
{
	stack_item_t *item;

	if (stack_is_empty(stack))
		return (NULL);

	item = stack->items[stack->bottom];
	stack->bottom++;

	return (item);
}

