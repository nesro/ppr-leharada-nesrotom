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

typedef struct stack {
	int size;
	int items_cnt;
	void *items;
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

	if ((bit_array->data = malloc(n * sizeof(char))) == NULL) {
		fprintf(stderr, "malloc bit_array data has failed\n");
		exit(EXIT_FAILURE);
	}

	return bit_array;
}

static void
bit_array_free(bit_array_t *bit_array)
{
	assert(bit_array != NULL);

	free(bit_array->data);
	free(bit_array);
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
	/* TODO */
	return 0;
}


/******************************************************************************/
/* function for i-domintaion */

/* FIXME: this was not tested yet !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

static void
domination_rec(graph_t *graph, bit_array_t *domination, int node, int depth)
{
	int i;

	domination->data[node] = 1;

	if (depth == 0)
		return;

	for (i = 0; i < graph->n; i++)
		if (graph->am[node][i])
			domination_rec(graph, domination, graph->am[node][i],
			    depth - 1);
}

/*
   solution is a bit_array that marks which nodes are creating an i-dominator
*/
static int
is_solution(graph_t *graph, int i_domination, bit_array_t *solution)
{
	int i;
	bit_array_t *domination;

	assert(i_domination >= 0);

	domination = bit_array_init(graph->n);

	/* recursively marks neighbours of nodes from solution in distance
	   i_dominance */
	for (i = 0; i < graph->n; i++)
		domination_rec(graph, domination, solution->data[i],
		    i_domination);

	/* check if all nodes are marked */
	for (i = 0; i < graph->n; i++) {
		if (domination->data[i] == 0) {
			bit_array_free(domination);
			return 0;
		}
	}

	bit_array_free(domination);
	return 1;
}

/******************************************************************************/
/* main */

int
main(int argc, char *argv[])
{
	graph_t *graph;

	if (argc != 2) {
		printf("Usage: %s matrixfile\n", argv[0]);
		return EXIT_FAILURE;
	}

	graph = graph_load(argv[1]);
	graph_print(graph);
	graph_free(graph);

	return EXIT_SUCCESS;
}

