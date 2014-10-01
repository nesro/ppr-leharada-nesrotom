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
    int **distances;
    int i;
    int j;
    int k;
    int max;

	/* allocate an array of size n*n for distances calculation*/
	if ((distances = malloc(graph->n * sizeof(int*)))
	    == NULL) {
		fprintf(stderr, "malloc distances data has failed\n");
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < graph->n; i++){
        if ((distances[i] = malloc(graph->n * sizeof(int)))
            == NULL) {
            fprintf(stderr, "malloc distances data has failed\n");
            exit(EXIT_FAILURE);
        }
	}

	/* initialize distances matrix*/
	for(i = 0; i < graph->n; i++){
        for(j = 0; j < graph->n; j++){
            if(i == j){
                distances[i][j] = 0;
            }
            else if(graph->am[i][j] == 1){
                distances[i][j] = 1;
            }
            else{
                distances[i][j] = INT_MAX;
            }
        }
	}

	/* compute matrix of distances */
	for(i = 0; i < graph->n; i++){
        for(j = 0; j < graph->n; j++){
            for(k = 0; k < graph->n; k++){
                /* test to infinity because it can overflow */
                if(distances[j][k] > distances[j][i] + distances[i][k] &&
		    distances[j][i] != INT_MAX && distances[i][k] != INT_MAX){
                    distances[j][k] = distances[j][i] + distances[i][k];
                }
            }
        }
	}

	/* choose graph diamener, maximum non infinite number in distances matrix */
	max = 0;
	for(i = 0; i < graph->n; i++){
        for(j = 0; j < graph->n; j++){
            if(distances[i][j] > max && distances[i][j] < INT_MAX){
                max = distances[i][j];
            }
        }
	}

	/* free allocated space */
	assert(distances != NULL);

	for(i = 0; i < graph->n; i++){
        free(distances[i]);
	}
	free(distances);

	return max;
}


/******************************************************************************/
/* function for i-domintaion */

/* FIXME: this was not tested yet !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

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
		if (solution->data[i])
			domination_rec(graph, domination, i,
			    i_domination);

	/* check if all nodes are marked */
	for (i = 0; i < graph->n; i++) {
		if (domination->data[i] == 0) {
			bit_array_free(domination);
			printf("solution not found. missing %d\n", i);
			return 0;
		}
	}

	bit_array_free(domination);

	printf("solution found!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! nodes=");
	bit_array_print(solution);

	return 1;
}

static void
solution_try_all_rec(graph_t *graph, int i_domination, bit_array_t *solution,
    int nodes_min, int nodes_max, int from_position, int nodes)
{
	int i;
	bit_array_t *tmp_solution;

	printf("trying from=%d, nodes=%d solution=", from_position, nodes);
	bit_array_print(solution);

	if (nodes_max < nodes)
		return;

	tmp_solution = bit_array_clone(solution);

	for (i = from_position; i < graph->n; i++) {
		tmp_solution->data[i] = 1;

		solution_try_all_rec(graph, i_domination, tmp_solution,
		    nodes_min, nodes_max, from_position + 1, nodes + 1);

		if (nodes_min <= nodes)
		       is_solution(graph, i_domination, tmp_solution);

		tmp_solution->data[i] = 0;
	}

	bit_array_free(tmp_solution);
}

static void
solution_try_all(graph_t *graph, int i_domination)
{
	int diameter;
	int nodes_min;
	int nodes_max;
	bit_array_t *solution;

	assert(i_domination >= 0);

	solution = bit_array_init(graph->n);

	/* TODO: test for these values */
	diameter = graph_diameter(graph);
	nodes_min = ceil(diameter/(2*i_domination + 1));
	nodes_max = ceil(graph->n/(2*i_domination + 1)) + 1;

	printf("diameter=%d, nodes_min=%d, nodes_max=%d\n", diameter, nodes_min,
	    nodes_max);

	assert(nodes_min <= nodes_max);
	assert(nodes_min > 0);
	assert(nodes_max <= graph->n);
	assert(i_domination == 0 && nodes_max == graph->n);

	solution_try_all_rec(graph, i_domination, solution, nodes_min,
	    nodes_max, 0, 0);

	bit_array_free(solution);
}

/******************************************************************************/
/* LEHARADA */

static int
is_solution_leharada(bit_array_t *solution, bit_array_t *dominated_nodes)
{
    int i;

    for(i = 0; i < dominated_nodes->n; i++){
        if(dominated_nodes->data[i] == 0) return 0;
    }

    printf("SOLUTION: ");
    bit_array_print(solution);

    return 1;
}

static void
add_dominated_nodes_rec_leharada(graph_t *graph, int level, int actual_node, int last_node, bit_array_t *dominated_nodes)
{
    int i;

    if(level == 0) return;

    for(i = 0; i < graph->n; i++){
        if(graph->am[actual_node][i] == 1 && i != last_node){
            dominated_nodes->data[i] = 1;
            add_dominated_nodes_rec_leharada(graph, level - 1, i, actual_node, dominated_nodes);
        }
    }
}

static void
add_dominated_nodes_leharada(graph_t *graph, int i_domination, int node_index, bit_array_t *dominated_nodes)
{
    dominated_nodes->data[node_index] = 1;
    add_dominated_nodes_rec_leharada(graph, i_domination, node_index, -1, dominated_nodes);
}

static void
solution_try_all_rec_leharada(graph_t *graph, int i_domination, bit_array_t *solution, bit_array_t *dominated_nodes)
{
    int i;
    int res;
	bit_array_t *tmp_solution;
	bit_array_t *tmp_dominated_nodes;

	res = is_solution_leharada(solution, dominated_nodes);
	if(res == 1) return;

	for(i = 0; i < graph->n; i++){
        if(solution->data[i] == 0){
            tmp_solution = bit_array_clone(solution);
            tmp_dominated_nodes = bit_array_clone(dominated_nodes);

            tmp_solution->data[i] = 1;
            add_dominated_nodes_leharada(graph, i_domination, i, tmp_dominated_nodes);

            solution_try_all_rec_leharada(graph, i_domination, tmp_solution, tmp_dominated_nodes);

            bit_array_free(tmp_solution);
            bit_array_free(tmp_dominated_nodes);
        }
	}
}

static void
solution_try_all_leharada(graph_t *graph, int i_domination)
{
	bit_array_t *solution;
	bit_array_t *dominated_nodes;

	assert(i_domination >= 0);

	solution = bit_array_init(graph->n);
	dominated_nodes = bit_array_init(graph->n);

    solution_try_all_rec_leharada(graph, i_domination, solution, dominated_nodes);

	bit_array_free(solution);
	bit_array_free(dominated_nodes);
}

/******************************************************************************/
/* main */

int
main(int argc, char *argv[])
{
	graph_t *graph;
	long int i_domination;
	char *tmp_c;

	if (argc != 3) {
		printf("Usage: %s matrixfile i-domination\n", argv[0]);
		return EXIT_FAILURE;
	}

	i_domination = strtol(argv[2], &tmp_c, 10);

	graph = graph_load(argv[1]);

	solution_try_all_leharada(graph, (int) i_domination);

	graph_free(graph);

	return EXIT_SUCCESS;
}

