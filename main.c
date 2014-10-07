/*
 * leharada@fit.cvut.cz
 * nesrotom@fit.cvut.cz
 * https://github.com/nesro/ppr-leharada-nesrotom
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h> /* INT_MAX */
#include <math.h> /* ceil */

#define DEFAULT_STACK_SIZE 10000

/******************************************************************************/
/* structure definitions */

typedef struct bit_array {
	int n;
	char *data;
} bit_array_t;

typedef struct graph {
	int n;
	char **am; /* adjacency matrix */

	/* private */
	char *_data;
} graph_t;

typedef struct stack_item {
	bit_array_t *solution;
	bit_array_t *dominated_nodes;
	int level;
} stack_item_t;

typedef struct stack {
	int size;
	int items_cnt;
	stack_item_t **items; /* array of pointers */
} stack_t;

/******************************************************************************/
/* bit_array functions */

static bit_array_t *
bit_array_init(int n)
{
	bit_array_t *bit_array;

	assert(n > 0);

	if ((bit_array = malloc(sizeof(bit_array_t))) == NULL) {
		fprintf(stderr, "malloc bit_array has failed\n");
		exit(EXIT_FAILURE);
	}

	bit_array->n = n;

	if ((bit_array->data = calloc(n, sizeof(char))) == NULL) {
		fprintf(stderr, "calloc bit_array data has failed\n");
		exit(EXIT_FAILURE);
	}

	return bit_array;
}

static bit_array_t *
bit_array_clone(bit_array_t *bit_array)
{
	bit_array_t *clone;

	clone = bit_array_init(bit_array->n);
	memcpy(clone->data, bit_array->data, bit_array->n);

	return clone;
}

static void
bit_array_copy(bit_array_t *destination, bit_array_t *source)
{
	assert(destination != NULL);
	assert(source != NULL);
	assert(destination->n == source->n);

	memcpy(destination->data, source->data, destination->n);
}

static void
bit_array_print(bit_array_t *bit_array)
{
	int i;

	for (i = 0; i < bit_array->n; i++)
		printf("%c", bit_array->data[i] == 1 ? '1' : '0');

	printf("\n");
}

static void
bit_array_free(bit_array_t *bit_array)
{
	assert(bit_array != NULL);

	free(bit_array->data);
	free(bit_array);
}

/******************************************************************************/
/* stack_item functions */

static stack_item_t *
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

	return stack_item;
}

static stack_item_t *
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

	return clone;
}

static void
stack_item_free(stack_item_t *stack_item)
{
	assert(stack_item != NULL);

	bit_array_free(stack_item->solution);
	bit_array_free(stack_item->dominated_nodes);
	free(stack_item);
}

/******************************************************************************/
/* stack functions */

static stack_t *
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
	stack->items_cnt = 0;

	return stack;
}

static int
stack_is_empty(stack_t *stack)
{
	assert(stack != NULL);

	return (stack->items_cnt == 0);
}

static stack_item_t *
stack_pop(stack_t *stack)
{
	assert(stack->items_cnt > 0);

	stack->items_cnt--;
	return stack->items[stack->items_cnt];
}

static void
stack_free(stack_t *stack)
{

	assert(stack != NULL);

	while (!stack_is_empty(stack))
		stack_item_free(stack_pop(stack));

	free(stack->items);
	free(stack);
}

static void
stack_push(stack_t *stack, stack_item_t *item)
{
	stack_item_t **tmp_items;

	if(stack->items_cnt >= stack->size) {
		stack->size *= 2;

		if ((tmp_items = realloc(stack->items, stack->size *
		    sizeof(stack_item_t *))) == NULL) {
			stack_free(stack);
			fprintf(stderr, "stack realloc has failed\n");
			exit(EXIT_FAILURE);
		}

		stack->items = tmp_items;
	}

	stack->items[stack->items_cnt] = item;
	stack->items_cnt++;
}


/******************************************************************************/
/* graph functions */

