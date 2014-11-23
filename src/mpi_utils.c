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

char *g_mpi_tags[] = { "STACK", "BEST_NODES", "SOLUTION", "TOKEN",
	"NEED_JOB", "NO_JOB", "FINALIZE", "FINALIZE_OK", "INIT_STACK" };

void
mpi_send_best_solution_nodes(problem_t *problem)
{
	int i;

	for (i = 0; i < problem->mpi_cpus; i++) {
		if (i == problem->mpi_rank)
			continue;

		mpi_printf(problem, "MPI_Send cpu=%d nodes=%d BEST_NODES\n", i,
		    problem->best_solution_nodes);
		MPI_Send(&problem->best_solution_nodes, 1, MPI_INT, i,
		    TAG_BEST_NODES, MPI_COMM_WORLD);
	}
}


void
mpi_recv_stack(problem_t *problem)
{
	int stack_items;
	int pack_position;
	int i;
	stack_item_t *item;

	MPI_Recv(problem->buffer, BUFFER_LENGTH, MPI_PACKED, MPI_ANY_SOURCE,
	    MPI_ANY_TAG, MPI_COMM_WORLD, &problem->status);

	pack_position = 0;
	MPI_Unpack(problem->buffer, BUFFER_LENGTH, &pack_position, &stack_items,
	    1, MPI_INT, MPI_COMM_WORLD);

	for (i = 0; i < stack_items; i++) {
		item = stack_item_init(problem->graph->n);

		MPI_Unpack(problem->buffer, BUFFER_LENGTH, &pack_position,
		    item->solution->data, problem->graph->n, MPI_CHAR,
		    MPI_COMM_WORLD);
		MPI_Unpack(problem->buffer, BUFFER_LENGTH, &pack_position,
		    item->dominated_nodes->data, problem->graph->n, MPI_CHAR,
		    MPI_COMM_WORLD);
		MPI_Unpack(problem->buffer, BUFFER_LENGTH, &pack_position,
		    &item->level, 1, MPI_INT, MPI_COMM_WORLD);

		stack_push(problem->stack, item);
	}
}

/* pri ptani o praci si pamatuju koho jsem se naposledy zeptal. zeptam se
 * jen jednoho procesoru a vyskocim z funkce, abych pak mel moznost vysledek
 * prijmout
 *
 * nastavim si, ze cekam na no_job a nebudu se ptat nikoho dalsiho, dokud
 * mi ta no_job neprijde */
void
mpi_send_need_job(problem_t *problem)
{
	mpi_printf(problem, "I'm asking for a job.\n");

	mpi_printf(problem, "MPI_Send cpu=%d NEED_JOB\n",
	    problem->mpi_cpu_to_ask);
	MPI_Send(NULL, 0, MPI_CHAR, problem->mpi_cpu_to_ask, TAG_NEED_JOB,
	    MPI_COMM_WORLD);

	problem->mpi_waiting_for_no_job = TRUE;

	problem->mpi_cpu_to_ask++;
	if (problem->mpi_cpu_to_ask % problem->mpi_cpus == 0)
		problem->mpi_cpu_to_ask = 0;
}

/******************************************************************************/
/* TOKEN */

void
mpi_send_master_finalize(problem_t *problem)
{
	int i;

	/* TODO: poslat vsem cpu zpravu o ukonceni a oni poslou bud OK, nebo
	 * nejlepsi reseni co spocitali
	 * vytvorit tagy TAG_FINALIZE, TAG_FINALIZE_OK */

	/* send broadcast with TAG_FINALIZE */
	for (i = 1; i < problem->mpi_cpus; i++) {
		mpi_printf(problem, "MPI_Send cpu=%d FINALIZE\n", i);
		MPI_Send(NULL, 0, MPI_CHAR, i, TAG_FINALIZE, MPI_COMM_WORLD);
	}

	/* ted prijmu vsechny zpravy, skoncim, az mi prijdou vsechny odpovedi */
	while (!(problem->master_finalize_answers == problem->mpi_cpus - 1))
		mpi_recv(problem);

	/* ted bysme uz meli mit nejlepsi reseni . nee, to  bude pred finalize
	   mpi_printf(problem, "MASTER RESULT %d ", problem->best_solution_nodes);
	   bit_array_print(problem->best_solution);*/

	/* ted muzeme ukoncit i mastera */
	problem->finalize = TRUE;
}

