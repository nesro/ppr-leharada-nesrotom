
#include "mpi.h"
#include "utils.h"
#include "graph.h"
#include "problem.h"

#ifndef MPI_UTILS_H
#define MPI_UTILS_H

#define MASTER_CPU 0
#define TAG_CNT 4

#define TAG_STACK 0 /* sending part of stack */
#define TAG_BEST_NODES 1 /* sending number of nodes in best soution */
#define TAG_SOLUTION 2 /* sending nodes of the best solution */
#define TAG_TOKEN 3 /* sedning token */
#define TAG_NEED_JOB 4 /* if we ask for job */
#define TAG_NO_JOB 5 /* we were asked for job but we don't have any */
#define TAG_FINALIZE 6 /* master received clean token and is sending info */
#define TAG_FINALIZE_OK 7 /* we received ^ and don't have the best solution */
#define TAG_INIT_STACK 8 /* sending the initial work. i.e. one item */


#define PROCCESSORS_MAX 12

#define TOKEN_DEFAULT 0
#define TOKEN_CLEAN 1
#define TOKEN_DIRTY 2

/* XXX: after how many cycles we recieve and send messages */
#define CYCLES_MPI_MESSAGES 1000

#define DEFAULT_STACK_SIZE 10000
#define BUFFER_LENGTH 990


void mpi_send_best_solution_nodes(problem_t *problem);
void mpi_recv_stack(problem_t *problem);
void mpi_send_need_job(problem_t *problem);
void mpi_send_master_finalize(problem_t *problem);
void mpi_handle_token(problem_t *problem);
void mpi_recv_token(problem_t *problem);
void mpi_send(problem_t *problem);
void mpi_recv_need_job(problem_t *problem);
void mpi_recv_best_nodes(problem_t *problem);
void mpi_recv_finalize(problem_t *problem);
void mpi_recv(problem_t *problem);
void mpi_init_master_cpu(problem_t *problem);
void mpi_init_slave_cpu(problem_t *problem);
int main_mpi(int *argc, char **argv[], const char *graph_filename,
    int i_domination);
void mpi_printf(problem_t *problem, const char *format, ...);

#endif /* MPI_UTILS_H */

