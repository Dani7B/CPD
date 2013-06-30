#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define INDEX(i,j,rowlen) ((i*rowlen)+j)

typedef struct coppia {
	float value;
	int index;
} pair;

int comp (const void * elem1, const void * elem2) {
	pair f,s;
	f = *((pair*)elem1);
	s = *((pair*)elem2);
    if (f.value > s.value)
		return  1;
    if (f.value < s.value)
		return -1;
    return 0;
}

pair calcolaNorma(float* experimental, float* numeric, int k, int index) {
	float result;
	int i;
	pair *coppia;

	result = 0;
	coppia = malloc(sizeof(pair));

	for(i=0; i<k; i++)
		result += pow(experimental[i] - numeric[i],2);

	coppia->index = index;
	coppia->value = sqrt(result);
	return *coppia;
}


int main( int argc, char *argv[] ) {

    int rank, size, rest, n, k, i, j, righeXproc, numProc, soFar;
    int *righeProcessi;
	float d;
	FILE *numeric, *experimental;
	float *num, *exp, *localNum, *localExp;
	pair *localResult, *result;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(argc<6) {
		printf("Numero argomenti non sufficiente: %d richiesto %d", argc, 6);
		MPI_Abort(MPI_COMM_WORLD, 0);
		return 1;
	}

	n = atoi(argv[3]);
	k = atoi(argv[4]);
	d = atof(argv[5]);

	if(n <= 0) {
		printf("Il parametro n non è un intero positivo");
		MPI_Abort(MPI_COMM_WORLD, 0);
		return 1;
	}
	if(d <= 0) {
		printf("Il parametro d non è un intero positivo");
		MPI_Abort(MPI_COMM_WORLD, 0);
		return 1;
	}
	if(k <= 0) {
		printf("Il parametro k non è un float");
		MPI_Abort(MPI_COMM_WORLD, 0);
		return 1;
	}

	righeXproc=n/size;
	rest=n%size;

	if(rank<rest)
		righeXproc++;

	if(rank>0)
		MPI_Send (&righeXproc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);


	if(rank==0) {
		numeric = fopen(argv[1],"r"); // numeric
		experimental = fopen(argv[2],"r"); // experimental

		if(numeric==NULL) {
			printf("Non è stato possibile aprire il file: %s", argv[1]);
			MPI_Abort(MPI_COMM_WORLD, 0);
			return 1;
		}

		if(experimental==NULL) {
			printf("Non è stato possibile aprire il file: %s", argv[2]);
			MPI_Abort(MPI_COMM_WORLD, 0);
			return 1;
		}

		localResult = malloc(sizeof(pair)*righeXproc);
		localNum = malloc(sizeof(float)*k*righeXproc);
		localExp = malloc(sizeof(float)*k*righeXproc);

		righeProcessi = malloc(sizeof(int)*(size-1));
		result = malloc(sizeof(pair)*n);

		for(i=1; i<size; i++)
			MPI_Recv(&righeProcessi[i-1], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		num = malloc(sizeof(float)*k*righeXproc);// sicuramente visto che sono root, tutti gli altri hanno <= righe
		exp = malloc(sizeof(float)*k*righeXproc);
		numProc = 0;
		soFar = 0;
		for(i=0; i<n; i++) {
			for(j=0; j<k; j++) {
				if(numProc == 0) { // 0 legge e si tiene le righe
					fscanf(numeric,"%f",&localNum[soFar*k+j]);
					fscanf(experimental,"%f",&localExp[soFar*k+j]);
				}
				else {
					fscanf(numeric,"%f",&num[soFar*k+j]);
					fscanf(experimental,"%f",&exp[soFar*k+j]);
				}
			}
			soFar++;
			if(numProc == 0 && soFar == righeXproc) {
				soFar = 0;
				numProc++;
			}

			if(numProc > 0 && soFar == righeProcessi[numProc-1]){
				MPI_Send(num, righeProcessi[numProc-1]*k, MPI_FLOAT, numProc, 1, MPI_COMM_WORLD);
				MPI_Send(exp, righeProcessi[numProc-1]*k, MPI_FLOAT, numProc, 2, MPI_COMM_WORLD);
				soFar = 0;
				numProc++;
			}
		}
		fclose(numeric);
		fclose(experimental);
		free(num);
		free(exp);

		for(i=0;i<righeXproc;i++)
			for(j=0;j<k;j++)
				result[i] = calcolaNorma(&localExp[i*k],&localNum[i*k],k,(rank*righeXproc)+i);
		soFar = righeXproc;
		for(i=1; i<size; i++){
			MPI_Send(&soFar, 1, MPI_INT, i, 4, MPI_COMM_WORLD);
			MPI_Recv(&result[soFar], righeProcessi[i-1], MPI_FLOAT_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			soFar += righeProcessi[i-1];
		}

		qsort(result, n, sizeof(pair),comp);
		for(i=0; i<n; i++) {
			if(result[i].value<d)
				printf("%d ", result[i].index);
		}

		free(righeProcessi);
		free(localNum);
		free(localExp);
		free(localResult);
		free(result);
	}

	else {
		localResult = malloc(sizeof(pair)*righeXproc);
		localNum = malloc(sizeof(float)*k*righeXproc);
		localExp = malloc(sizeof(float)*k*righeXproc);

		MPI_Recv(localNum, k*righeXproc, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(localExp, k*righeXproc, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		MPI_Recv(&soFar, 1, MPI_INT, 0, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		for(i=0;i<righeXproc;i++)
			for(j=0;j<k;j++)
				localResult[i] = calcolaNorma(&localExp[i*k],&localNum[i*k],k,soFar+i);

		MPI_Send (localResult, righeXproc, MPI_FLOAT_INT, 0, 3, MPI_COMM_WORLD); // non necessario

		free(localNum);
		free(localExp);
		free(localResult);
	}

	MPI_Finalize();
    return 0;
}