/* takhle funkce se zavola, kdyz mame token u sebe */
void
mpi_handle_token(problem_t *problem)
{
	/* pokud jsem master a uz mi dosla prace, vyslu peska
	 * je potreba ho vyslat jen jednou */
	if (problem->mpi_rank == MASTER_CPU &&
	    !problem->master_token_dispatched &&
	    stack_is_empty(problem->stack)) {

		problem->master_token_dispatched = TRUE;
		problem->token_dirty = TOKEN_CLEAN;

		mpi_printf(problem, "MPI_Send cpu=%d FIRST TOKEN\n", 1);
		MPI_Send(&problem->token_dirty, 1, MPI_INT, 1, TAG_TOKEN,
		    MPI_COMM_WORLD);

		return;
	}

	if (!problem->token_have || problem->finalize)
		return;

	/* pokud mame prazdny zasobnik, posleme peska dal. pesek si zachova
	 * svoji spinavost. po odeslani peska se ale my ocistime */
	if (stack_is_empty(problem->stack)) {

		if (problem->token_dirty == TOKEN_DIRTY)
			problem->token = TOKEN_DIRTY;
		else
			problem->token = TOKEN_CLEAN;

		mpi_printf(problem, "MPI_Send cpu=%d TOKEN dirty=%d\n",
		    ((problem->mpi_rank + 1) % problem->mpi_cpus),
		    problem->token);
		MPI_Send(&problem->token, 1, MPI_CHAR,
		    ((problem->mpi_rank + 1) % problem->mpi_cpus), TAG_TOKEN,
		    MPI_COMM_WORLD);

		problem->token_dirty = TOKEN_CLEAN;
		problem->token_have = FALSE;
	}
}

void
mpi_recv_token(problem_t *problem)
{
	MPI_Recv(&problem->token, 1, MPI_INT, problem->status.MPI_SOURCE,
	    MPI_ANY_TAG, MPI_COMM_WORLD, &problem->status);
	mpi_printf(problem, "MPI_Recv from=%d TOKEN dirty=%d\n",
	    problem->status.MPI_SOURCE, problem->token);

	problem->token_have = TRUE;

	if (problem->mpi_rank == MASTER_CPU && problem->token == TOKEN_CLEAN) {
		mpi_printf(problem, "FINALIZE mpi_send_master_finalize\n");
		mpi_send_master_finalize(problem);
	}

}

/******************************************************************************/

void
mpi_send(problem_t *problem)
{
	/* zadame praci, pokud
	 * 1) mame prazdny stack
	 * 2) necekame na odpoved na predchozi pozadavek o praci
	 * 3) neukoncujeme */
	if (stack_is_empty(problem->stack) && !problem->mpi_waiting_for_no_job
	    && !problem->finalize)
		mpi_send_need_job(problem);

	mpi_handle_token(problem);
}

/******************************************************************************/

