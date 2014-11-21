#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>
#include <stdarg.h>

#include "utils.h"
#include "graph.h"
#include "problem.h"
#include "mpi_utils.h"

bool_t
problem_is_solution(problem_t *problem, stack_item_t *item)
{
	int i;

	if (item->level < problem->nodes_min)
		return (FALSE);

	if (item->level > problem->nodes_max)
		return (FALSE);

	if (item->level > problem->best_solution_nodes)
		return (FALSE);

	for (i = 0; i < item->dominated_nodes->n; i++)
		if (item->dominated_nodes->data[i] == 0)
			return (FALSE);

	if (item->level < problem->best_solution_nodes) {
		problem->best_solution_nodes = item->level;
		bit_array_copy(problem->best_solution, item->solution);
		problem->best_solution_i_computed_it = TRUE;

		/* nasli jsme nejlepsi reseni, musime ho broadcastovat ostatnim
		 */
		if (problem->mpi_on)
			mpi_send_best_solution_nodes(problem);
	}

	return (TRUE);
}

void
problem_add_dominated_nodes_rec(problem_t *problem, int level, int actual_node,
    int last_node, bit_array_t *dominated_nodes)
{
	int i;

	if (level == 0)
		return;

	for (i = 0; i < problem->graph->n; i++) {
		if (problem->graph->am[actual_node][i] == 1 &&
		    i != last_node) {
			dominated_nodes->data[i] = 1;
			problem_add_dominated_nodes_rec(problem, level - 1, i,
			    actual_node, dominated_nodes);
		}
	}
}

/* TODO: XXX: bylo by fajn udelat funkci, ktera proste prida dominujici uzly
 * bez zadnyho indexu. potom bych udelal jen posilani solution a zbytek by se
 * dopocital */
void
problem_add_dominated_nodes(problem_t *problem, int node_index,
    bit_array_t *dominated_nodes)
{
	dominated_nodes->data[node_index] = 1;
	problem_add_dominated_nodes_rec(problem, problem->i_domination,
	    node_index, -1, dominated_nodes);
}


/******************************************************************************/
/* problem */

problem_t *
problem_init(const char *graph_filename, int i_domination)
{
	problem_t *problem;

	assert(i_domination > 0);

	problem = calloc(1, sizeof (problem_t));
	assert(problem != NULL);

	problem->i_domination = i_domination;

	problem->graph = graph_load(graph_filename);

	problem->nodes_min = ceil(
	    problem->graph->diameter/(2.0*problem->i_domination + 1.0));

	problem->nodes_max = ceil(
	    problem->graph->n/(2.0*problem->i_domination + 1.0) + 1.0);

	assert(problem->nodes_min <= problem->nodes_max);
	assert(problem->nodes_min > 0);
	assert(problem->nodes_max < problem->graph->n);
	assert(problem->nodes_max <= problem->graph->n);
	assert(problem->i_domination > 0 || (problem->i_domination == 0 &&
	     problem->nodes_max == problem->graph->n));

	problem->stack = stack_init();

	problem->best_solution = bit_array_init(problem->graph->n);
	problem->best_solution_nodes = problem->graph->n + 1;

	problem->buffer = malloc(BUFFER_LENGTH);
	assert(problem->buffer != NULL);

	return (problem);
}

void
problem_free(problem_t *problem)
{
	graph_free(problem->graph);
	stack_free(problem->stack);
	bit_array_free(problem->best_solution);
	free(problem->buffer);
	free(problem);
}

void
problem_stack_expand(problem_t *problem, stack_item_t *item)
{
	int i;
	stack_item_t *tmp_item;

	item->level++;

	if (item->level > problem->nodes_max)
		return;

	for (i = problem->graph->n - 1; i >= 0; i--) {
		if (item->solution->data[i] == 0) {
			tmp_item = stack_item_clone(item);

			tmp_item->solution->data[i] = 1;
			problem_add_dominated_nodes(problem, i,
			    tmp_item->dominated_nodes);

			stack_push(problem->stack, tmp_item);
		}
	}
}

