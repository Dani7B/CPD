#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define INDEX(i,j,rowlen) ((i)*(rowlen)+j)

int main(int argc, char** argv) {
	float *A, *scattered;
	float *x, *result, *global, finale;
	int rank, size, chunk;
	unsigned int i, j, righe, colonne;

	FILE* file = fopen("C:\\Users\\Daniele\\Documents\\Visual Studio 2012\\Projects\\Test\\Debug\\Prova.txt","r");
	if(file==NULL)
		return 1;
	fscanf(file,"%u", &righe);
	fscanf(file,"%u", &colonne);
	if(righe!=colonne)
		return 1;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	x = malloc(righe * sizeof(float));
	chunk = ceil((double)(righe * colonne)/size);
	scattered = malloc(chunk * sizeof(float));
	global = malloc(righe * colonne * sizeof(float));

	if(rank == 0) {
		for(i = 0; i < colonne; i++)
			fscanf(file,"%f",&x[i]);
		A = malloc(righe * colonne * sizeof(float));
		for(i=0; i<righe; i++) {
			for(j=0; j<colonne; j++) {
				fscanf(file,"%f",&A[INDEX(i,j,colonne)]);
				if(j==i) {
					if(A[INDEX(i,j,colonne)] != 0) {
						return 1;
						exit;
					}
				}
				else if(j<i) {
					if(A[INDEX(i,j,colonne)] != -A[INDEX(j,i,colonne)]) {
						return 1;
						exit;
					}
				}
			}
		}
	}

	MPI_Bcast(x, colonne, MPI_FLOAT, 0 , MPI_COMM_WORLD); // invio X a tutti gli altri

	MPI_Scatter (A, chunk, MPI_FLOAT, scattered, chunk, MPI_FLOAT, 0, MPI_COMM_WORLD);
		
	/*
	for(i=0; i<colonne; i++) {
		printf("Rank %d Elemento %f \t ",rank, scattered[i]);
	} */
	result = malloc(chunk * sizeof(float));
	result[0] = 0;
	for(i=0;i<chunk;i++) {
			result[0] += scattered[i]*x[i];
			if(i!=0)
				result[i] = 0;
	}

	MPI_Gather(&result[0],1,MPI_FLOAT,global,1,MPI_FLOAT, 0 ,MPI_COMM_WORLD);
	
	if(rank == 0) {
		finale = 0;
		for(i=0;i<chunk;i++)
			finale += global[i] * x[i];
		printf("La matrice e' antisimmetrica e la norma quindi e' %f", finale);
	}

	MPI_Finalize();
	return 0;
}

/*
int prova(int argc, char* argv[]) {
	float *A;
	float *x;
	float n, temp, res;
	unsigned int i, j, righe, colonne, k;
	FILE* file = fopen("C:\\Users\\Daniele\\Documents\\Visual Studio 2012\\Projects\\Test\\Debug\\Prova.txt","r");
	
	if(file==NULL)
		return 1;
	fscanf(file,"%u", &righe);
	fscanf(file,"%u", &colonne);
	if(righe!=colonne)
		return 1;
	x = malloc(righe * sizeof(float));
	A = malloc(righe * colonne * sizeof(float*));
	for(i = 0; i < colonne; i++)
		fscanf(file,"%f",&x[i]);
	res = 0.0;
	for(i=0; i<righe; i++) {
		temp = 0.0;
		for(j=0; j<colonne; j++) {
			if(j>i) {
				k = j-i;
				fscanf(file,"%f",&A[INDEX(i,k,colonne)]);
				temp += A[INDEX(i,k,colonne)] * x[j];
			}
			else if(j==i) {
				k = j-i;
				fscanf(file,"%f",&n);
				if(n != 0.0) {
					return 1;
					exit;
				}
				A[INDEX(i,k,colonne)] = n;
				temp += n * x[j];
			}
			else {
				k = i-j;
				fscanf(file,"%f",&n);
				if(n != -A[INDEX(j,k,colonne)]) {
					return 1;
					exit;
				}
				temp += x[j]*n;
			}
		}
		res += temp;
	}
	printf("La matrice e' antisimmetrica e la norma quindi e' %f",res);
	return 0;
}
*/