void
mpi_recv_need_job(problem_t *problem)
{
	int items_to_send;
	int pack_position;
	int i;
	int j;
	int one_item_length;
	int max_items_in_part;
	int parts_to_send;
	int actual_item;
	int left_send;
	int actual_to_send;
	int mpi_char_size;
	int mpi_int_size;
	stack_item_t *item;

	MPI_Recv(NULL, 0, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
	    &problem->status);

	if (stack_items(problem->stack) < 5) {
		mpi_printf(problem, "MPI_Send cpu=%d NO_JOB\n",
		    problem->status.MPI_SOURCE);
		MPI_Send(NULL, 0, MPI_CHAR, problem->status.MPI_SOURCE,
		    TAG_NO_JOB, MPI_COMM_WORLD);
		return;
	}

	/* jdeme poslat praci, takze musime zkontrolovat, zdali se nemame
	 * ospinit */

	/* XXX: TODO: FIXME: */
	if (problem->status.MPI_SOURCE < problem->mpi_rank){
		mpi_printf(problem, "Sending job to lower cpu I'm TOKEN DIRTY\n");
		problem->token_dirty = TOKEN_DIRTY;
	}

	mpi_char_size = 1;
	mpi_int_size = 4;

	one_item_length = 2 * problem->graph->n * mpi_char_size + mpi_int_size;
	items_to_send = (int)(stack_items(problem->stack) / 3);
	max_items_in_part = BUFFER_LENGTH / one_item_length;
	parts_to_send = items_to_send / max_items_in_part + 1;
	actual_item = 0;

	for(j = 0; j < parts_to_send; j++){
		pack_position = 0;
		left_send = items_to_send - actual_item;
		actual_to_send = fmin(left_send, max_items_in_part);
		MPI_Pack(&actual_to_send, 1, MPI_INT, problem->buffer, BUFFER_LENGTH,
		    &pack_position, MPI_COMM_WORLD);
		for (i = 0; i < actual_to_send; i++) {
			item = stack_divide(problem->stack);

			assert(item != NULL);
			assert(item->solution != NULL);
			assert(item->solution->data != NULL);
			assert(problem->buffer != NULL);

			MPI_Pack(item->solution->data, problem->graph->n, MPI_CHAR,
			    problem->buffer, BUFFER_LENGTH, &pack_position,
			    MPI_COMM_WORLD);
			MPI_Pack(item->dominated_nodes->data, problem->graph->n,
			    MPI_CHAR, problem->buffer, BUFFER_LENGTH, &pack_position,
			    MPI_COMM_WORLD);
			MPI_Pack(&item->level, 1, MPI_INT, problem->buffer,
			    BUFFER_LENGTH, &pack_position, MPI_COMM_WORLD);

			stack_item_free(item);
			actual_item++;
		}

		mpi_printf(problem, "MPI_Send cpu=%d items=%d STACK\n",
		    problem->status.MPI_SOURCE, items_to_send);
		MPI_Send(problem->buffer, pack_position, MPI_PACKED,
		    problem->status.MPI_SOURCE, TAG_STACK, MPI_COMM_WORLD);
	}

}

void
mpi_recv_best_nodes(problem_t *problem)
{
	int best_nodes_recv;
	MPI_Recv(&best_nodes_recv, 1, MPI_INT, MPI_ANY_SOURCE,
	    MPI_ANY_TAG, MPI_COMM_WORLD, &problem->status);

	mpi_printf(problem, "got best nodes=%d from=%d, i have=%d\n",
	    best_nodes_recv, problem->status.MPI_SOURCE,
	    problem->best_solution_nodes);

	if (best_nodes_recv < problem->best_solution_nodes) {
		problem->best_solution_nodes = best_nodes_recv;
		problem->best_solution_i_computed_it = FALSE;
	}
}

void
mpi_recv_finalize(problem_t *problem)
{
	/* prijali jsme zpravu o ukonceni vypoctu. pokud nejme ti co vypocitali
	 * nejlepsi reseni tak posleme ok, pokud jsme ti co ho vypocitali,
	 * posleme uzly masterovi */

	assert(stack_is_empty(problem->stack));

	MPI_Recv(NULL, 0, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
	    &problem->status);

	if (problem->best_solution_i_computed_it) {
		mpi_printf(problem, "MPI_Send cpu=%d SOLUTION\n", MASTER_CPU);
		MPI_Send(problem->best_solution->data, problem->graph->n,
		    MPI_CHAR, MASTER_CPU, TAG_SOLUTION, MPI_COMM_WORLD);
	} else {
		mpi_printf(problem, "MPI_Send cpu=%d FINALIZE_OK\n",
		    MASTER_CPU);
		MPI_Send(NULL, 0, MPI_CHAR, MASTER_CPU, TAG_FINALIZE_OK,
		    MPI_COMM_WORLD);
	}

	problem->finalize = TRUE;
}

