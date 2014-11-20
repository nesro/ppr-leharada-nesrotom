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
#include <limits.h>
#include <math.h>
#include <unistd.h>
#include <stdarg.h>

#include "mpi.h"

#include "utils.h"
#include "graph.h"
#include "problem.h"
#include "mpi_utils.h"

/******************************************************************************/

int
main_seq(const char *graph_filename, int i_domination)
{
	problem_t *problem;
	stack_item_t *item;

	problem = problem_init(graph_filename, i_domination);

	item = stack_item_init(problem->graph->n);
	stack_push(problem->stack, item);

	while ((item = stack_pop(problem->stack))) {
		if (!problem_is_solution(problem, item))
			problem_stack_expand(problem, item);

		stack_item_free(item);
	}

	printf("RESULT: %d ", problem->best_solution_nodes);
	bit_array_print(problem->best_solution);

	problem_free(problem);

	return (EXIT_SUCCESS);
}

int
main_optimize(int i_domination, const char *in, const char *out)
{
	graph_t *graph;
	int *trace;
	int i;

	graph = graph_load(in);

	trace = graph_domination_sort(graph, i_domination);
	graph_save(graph, out);

	for (i = 0; i < graph->n; i++)
		printf("%d, ", trace[i]);
	printf("\n");

	free(trace);
	graph_free(graph);

	return (EXIT_SUCCESS);
}

int
main_test_diameter(const char *input_graph, int diameter_right)
{
	int diameter_computed;
	graph_t *graph;

	graph = graph_load(input_graph);
	diameter_computed = graph_diameter(graph);
	graph_free(graph);

	if (diameter_computed != diameter_right) {
		fprintf(stderr, "Diameters don't match, "
		    "computed=%d, right=%d\n", diameter_computed,
		    diameter_right);

		return (EXIT_FAILURE);
	} else {
		return (EXIT_SUCCESS);
	}
}

int
main(int argc, char *argv[])
{
	char *tmp_c;

	/* ./main --optimize i-dominaton input.txt output.txt */
	if (argc == 5 && !strcmp(argv[1], "--optimize"))
		return (main_optimize(strtol(argv[2], &tmp_c, 10), argv[3],
		    argv[4]));

	/* ./main --test-diameter graph.txt N */
	if (argc == 3 && !strcmp(argv[1], "--test-diameter"))
		return (main_test_diameter(argv[2], strtol(argv[3], &tmp_c,
		    10)));

	/* ./main --mpi graph.txt i_domination */
	if (argc == 4 && !strcmp(argv[1], "--mpi"))
		return (main_mpi(&argc, &argv, argv[2], strtol(argv[3],
		    &tmp_c, 10)));

	/* ./main --seq graph.txt i_domination */
	if (argc == 4 && !strcmp(argv[1], "--seq"))
		return (main_seq(argv[2], strtol(argv[3],
		    &tmp_c, 10)));

	printf("Usage:\n");
	printf("%s --optimize i-domination graph-in graph-out\n", argv[0]);
	printf("%s --test-diameter graph expected-value\n", argv[0]);
	printf("%s --seq graph i-domination\n", argv[0]);
	printf("%s --mpi graph i-domination\n", argv[0]);

	return (EXIT_FAILURE);
}

