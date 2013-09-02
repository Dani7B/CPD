#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int writeFile(){
	FILE *f;
	int n = 16;
	float w[16]={1.0, 3.0, 7.0, 9.0, 11.0, 13.0, 15.0, 16.0, 14.0, 12.0, 10.0, 8.0, 6.0, 5.0, 4.0, 2.0};

	f = fopen("prova.bin", "wb");
	if(f!=NULL) {
		fwrite(&n, sizeof(int), 1, f);
		fwrite(w, sizeof(float), 16, f);
	}
	fclose(f);
	return 0;
}

int floatcomp(const void* a, const void* b)
{
    if(*(const float*)a < *(const float*)b)
        return -1;
    return *(const float*)a > *(const float*)b;
}

typedef struct afterSwap {
	float* mieiElementi;
	int hadToSwap;
} orderedAfterSwap;

orderedAfterSwap* swapMin(float* mieiElementi, float* ricevuti, int elementiXproc){
	int i,j,count;
	orderedAfterSwap *result;
	i=0; j=0; count=0;
	result = malloc(sizeof(orderedAfterSwap));
	result->mieiElementi = malloc(sizeof(float)*elementiXproc);
	result->hadToSwap = 0;
	while(count<elementiXproc){
		if(mieiElementi[i]<=ricevuti[j]){
			result->mieiElementi[count] = mieiElementi[i];
			i++;
		}
		else{
			result->mieiElementi[count] = ricevuti[j];
			result->hadToSwap++;
			j++;
		}
		count++;
	}
	return result;
}

orderedAfterSwap* swapMax(float* mieiElementi, float* ricevuti, int elementiXproc){
	int i,j,count;
	orderedAfterSwap *result;
	i=elementiXproc-1; j=elementiXproc-1; count=elementiXproc-1;
	result = malloc(sizeof(orderedAfterSwap));
	result->mieiElementi = malloc(sizeof(float)*elementiXproc);
	result->hadToSwap = 0;
	while(count>=0){
		if(mieiElementi[i]>=ricevuti[j]){
			result->mieiElementi[count] = mieiElementi[i];
			i--;
		}
		else{
			result->mieiElementi[count] = ricevuti[j];
			result->hadToSwap++;
			j--;
		}
		count--;
	}
	return result;
}

char* intToBinary(int k, int length){
	int i;
	char *result;
	result = malloc(length);
	for(i=length-1; i>=0; i--){
		if(k%2==0)
			result[i] = '0';
		else
			result[i] = '1';
		k/=2;
	}
	return result;
}

int binaryToInt(char* binary, int length){
	int i, k, result;
	result = 0;
	k = length-1;
	for(i=0; i<length; i++){
		if(binary[i]=='1')
			result += pow(2,k);
		k--;
	}
	return result;
}

int main(int argc, char* argv[]){
	int rank, size, n, i, j, elementiXproc, stage, length, next;
	orderedAfterSwap *m;
	char *binary;
	FILE *file;
	float *elementi, *mieiElementi, *result;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(argc<2) {
		printf("Numero argomenti non sufficiente: %d richiesto %d", argc-1, 1);
		MPI_Abort(MPI_COMM_WORLD, 0);
		return 1;
	}

	if(rank==0) {
		writeFile();
		file = fopen(argv[1],"rb");

		if(file==NULL) {
			printf("Non è stato possibile aprire il file: %s", argv[1]);
			MPI_Abort(MPI_COMM_WORLD, 0);
			return 1;
		}

		fread(&n, sizeof(int), 1, file);
		elementiXproc = n/size;
		mieiElementi = malloc(sizeof(float)*elementiXproc);
		elementi = malloc(sizeof(float)*elementiXproc);
		fread(mieiElementi, sizeof(float), elementiXproc, file);

		for(i=1; i<size; i++){
			MPI_Send (&elementiXproc, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			fread(elementi, sizeof(float), elementiXproc, file);
			MPI_Send (elementi, elementiXproc, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
		}
		fclose(file);
		result = malloc(sizeof(float)*n);
	}

	else {
		MPI_Recv (&elementiXproc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		mieiElementi = malloc(sizeof(float)*elementiXproc);
		MPI_Recv (mieiElementi, elementiXproc, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		elementi = malloc(sizeof(float)*elementiXproc);
	}

	qsort(mieiElementi, elementiXproc, sizeof(float), floatcomp);
	length = log(size)/log(2);
	binary = intToBinary(rank,length);
	for(stage=0; stage<length; stage++) {
		if(binary[stage]=='0'){
			binary[stage] = '1';
			next = binaryToInt(binary, length);
			binary[stage] = '0';
			MPI_Send (mieiElementi, elementiXproc, MPI_FLOAT, next, 0, MPI_COMM_WORLD);
			MPI_Recv (elementi, elementiXproc, MPI_FLOAT, next, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			m = swapMin(mieiElementi,elementi,elementiXproc);
			mieiElementi = m->mieiElementi;
		}
		else {
			binary[stage] = '0';
			next = binaryToInt(binary, length);
			binary[stage] = '1';
			MPI_Recv (elementi, elementiXproc, MPI_FLOAT, next, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Send (mieiElementi, elementiXproc, MPI_FLOAT, next, 0, MPI_COMM_WORLD);
			m = swapMax(mieiElementi,elementi,elementiXproc);
			mieiElementi = m->mieiElementi;
		}
	}

	MPI_Gather(mieiElementi, elementiXproc, MPI_FLOAT, result, elementiXproc, MPI_FLOAT, 0, MPI_COMM_WORLD);

	if(rank==0){
		printf("[ ");
		for(j=0; j<n; j++) {
			printf("%f ", result[j]);
		}
		printf("] \n");
		free(result);
	}

	free(m);
	free(binary);
	free(mieiElementi);
	free(elementi);

	MPI_Finalize();
	return 0;
}
