#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define INDEX(i,j,rowlen) ((i)*(rowlen)+j)

int main(int argc, char** argv) {
	FILE* file;
	float *A, *scattered;
	float *x, *result, *global, finale, partial;
	int rank, size, chunk, remainder, sendChunk, isAntisimmetric;
	unsigned int i, j, righe, colonne;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(rank == 0) {
		if(argc<2) {
			printf("Numero argomenti non sufficiente: %d richiesto %d", argc-1, 1);
			MPI_Abort(MPI_COMM_WORLD, 0);
			return 1;
		}

		file = fopen(argv[1],"r");
		if(file==NULL) {
			printf("Non è stato possibile aprire il file: %s", argv[1]);
			MPI_Abort(MPI_COMM_WORLD, 0);
			return 1;
		}

		fscanf(file,"%u", &righe);
		fscanf(file,"%u", &colonne);
		if(righe!=colonne){
			MPI_Abort(MPI_COMM_WORLD, 0);
			return 1;
		}

		chunk = righe/size;
		remainder = righe%size;

		for(i=1; i<size; i++){
			sendChunk = chunk;
			if(i<remainder)
				sendChunk++;
			MPI_Send (&righe, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send (&sendChunk, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
		}
		if(remainder!=0)
			chunk++;
		global = malloc(righe * sizeof(float));
		x = malloc(righe * sizeof(float));
		scattered = malloc(chunk * righe * sizeof(float));
	}

	else{
		MPI_Recv (&righe, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv (&chunk, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		x = malloc(righe * sizeof(float));
		scattered = malloc(chunk * righe * sizeof(float));
	}

	if(rank == 0) {
		isAntisimmetric = 0;
		A = malloc(righe * colonne * sizeof(float));
		for(i = 0; i < colonne; i++)
			fscanf(file,"%f",&x[i]);
		for(i=0; i<righe; i++) {
			for(j=0; j<colonne; j++) {
				fscanf(file,"%f",&A[INDEX(i,j,colonne)]);
				if(j==i) {
					if(A[INDEX(i,j,colonne)] != 0)
						isAntisimmetric++;
				}
				else if(j<i) {
					if(A[INDEX(i,j,colonne)] != -A[INDEX(j,i,colonne)])
						isAntisimmetric++;
				}
			}
		}
		/*
		if(isAntisimmetric!=0){
			MPI_Abort(MPI_COMM_WORLD, 0);
			return 1;
		}*/
	}

	MPI_Bcast(x, righe, MPI_FLOAT, 0 , MPI_COMM_WORLD); // invio X a tutti gli altri

	MPI_Scatter (A, chunk*righe, MPI_FLOAT, scattered, chunk*righe, MPI_FLOAT, 0, MPI_COMM_WORLD);

	/*
	for(i=0; i<righe*chunk; i++) {
		printf("Rank %d Elemento %f \t ",rank, scattered[i]);
	}*/

	result = malloc(chunk * sizeof(float));
	partial = 0;
	for(i=0;i<chunk * righe; i++) {
		partial += scattered[i]*x[i%righe];
		if(i%righe==righe-1){
			result[i/righe] = partial;
			partial = 0;
		}
	}

	MPI_Gather(result, chunk, MPI_FLOAT, global, chunk, MPI_FLOAT, 0, MPI_COMM_WORLD);

	if(rank == 0) {
		finale = 0;
		for(i=0;i<righe;i++)
			finale += global[i] * x[i];
		if(isAntisimmetric==0)
			printf("La matrice e' antisimmetrica, quindi la sua norma e' %f \n", finale);
		else
			printf("La matrice non e' antisimmetrica e la sua norma e' %f \n", finale);
		free(global);
		free(A);
	}

	free(result);
	free(x);
	free(scattered);

	MPI_Finalize();
	return 0;
}
