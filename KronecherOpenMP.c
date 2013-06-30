#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char *argv[]) {
	float *A, *B, *C; // Assumo che B sia la matrice più grande e sia l'ultima letta dal file
	int check, i, j, k, righeA, colonneA, righeB, colonneB, righeC, colonneC;

	if(argc < 2) {
		printf("Number of arguments not sufficient. Is %d, %d needed\n", argc-1, 1);
		return 1;
	}

	FILE* file = fopen(argv[1],"rb");
	if(file==NULL) {
		printf("Couldn't open file %s\n", argv[1]);
		return 1;
	}

	check = 0;
	check += fread(&righeA, sizeof(int), 1, file);
	check += fread(&colonneA, sizeof(int), 1, file);
	check += fread(&righeB, sizeof(int), 1, file);
	check += fread(&colonneB, sizeof(int), 1, file);

	if(check < 4) {
		printf("Check if row and column numbers in file are consistent\n");
		return 1;
	}

	righeC = righeA * righeB;
	colonneC = colonneA * colonneB;
	A = malloc(sizeof(float) * righeA * colonneA);
	B = malloc(sizeof(float) * righeB * colonneB);
	C = malloc(sizeof(float) * righeC * colonneC);

	/* Lettura di A */
	fread(A, sizeof(float), righeA * colonneA, file);

	/* Lettura di B */
	fread(B, sizeof(float), righeB * colonneB, file);

	/* Calcolo della matrice risultante */
	#pragma omp parallel for shared(C)
	for(i = 0; i < righeA * colonneA; i++) {
		for (j = 0; j < righeB * colonneB; j++) {
			k = cIndex(i,j,righeA,colonneA,righeB,colonneB);
			C[k] = A[i] * B[j];
		}
	}

	for(i = 0; i < righeC * colonneC; i++)
			printf("%f ", C[i]);

	free(A);
	free(B);
	free(C);

	return 0;
}
