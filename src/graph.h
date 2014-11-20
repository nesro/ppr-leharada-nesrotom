
#ifndef GRAPH_H
#define GRAPH_H

typedef struct graph {
	int n;
	char **am; /* adjacency matrix */
	int diameter;

	/* private */
	char *_data;
} graph_t;

graph_t *graph_init(int n);
graph_t *graph_load(const char *filename);
void graph_save(graph_t *graph, const char *filename);
void graph_free(graph_t *graph);
void graph_print(graph_t *graph);
int graph_diameter(graph_t *graph);
int graph_node_count_domination(graph_t *graph, int node, int domination);
void graph_swap_nodes(graph_t *graph, int a, int b);
int * graph_domination_sort(graph_t *graph, int i_domination);

#endif /* GRAPH_H */

