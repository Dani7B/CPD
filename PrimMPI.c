#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MAXIMUM_DOUBLE_VALUE 99999
#define INDEX(i,j,rowlen) ((i*rowlen)+j)

int main (int argc, char **argv){

	FILE *f;
	int rank, size, n, check, righeXproc, rest, offset, min, lineNumber;
	double *partialMatrix; 
	int *visited;
	int i, j, fromIndex, toIndex, iterationCounter, minProcess;
	double minVal, receivedVal, total;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(argc<2) {
		printf("Numero argomenti non sufficiente: %d richiesto %d\n", argc-1, 1);
		MPI_Abort(MPI_COMM_WORLD, 0);
		return 1;
	}


	/* For Testing only:
	if (rank==0){
		int s=7;
		double v1[7]={0, 7, MAXIMUM_DOUBLE_VALUE, 5, MAXIMUM_DOUBLE_VALUE, MAXIMUM_DOUBLE_VALUE, MAXIMUM_DOUBLE_VALUE};
		double v2[7]={7, 0, 8, 9, 7, MAXIMUM_DOUBLE_VALUE, MAXIMUM_DOUBLE_VALUE};
		double v3[7]={MAXIMUM_DOUBLE_VALUE, 8, 0, MAXIMUM_DOUBLE_VALUE, 5, MAXIMUM_DOUBLE_VALUE, MAXIMUM_DOUBLE_VALUE};
		double v4[7]={5, 9, MAXIMUM_DOUBLE_VALUE, 0, 15, 6, MAXIMUM_DOUBLE_VALUE};
		double v5[7]={MAXIMUM_DOUBLE_VALUE, 7, 5, 15, 0, 8, 9};
		double v6[7]={MAXIMUM_DOUBLE_VALUE, MAXIMUM_DOUBLE_VALUE, MAXIMUM_DOUBLE_VALUE, 6, 8, 0, 11};
		double v7[7]={MAXIMUM_DOUBLE_VALUE, MAXIMUM_DOUBLE_VALUE, MAXIMUM_DOUBLE_VALUE, MAXIMUM_DOUBLE_VALUE, 9, 11, 0};

		f = fopen(argv[1], "wb");
		fwrite(&s, sizeof(int), 1, f);
		fwrite(v1, sizeof(double), 7, f);
		fwrite(v2, sizeof(double), 7, f);
		fwrite(v3, sizeof(double), 7, f);
		fwrite(v4, sizeof(double), 7, f);
		fwrite(v5, sizeof(double), 7, f);
		fwrite(v6, sizeof(double), 7, f);
		fwrite(v7, sizeof(double), 7, f);

		fclose(f);
	}*/

	f = fopen(argv[1], "rb");

	if(f == NULL) {
		printf("Couldn't open file %s\n", argv[1]);
		MPI_Abort(MPI_COMM_WORLD, 0);
		return 1;
	}

	check = fread(&n, sizeof(int), 1, f);

	if(check < 1) {
		printf("Value is not an integer\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
		return 1;
	}

	righeXproc=n/size;
	rest=n%size;
 
	min = (rank<rest)?rank:rest;
	lineNumber = rank*righeXproc+min;

	if (rank < rest) 
		righeXproc++;	// Le righe rimanenti vengono assegnate ai primi REST processi


	offset = lineNumber*n*sizeof(double)+sizeof(int);
	fseek(f, offset, SEEK_SET);

	partialMatrix = malloc(sizeof(double)*righeXproc*n);
	visited = malloc(sizeof(int)*n);

	fread(partialMatrix, sizeof(double), n*righeXproc, f);

	visited[0] = 1; // Inserisco il primo nodo tra i visitati

	fromIndex=-1;
	toIndex=-1;
	total = 0;

	for (iterationCounter=1; iterationCounter<n; iterationCounter++){
		minVal=MAXIMUM_DOUBLE_VALUE;

		// Ogni processo calcola, per ogni riga visitata, la distanza minima dai nodi da visitare 
		for (i=0; i<righeXproc; i++){
			if (visited[i+lineNumber]==1){
				for (j=0; j<n; j++){
					if (partialMatrix[INDEX(i,j,n)]<minVal && visited[j]!=1){
						minVal=partialMatrix[INDEX(i,j,n)];
						if(rank<rest)
							fromIndex = righeXproc*rank + i;
						else
							fromIndex = i + rest * (righeXproc+1) + (rank - size + rest + 1) * righeXproc;
						toIndex=j;
					}
				}
			}
		}

		minProcess=0;

		// ogni processo invia al master il minimo
		if (rank>0)
			MPI_Send(&minVal,1,MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);

		// il master sceglie il minimo migliore
		else {
			for (i=1; i<size; i++){
				MPI_Recv(&receivedVal, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
				if (receivedVal<minVal){
					minVal=receivedVal;
					minProcess=i;
				}
			}
		}


		// 0 comunica a tutti i processi chi di loro ha il "minimo globale"
		MPI_Bcast(&minProcess, 1, MPI_INT, 0, MPI_COMM_WORLD);

		// questo processo invia a tutti il nodo da visitare
		MPI_Bcast(&toIndex, 1, MPI_INT, minProcess, MPI_COMM_WORLD);

		visited[toIndex]=1;
			
		// Bisogna recuperare anche il nodo di partenza dell'arco da inserire	
		if(rank == minProcess && rank > 0)
			MPI_Send(&fromIndex, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);

		if (rank==0) {
			if(minProcess == 0)
				printf("Iteration %d: (%d,%d) weight %lf\n", iterationCounter, fromIndex, toIndex, minVal);
			else {
				MPI_Recv(&fromIndex, 1, MPI_INT, minProcess, 2, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
				printf("Iteration %d: (%d,%d) weight %lf\n", iterationCounter, fromIndex, toIndex, minVal);
			}
			total += minVal;
		}
	}

	if(rank == 0)
		printf("Total weight: %lf\n", total);
	
	free(partialMatrix);
	free(visited);

	MPI_Finalize();
	return 0;
}