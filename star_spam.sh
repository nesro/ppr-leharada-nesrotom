#!/bin/bash

graph[0]="1o"
dom[0]=4
graph[1]="2o"
dom[1]=1
graph[2]="3o"
dom[2]=2

main() {
	for (( g=0; g < ${#graph[@]}; g++ )); do
		for (( cpus=2; cpus < 33; cpus++ )); do
			wait_for_slot
			echo ./measure/graph${graph[g]}.txt ${dom[g]} $cpus
			run_job $cpus ./measure/graph${graph[g]}.txt ${dom[g]}
		done
	done
}

function wait_for_slot {
	if [[ "$(hostname)" != "star" ]]; then
		return
	fi
	while :; do
		sleep 5
		if (( $( qstat | grep $(whoami) | wc -l ) < 2 )); then
			return
		fi
	done
}

# $1 = # of cpus
# $2 = graph name
# $3 = i-domination
run_job() {
	f=__________$(date +%s)_$1_${2##*.}_$3.sh
		cat <<__EOF__ >>$f
#!/bin/sh

#  ===========================================================================
# |                                                                           |
# |             COMMAND FILE FOR SUBMITTING SGE JOBS                          |
# |                                                                           |
# |                                                                           |
# | SGE keyword statements begin with #$                                      |
# |                                                                           |
# | Comments begin with #                                                     |
# | Any line whose first non-blank character is a pound sign (#)              |
# | and is not a SGE keyword statement is regarded as a comment.              |
#  ===========================================================================

#  ===========================================================================
# |                                                                           |
# | Request Bourne shell as shell for job                                     |
# |                                                                           |
#  ===========================================================================
#$ -S /bin/sh

#  ===========================================================================
# |                                                                           |
# | Execute the job from the current working directory.                       |
# |                                                                           |
#  ===========================================================================
#$ -cwd

#  ===========================================================================
# |                                                                           |
# | Defines or redefines the path used for the standard error stream          |
# | of the job.                                                               |
# |                                                                           |
#  ===========================================================================
#$ -e .

#  ===========================================================================
# |                                                                           |
# | The path used for the standard output stream of the job.                  |
# |                                                                           |
#  ===========================================================================
#$ -o .

#  ===========================================================================
# |                                                                           |
# | Specifies that all environment variables active within the qsub utility   |
# | be exported to the context of the job.                                    |
# |                                                                           |
#  ===========================================================================
#$ -V

#  ===========================================================================
# |                                                                           |
# | Set network communications - over Ethernet or InfiniBand.                 |
# |   false - Network communications over Ethernet                            |
# |   true  - Network communications over Infniband                           |
# |                                                                           |
#  ===========================================================================
INFINIBAND="true"

#  ===========================================================================
# |                                                                           |
# | Parallel program with arguments.                                          |
# |                                                                           |
#  ===========================================================================
MY_PARALLEL_PROGRAM="./main --mpi $2 $3"

#  ===========================================================================
# |                                                                           |
# | Export environment variable to execution nodes                            |
# |                                                                           |
#  ===========================================================================
# export MY_VARIABLE1="..."
# export MY_VARIABLE2="..."

# %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#  ===========================================================================
# |                                                                           |
# | !!! Do not change !!!                                                     |
# | mpirun -np $NSLOTS                                                        |
# | !!! Do not change !!!                                                     |
# |                                                                           |
#  ===========================================================================

if [[ \${INFINIBAND} = "true" ]]
then
#  -------------------------------------------------------------------------
# | Set network communication openMPI between nodes - InfiniBand (openib)   |
#  -------------------------------------------------------------------------
	mpirun -np \$NSLOTS \${MY_PARALLEL_PROGRAM}
else
#  -------------------------------------------------------------------------
# | Set network communication openMPI between nodes - Ethernet (tcp)        |
#  -------------------------------------------------------------------------
	mpirun --mca btl tcp,self -np \$NSLOTS \${MY_PARALLEL_PROGRAM}
fi
__EOF__
	chmod +x $f
	qrun.sh $1 long $f 
}

main
