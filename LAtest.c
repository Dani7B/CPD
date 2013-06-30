#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define INDEX(i,j,rowlen) ((i)*(rowlen)+j)

int main7(int argc, char** argv) {
	float *A, *B, *scattered; // Assumo che B sia la matrice pi� grande e sia l'ultima letta dal file
	float *result, *global, finale;
	int rank, size, chunk, i, j;
	unsigned int righeA, colonneA, righeB, colonneB;

	FILE* file = fopen("C:\\Users\\Daniele\\Documents\\Visual Studio 2012\\Projects\\Test\\Debug\\K.txt","r");
	if(file==NULL)
		return 1;
	fscanf(file,"%u", &righeA);
	fscanf(file,"%u", &colonneA);
	fscanf(file,"%u", &righeB);
	fscanf(file,"%u", &colonneB);

	MPI_Init (&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	chunk = ceil((double)(righeB * colonneB)/size); // elementi per processo
	scattered = malloc(chunk * righeA * sizeof(float));
	global = malloc((righeA * righeB) * (colonneA * colonneB) * sizeof(float));

	if(rank == 0) {
		A = malloc(righeA * colonneA * sizeof(float));
		for(i=0; i<righeA; i++)
			for(j=0; j<colonneA; j++)
				fscanf(file,"%f",&A[INDEX(i,j,colonneA)]);
	}

	MPI_Bcast(A, righeA * righeB, MPI_FLOAT, 0 , MPI_COMM_WORLD); // invio A a tutti gli altri

	MPI_Scatter (A, chunk, MPI_FLOAT, scattered, chunk, MPI_FLOAT, 0, MPI_COMM_WORLD); // scatter di B
		
	/*
	for(i=0; i<colonne; i++) {
		printf("Rank %d Elemento %f \t ",rank, scattered[i]);
	} */
	result = malloc(chunk * sizeof(float));
	result[0] = 0;
	for(i=0;i<chunk;i++) {
			result[0] += scattered[i]; //*x[i];
			if(i!=0)
				result[i] = 0;
	}

	MPI_Gather(&result[0],1,MPI_FLOAT,global,1,MPI_FLOAT, 0 ,MPI_COMM_WORLD);
	
	if(rank == 0) {
		finale = 0;
		for(i=0;i<chunk;i++)
			finale += global[i]; // * x[i];
		printf("La matrice e' antisimmetrica e la norma quindi e' %f", finale);
	}

	MPI_Finalize();
	return 0;
}

/**

int cIndex(int a, int b, int righeA, int colonneA, int righeB, int colonneB) {
	int rA, cA, rB, cB;
	int rigaR, colonnaR;
	rA = a/colonneA;
	cA = a%colonneA;
	rB = b/colonneB;
	cB = b%colonneB;
	rigaR = righeB * rA + rB;
	colonnaR = colonneB * cA + cB;
	return rigaR * colonneA * colonneB + colonnaR;
}
**/