void
mpi_recv(problem_t *problem)
{
	int flag;

	/* for (;;) { */
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag,
		    &problem->status);

		if (!flag)
			return;

		mpi_printf(problem, "Iprobe tag=%s(%d) from=%d\n",
		    g_mpi_tags[problem->status.MPI_TAG],
		    problem->status.MPI_TAG, problem->status.MPI_SOURCE);

		switch (problem->status.MPI_TAG) {
		case TAG_NEED_JOB:
			mpi_recv_need_job(problem);
			break;
		case TAG_TOKEN:
			mpi_recv_token(problem);
			break;
		case TAG_BEST_NODES:
			mpi_recv_best_nodes(problem);
			break;
		case TAG_FINALIZE:
			assert(problem->mpi_rank != MASTER_CPU);
			mpi_recv_finalize(problem);
			break;
		case TAG_NO_JOB:
			MPI_Recv(NULL, 0, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG,
			    MPI_COMM_WORLD, &problem->status);
			problem->mpi_waiting_for_no_job = FALSE;
			break;
		case TAG_STACK:
			problem->mpi_waiting_for_no_job = FALSE;
			mpi_recv_stack(problem);
			break;
		case TAG_FINALIZE_OK:
			assert(problem->mpi_rank == MASTER_CPU);
			problem->master_finalize_answers++;
			MPI_Recv(NULL, 0, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG,
			    MPI_COMM_WORLD, &problem->status);
			break;
		case TAG_SOLUTION:
			assert(problem->mpi_rank == MASTER_CPU);
			problem->master_finalize_answers++;

			MPI_Recv(problem->best_solution->data,
			    problem->graph->n, MPI_CHAR, MPI_ANY_SOURCE,
			    MPI_ANY_TAG, MPI_COMM_WORLD, &problem->status);

			problem->best_solution_nodes =
			    bit_array_count_nodes(problem->best_solution);
			break;
		default:
			mpi_printf(problem, "unknown tag\n");
			assert(0);
			break;
		}
	/* } */
}


void
mpi_init_master_cpu(problem_t *problem)
{
	int i;
	int pack_position;
	stack_item_t *item;

	item = stack_item_init(problem->graph->n);
	problem_stack_expand(problem, item);
	stack_item_free(item);

	for (i = 1; i < problem->mpi_cpus; i++){
		item = stack_divide(problem->stack);

		if (item == NULL)
			break;

		pack_position = 0;
		MPI_Pack(&item->level, 1, MPI_INT, problem->buffer,
		    BUFFER_LENGTH, &pack_position, MPI_COMM_WORLD);
		MPI_Pack(item->solution->data, problem->graph->n, MPI_CHAR,
		    problem->buffer, BUFFER_LENGTH, &pack_position,
		    MPI_COMM_WORLD);
		MPI_Pack(item->dominated_nodes->data, problem->graph->n,
		    MPI_CHAR, problem->buffer, BUFFER_LENGTH, &pack_position,
		    MPI_COMM_WORLD);
		mpi_printf(problem, "MPI_Send cpu=%d INIT_STACK\n", i);
		MPI_Send(problem->buffer, pack_position, MPI_PACKED, i,
		    TAG_INIT_STACK, MPI_COMM_WORLD);

		stack_item_free(item);
	}
}

void
mpi_init_slave_cpu(problem_t *problem)
{
	int pack_position;
	stack_item_t *item;

	if(problem->mpi_rank < problem->graph->n){
		item = stack_item_init(problem->graph->n);

		pack_position = 0;

		MPI_Recv(problem->buffer, BUFFER_LENGTH, MPI_PACKED, MASTER_CPU,
		    TAG_INIT_STACK, MPI_COMM_WORLD, &problem->status);

		MPI_Unpack(problem->buffer, BUFFER_LENGTH, &pack_position,
		    &item->level, 1, MPI_INT, MPI_COMM_WORLD);

		MPI_Unpack(problem->buffer, BUFFER_LENGTH, &pack_position,
		    item->solution->data, problem->graph->n, MPI_CHAR,
		    MPI_COMM_WORLD);

		MPI_Unpack(problem->buffer, BUFFER_LENGTH, &pack_position,
		    item->dominated_nodes->data, problem->graph->n, MPI_CHAR,
		    MPI_COMM_WORLD);

		mpi_printf(problem, "received the first job\n");

		stack_push(problem->stack, item);
	}
}

/******************************************************************************/
/* entry points from the main function */