static graph_t *
graph_init(int n)
{
	int i;
	graph_t *graph;

	assert(n > 0);

	if ((graph = malloc(sizeof(graph_t))) == NULL) {
		fprintf(stderr, "malloc graph has failed\n");
		exit(EXIT_FAILURE);
	}

	graph->n = n;

	/* allocate an array of pointers */
	if ((graph->am = malloc(graph->n * sizeof(char *)))
	    == NULL) {
		fprintf(stderr, "malloc graph ajd ptrs has failed\n");
		exit(EXIT_FAILURE);
	}

	/* allocate an array of size n*n */
	if ((graph->_data = malloc(graph->n * graph->n * sizeof(char)))
	    == NULL) {
		fprintf(stderr, "malloc graph data adj mtx has failed\n");
		exit(EXIT_FAILURE);
	}

	/* map pointers to rows */
	for (i = 0; i < graph->n; i++)
		graph->am[i] = graph->_data + i * graph->n;

	return graph;
}

static graph_t *
graph_load(const char *filename)
{
	FILE *f;
	int n;
	int i;
	int j;
	char c;
	graph_t *graph;

	if ((f = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "failed to open %s: %s\n", filename,
		    strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (fscanf(f, "%d", &n) != 1) {
		fprintf(stderr, "loading of N has failed\n");
		exit(EXIT_FAILURE);
	}

	graph = graph_init(n);

	for (i = 0; i < n; i++) {
		c = fgetc(f);
		if (c != '\n') {
			fprintf(stderr, "expected \\n, loaded: \"%c\"\n", c);
			exit(EXIT_FAILURE);
		}

		for (j = 0; j < n; j++) {
			c = fgetc(f);
			switch (c) {
			case '0':
				graph->am[i][j] = 0;
				break;
			case '1':
				graph->am[i][j] = 1;
				break;
			default:
				fprintf(stderr, "i=%d,j=%d, expected 0 or 1, "
				    "loaded: \"%c\"\n", i, j, c);
				exit(EXIT_FAILURE);
			}
		}
	}

	fclose(f);

	return graph;
}

static void
graph_save(graph_t *graph, const char *filename)
{
	FILE *f;
	int i;
	int j;

	if ((f = fopen(filename, "w+")) == NULL) {
		fprintf(stderr, "failed to open %s: %s\n", filename,
		    strerror(errno));
		exit(EXIT_FAILURE);
	}

	fprintf(f, "%d\n", graph->n);
	for (i = 0; i < graph->n; i++) {
		for (j = 0; j < graph->n; j++) {
			fprintf(f, "%d", graph->am[i][j]);
		}

		fprintf(f, "\n");
	}

	fclose(f);
}

static void
graph_free(graph_t *graph)
{
	assert(graph != NULL);
	free(graph->am);
	free(graph->_data);
	free(graph);
}

static void
graph_print(graph_t *graph)
{
	int i;
	int j;

	assert(graph != NULL);

	printf("%d\n", graph->n);

	for (i = 0; i < graph->n; i++) {
		for (j = 0; j < graph->n; j++)
			printf("%d", graph->am[i][j]);

		printf("\n");
	}
}

static int
graph_diameter(graph_t *graph)
{
	int **distances;
	int *_distances_data;
	int i;
	int j;
	int k;
	int max;

	/* allocate an array of size n*n for distances calculation */
	if ((distances = malloc(graph->n * sizeof(int*)))
	    == NULL) {
		fprintf(stderr, "malloc distances has failed\n");
		exit(EXIT_FAILURE);
	}

	/* allocate an array of size n*n */
	if ((_distances_data = malloc(graph->n * graph->n * sizeof(int)))
	    == NULL) {
		fprintf(stderr, "malloc distances data has failed\n");
		exit(EXIT_FAILURE);
	}

	/* map pointers to rows */
	for (i = 0; i < graph->n; i++)
		distances[i] = _distances_data + i * graph->n;

	/* initialize distances matrix*/
	for (i = 0; i < graph->n; i++) {
	for (j = 0; j < graph->n; j++) {
		if (i == j) {
			distances[i][j] = 0;
		} else if (graph->am[i][j] == 1) {
			distances[i][j] = 1;
		} else {
			distances[i][j] = INT_MAX;
		}
	}
	}

	/* compute matrix of distances */
	for (i = 0; i < graph->n; i++) {
	for (j = 0; j < graph->n; j++) {
	for (k = 0; k < graph->n; k++) {
		/* test to infinity because it can overflow */
		if(distances[j][k] > distances[j][i] + distances[i][k] &&
		    distances[j][i] != INT_MAX &&
		    distances[i][k] != INT_MAX) {
			distances[j][k] = distances[j][i] + distances[i][k];
		}
	}
	}
	}

	/* choose graph diamener, maximum non infinite number in distances
	   matrix */
	max = 0;
	for (i = 0; i < graph->n; i++) {
	for (j = 0; j < graph->n; j++) {
		if (distances[i][j] > max && distances[i][j] < INT_MAX) {
			max = distances[i][j];
		}
	}
	}

	/* free allocated space */
	free(distances);
	free(_distances_data);

	return max;
}

static int
graph_node_count_domination(graph_t *graph, int node, int domination)
{
	int i;
	int result = 0;

	if (domination == 0)
		return 1;

	for (i = 0; i < graph->n; i++)
		if (graph->am[node][i] == 1) /* XXX */
			result += graph_node_count_domination(graph, i,
			    domination - 1);

	return result;
}

static void
graph_swap_nodes(graph_t *graph, int a, int b)
{
	int i;
	int tmp;

	/* swap a and b column */
	for (i = 0; i < graph->n; i++) {
		tmp = graph->am[i][a];
		graph->am[i][a] = graph->am[i][b];
		graph->am[i][b] = tmp;
	}

	/* swap a and b row */
	for (i = 0; i < graph->n; i++) {
		tmp = graph->am[a][i];
		graph->am[a][i] = graph->am[b][i];
		graph->am[b][i] = tmp;
	}
}

/* return an array how the nodes were swapped */
static int *
graph_domination_sort(graph_t *graph, int i_domination)
{
	int i;
	int j;
	int tmp;
	int *nodes_domination; /* how many nodes the node dominates */
	int *trace;

	if ((nodes_domination = malloc(graph->n * sizeof(int))) == NULL) {
		fprintf(stderr, "gnd mem\n");
		exit(EXIT_FAILURE);
	}

	if ((trace = malloc(graph->n * sizeof(int))) == NULL) {
		fprintf(stderr, "trace mem\n");
		exit(EXIT_FAILURE);
	}


	for (i = 0; i < graph->n; i++) {
		nodes_domination[i] = graph_node_count_domination(graph, i,
		    i_domination);
		printf("dom i=%d=%d\n", i, nodes_domination[i]);
	}

	for (i = 0; i < graph->n; i++)
		trace[i] = i;

	/* TODO: quicksort */
	/* Now we need to sort nodes by their domination in adjacency matrix
	   and in our nodes_domination array. Algorithm: Bubble sort. */
	for (i = 0; i < graph->n - 1; i++) {
		for (j = 0; j < graph->n - i - 1; j++) {
			if (nodes_domination[j] < nodes_domination[j + 1]) {
				tmp = nodes_domination[j];
				nodes_domination[j] = nodes_domination[j + 1];
				nodes_domination[j + 1] = tmp;

				tmp = trace[j];
				trace[j] = trace[j + 1];
				trace[j + 1] = tmp;

				graph_swap_nodes(graph,j, j + 1);
			}
		}
	}

	free(nodes_domination);

	return trace;
}

/******************************************************************************/
/* solution */

static int
is_solution(stack_item_t *item)
{
	int i;

	for (i = 0; i < item->dominated_nodes->n; i++)
		if (item->dominated_nodes->data[i] == 0)
			return 0;

	return 1;
}

static void
add_dominated_nodes_rec(graph_t *graph, int level, int actual_node,
    int last_node, bit_array_t *dominated_nodes)
{
	int i;

	if (level == 0)
		return;

	for (i = 0; i < graph->n; i++) {
		if (graph->am[actual_node][i] == 1 && i != last_node) {
			dominated_nodes->data[i] = 1;
			add_dominated_nodes_rec(graph, level - 1, i,
			    actual_node, dominated_nodes);
		}
	}
}

static void
add_dominated_nodes(graph_t *graph, int i_domination, int node_index,
    bit_array_t *dominated_nodes)
{
	dominated_nodes->data[node_index] = 1;
	add_dominated_nodes_rec(graph, i_domination, node_index, -1,
	    dominated_nodes);
}

static bit_array_t*
solution_try_all(graph_t *graph, int i_domination)
{
	stack_t *stack;
	int i;
	int diameter;
	int nodes_min;
	int nodes_max;
	bit_array_t *best_solution;
	int best_solution_nodes;
	stack_item_t *item;
	stack_item_t *tmp_item;

	assert(i_domination >= 0);

	diameter = graph_diameter(graph);
	nodes_min = ceil(diameter/(2.0*i_domination + 1));
	nodes_max = ceil(graph->n/(2.0*i_domination + 1));

	assert(nodes_min <= nodes_max);
	assert(nodes_min > 0);
	assert(nodes_max < graph->n);
	assert(nodes_max <= graph->n);
	assert(i_domination > 0 || (i_domination == 0 && nodes_max ==
	    graph->n));

	best_solution = bit_array_init(graph->n);
	best_solution_nodes = INT_MAX;

	stack = stack_init();
	item = stack_item_init(graph->n);
	stack_push(stack, item);

	while (!stack_is_empty(stack)) {
		item = stack_pop(stack);

		/* BB if actual solution will not be better than best solution
		   or actual solution has more nodes than upper bound */
		if (item->level >= best_solution_nodes ||
		    item->level > nodes_max) {
			stack_item_free(item);
			continue;
		}

		if (is_solution(item)) {
			bit_array_copy(best_solution, item->solution);

			/* better solution don't exist, this is lower bound */
			if (item->level <= nodes_min) {
				stack_item_free(item);
				break;
			}

			stack_item_free(item);
			continue;
		}

		item->level++;

		for (i = graph->n - 1; i >= 0; i--) {
			if (item->solution->data[i] == 0) {
				tmp_item = stack_item_clone(item);

				tmp_item->solution->data[i] = 1;
				add_dominated_nodes(graph, i_domination, i,
				    tmp_item->dominated_nodes);

				stack_push(stack, tmp_item);
			}
		}

		stack_item_free(item);
	}

	stack_free(stack);

	return best_solution;
}

/******************************************************************************/
/* main */

int
main(int argc, char *argv[])
{
	graph_t *graph;
	int i_domination;
	char *tmp_c;
	int diameter_computed;
	int diameter_right;
	const char *arg_i_domination;
	const char *arg_filename_in;
	const char *arg_filename_out;
	int *trace;
	int i;
	bit_array_t *solution = NULL;

	/* ./main --optimize i-dominaton input.txt output.txt */
	if (argc == 5 && !strcmp(argv[1], "--optimize")) {
		arg_i_domination = argv[2];
		arg_filename_in = argv[3];
		arg_filename_out = argv[4];

		graph = graph_load(arg_filename_in);
		i_domination = strtol(arg_i_domination, &tmp_c, 10);

		trace = graph_domination_sort(graph, i_domination);
		graph_save(graph, arg_filename_out);

		for (i = 0; i < graph->n; i++)
			printf("%d", trace[i]);
		printf("\n");

		free(trace);
		graph_free(graph);

		return EXIT_SUCCESS;
	}

	/* ./main graph.txt --test-diameter N */
	if (argc >= 3 && !strcmp(argv[2], "--test-diameter")) {
		graph = graph_load(argv[1]);
		diameter_computed = graph_diameter(graph);
		graph_free(graph);

		diameter_right = strtol(argv[3], &tmp_c, 10);
		if (diameter_computed != diameter_right) {
			fprintf(stderr, "Diameters don't match, "
			    "computed=%d, right=%d\n", diameter_computed,
			    diameter_right);
			return EXIT_FAILURE;
		} else {
			return EXIT_SUCCESS;
		}
	}

	if (argc != 3) {
		printf("Usage: %s matrixfile i-domination\n", argv[0]);
		return EXIT_FAILURE;
	}

	i_domination = strtol(argv[2], &tmp_c, 10);

	graph = graph_load(argv[1]);

	solution = solution_try_all(graph, i_domination);
	bit_array_print(solution);

	graph_free(graph);
	bit_array_free(solution);

	return EXIT_SUCCESS;
}

