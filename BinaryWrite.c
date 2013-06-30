#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>


int mainWrite(int argc, char **argv){

	FILE *f;

	if(argc<2) {
		printf("Numero argomenti non sufficiente: %d richiesto %d\n", argc-1, 1);
		return 1;
	}

	int a, b, c, d;
	a=2; b=2; c=2; d=2;
	float w1[2]={1, 2};
	float w2[2]={3, 1};
	float v1[2]={0, 3};
	float v2[2]={2, 1};


	f = fopen(argv[1], "wb");
	fwrite(&a, sizeof(int), 1, f);
	fwrite(&b, sizeof(int), 1, f);
	fwrite(&c, sizeof(int), 1, f);
	fwrite(&d, sizeof(int), 1, f);

	fwrite(w1, sizeof(float), 2, f);
	fwrite(w2, sizeof(float), 2, f);
	fwrite(v1, sizeof(float), 2, f);
	fwrite(v2, sizeof(float), 2, f);


	fclose(f);

	return 0;
}
