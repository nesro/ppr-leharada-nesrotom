#include <errno.h>
#include <assert.h>
#include <limits.h> /* INT_MAX */

#ifndef UTILS_H
#define UTILS_H

#ifndef DEBUG

#define ASSERT(condition) do { \
	if (!condition) { \
		fprintf(stderr, "%s: %s:%d: %s: Assertion `%s' failed.\n", \
		    __BASE_FILE_,  __FILE__, __LINE__, __FUNCTION__, \
		    #condition); \
		abort(); \
	} \
} while (0)

#define DEBUG_PRINT(message) do { \
	fprintf(stderr, "DEBUG: %s:%d: %s: %s", __FILE__,  __LINE__, \
	    __FUNCTION__, message); \
} while (0)

#define DEBUG_PRINTF(message, ...) do { \
	fprintf(stderr, "DEBUG: %s:%d: %s: %s", __FILE__,  __LINE__, \
	    __FUNCTION__, message, __VA_ARGS__); \
} while (0)

#define DEBUG_CALL(call) call

#else /* DEBUG */

#define ASSERT(condition) ((void) 0)
#define DEBUG_PRINT(message) ((void) 0)
#define DEBUG_PRINTF(message, ...) ((void) 0)
#define DEBUG_CALL(call) ((void) 0)

#endif /* DEBUG */

typedef struct bit_array {
	int n;
	char *data;
} bit_array_t;


#define DEFAULT_STACK_SIZE 10000

typedef struct stack_item {
	bit_array_t *solution;
	bit_array_t *dominated_nodes;
	int level;
} stack_item_t;

typedef struct stack {
	int bottom;
	int top;
	int size;
	stack_item_t **items; /* array of pointers */
} stack_t;

bit_array_t *bit_array_init(int n);
bit_array_t *bit_array_clone(bit_array_t *bit_array);
void bit_array_copy(bit_array_t *destination, bit_array_t *source);
void bit_array_print(bit_array_t *bit_array);
void bit_array_free(bit_array_t *bit_array);
int bit_array_count_nodes(bit_array_t *bit_array);

stack_item_t *stack_item_init(int n);
stack_item_t *stack_item_clone(stack_item_t *stack_item);
void stack_item_free(stack_item_t *stack_item);
stack_t *stack_init(void);
int stack_is_empty(stack_t *stack);
stack_item_t *stack_pop(stack_t *stack);
void stack_free(stack_t *stack);
void stack_push(stack_t *stack, stack_item_t *item);
stack_item_t *stack_divide(stack_t *stack);
void stack_delete_items(stack_t *stack);
int stack_items(stack_t *stack);
void stack_delete_items(stack_t *stack);

#endif /* UTILS_H */
