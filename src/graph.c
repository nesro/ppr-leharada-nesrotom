#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include "graph.h"

graph_t *
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

	graph->diameter = 0;

	return (graph);
}

graph_t *
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

	graph->diameter = graph_diameter(graph);

	return (graph);
}

void
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

void
graph_free(graph_t *graph)
{
	assert(graph != NULL);
	free(graph->am);
	free(graph->_data);
	free(graph);
}

void
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

int
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

int
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

void
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

/* return an array how the nodes were swapped,
   this will change the graph! */
int *
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

