#include <stdio.h>
#include <stdlib.h>

#define INDEX(i,j,rowlen) ((i)*(rowlen)+j)

int main2(int argc, char* argv[]) {
	float **A;
	float *x;
	float n, temp, res, test;
	unsigned int i, j, righe, colonne, col, k;
	FILE* file = fopen("C:\\Users\\Daniele\\Documents\\Visual Studio 2012\\Projects\\Test\\Debug\\Prova.txt","r");
	
	if(file==NULL)
		return 1;
	fscanf(file,"%u", &righe);
	fscanf(file,"%u", &colonne);
	if(righe!=colonne)
		return 1;
	x = malloc(righe * sizeof(float));
	A = malloc(righe * sizeof(float*));
	for(i = 0; i < colonne; i++)
		fscanf(file,"%f",&x[i]);
	res = 0.0;
	for(i=0; i<righe; i++) {
		col = colonne - i;
		temp = 0.0;
		A[i] = malloc(col * sizeof(float));
		for(j=0; j<colonne; j++) {
			if(j>i) {
				k = j-i;
				fscanf(file,"%f",&A[i][k]);
				temp += A[i][k] * x[j];
			}
			else if(j==i) {
				k = j-i;
				fscanf(file,"%f",&n);
				if(n != 0.0) {
					return 1;
					exit;
				}
				A[i][k] = n;
				temp += n * x[j];
			}
			else {
				fscanf(file,"%f",&n);
				k = i-j;
				if(n != -A[j][k]) {
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

int main1(int argc, char** argv) {
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