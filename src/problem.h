#include "mpi.h"
#include "utils.h"
#include "graph.h"

#ifndef PROBLEM_H
#define PROBLEM_H

/* TODO: mpi variables shoud be in separate struct */
typedef struct problem {
	int i_domination;
	graph_t *graph;
	stack_t *stack;

	int nodes_min;
	int nodes_max;

	bit_array_t *best_solution;
	int best_solution_nodes;
	int best_solution_number_of_order; /* kolik sem uz jich nasel */
	bool_t best_solution_i_computed_it;
	bool_t opimal_solution_found;

	bool_t finalize;


	/* we need to have a nice summary, how much work the cpus did */
	int computed_items;

	/* token have is true if we have him and its set right.
	 * when we send the token, we set token_have to false */
	bool_t token_have;
	/* this is the token we've recieved */
	char token;
	/* if we send work to someone with lower id, this is true */
	bool_t token_dirty;

	/* rozdelovani prace */
	int mpi_cpu_to_ask; /* jakeho cpu jsem se ptal naposled */
	bool_t mpi_waiting_for_no_job; /* cekam na no_job/stack? */


	char *buffer;
	MPI_Status status;

	/* mpi pomocne promenne */
	bool_t mpi_on; /* co pouziva i seq reseni dovolit mpi vypnout */
	int mpi_rank;
	int mpi_cpus;


	/* nasledujici polozky muze nastatvovat jen master */
	bool_t master_token_dispatched;

	/* po poslani finalize citam tuhle promennou. az bude cpus-1, tak vim
	 * ze mam odpoved od vsech cpu a muzu se ukoncit */
	int master_finalize_answers;
} problem_t;

bool_t problem_is_solution(problem_t *problem, stack_item_t *item);
void problem_add_dominated_nodes_rec(problem_t *problem, int level,
    int actual_node, int last_node, bit_array_t *dominated_nodes);
void problem_add_dominated_nodes(problem_t *problem, int node_index,
    bit_array_t *dominated_nodes);
problem_t *problem_init(const char *graph_filename, int i_domination);
void problem_free(problem_t *problem);
void problem_stack_expand(problem_t *problem, stack_item_t *item);

#endif /* PROBLEM_H */