int
main_mpi(int *argc, char **argv[], const char *graph_filename, int i_domination)
{
	problem_t *problem;
	stack_item_t *item;
	double time_elapsed[2];
	double time_init;
	double time_finalize;
	unsigned long int cycle;

	problem = problem_init(graph_filename, i_domination);

	MPI_Init(argc, argv);
	mpi_printf(problem, "MPI_Init OK\n");

	MPI_Comm_rank(MPI_COMM_WORLD, &problem->mpi_rank);
	mpi_printf(problem, "MPI_Comm_rank OK rank=%d\n", problem->mpi_rank);

	time_init = MPI_Wtime();

	problem->mpi_on = TRUE;

	MPI_Comm_size(MPI_COMM_WORLD, &problem->mpi_cpus);
	mpi_printf(problem, "MPI_Comm_size OK cpus=%d\n", problem->mpi_cpus);

	problem->mpi_cpu_to_ask = (problem->mpi_rank + 1) % problem->mpi_cpus;

	if (problem->mpi_rank == MASTER_CPU) {
		mpi_init_master_cpu(problem);
	} else {
		mpi_init_slave_cpu(problem);
	}

	time_elapsed[0] = MPI_Wtime();
	for (cycle = 0; !problem->finalize; cycle++) {

		if(cycle%50000==0)
			printf("cpu=%d cycle=%ld, computed_items=%d "
			    "stack_items=%d dirty=%d, have=%d\n",
			    problem->mpi_rank,
			    cycle,
			    problem->computed_items,
			    stack_items(problem->stack),
			    problem->token_dirty,
			    problem->token_have);

		if ((item = stack_pop(problem->stack))) {
			problem->computed_items++;

			if (item->level >= problem->best_solution_nodes ||
			    item->level > problem->nodes_max) {
				stack_item_free(item);
				continue;
			}

			if (problem_is_solution(problem, item)) {
				if (item->level <= problem->nodes_min) {
					stack_item_free(item);
					break;
				}

				stack_item_free(item);
				continue;
			}

			problem_stack_expand(problem, item);
			stack_item_free(item);
		} else {
			/* nemam nic na stacku, takze budu spamovat ostatni */
			/* takze radsi si dachnu */
			sleep(3);
		}

		/* pokud uz nebylo nic v zasoniku, tak item je null */
		if (item == NULL ||
		    (cycle % CYCLES_MPI_MESSAGES) == 0) {

			time_elapsed[1] = MPI_Wtime();

			mpi_recv(problem);
			mpi_send(problem);

			time_elapsed[0] = MPI_Wtime();
		}
	}

	time_elapsed[0]=time_elapsed[1];

	time_finalize = MPI_Wtime();
	mpi_printf(problem, "Finalize! Total time=%lf\n",
	    time_finalize - time_init);
	MPI_Finalize();


	sleep(2);
	printf("I, %d,  did %d work. My best solution %d\n", problem->mpi_rank,
	    problem->computed_items, problem->best_solution_nodes);
	bit_array_print(problem->best_solution);

	if (problem->mpi_rank == MASTER_CPU) {
		/* aby byl vystup na konci, tak se prospime */
		sleep(2);

		printf("XXXXXXXXXXXXXXXXXXX\nRESULT: %d ",
		    problem->best_solution_nodes);
		bit_array_print(problem->best_solution);
	}

	problem_free(problem);

	return (EXIT_SUCCESS);
}

void
mpi_printf(problem_t *problem, const char *format, ...)
{
	unsigned char code_hi;
	unsigned char code_lo;
	va_list args;

	/*pokud neni zaply debug, nevypisovat tyhle volani */
#if !(DEBUG)
	return;
#endif /* DEBUG */

	code_lo = 30 + (4 - problem->mpi_rank + 64) % 8;
	code_hi = !(((problem->mpi_rank) % 16) / 8);
	fprintf(stdout, "\x1b[%d;%dm[%d][%d] ", code_hi, code_lo,
	    problem->mpi_rank, stack_items(problem->stack));
	fflush(stdout);

	va_start(args, format);
	(void)vfprintf(stdout, format, args);
	va_end(args);

	printf("\x1b[0m");
}

