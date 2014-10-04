#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define n 5

int stack[1000][n];
int stackp=0;

foo(int *a,int from, int to, int level){
	int i,j,*b=calloc(n,sizeof(int));
	memcpy(b,a,n*sizeof(int));

	for(j=0;j<level*4;j++)
		printf(" ");
	for(i=0;i<n;i++)
		printf("%d",b[i]);
	printf("\n");

	for(i=to-1;i>=from;i--) {
		b[i]=1;
		foo(b,0,i,level+1);
		memcpy(stack[stackp++],b,n*sizeof(int));
		b[i]=0;
	}

	free(b);
}

main(){
	int *g,i,j;
	g=calloc(n,sizeof(int));
	foo(g,0, n, 0);
	free(g);

	for(i=stackp;i>=0;i--){
		for(j=0;j<n;j++)
			printf("%d",stack[i][j]);
		printf("\n");
	}
	return 0;
}

