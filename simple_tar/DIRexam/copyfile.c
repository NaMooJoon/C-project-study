#include <stdio.h>
#include <stdlib.h>

int main ()
{
	FILE * pFile_r, * pFile_w;
	long lSize;
	char * buffer;
	size_t result;

	pFile_r = fopen("copyfile.c", "rb");
	pFile_w = fopen("./to/copyfile.c", "wb");
	if (pFile_r == NULL || pFile_w == NULL) {
		fputs("File error", stderr);
		exit(1);
	}

	fseek(pFile_r, 0, SEEK_END);
	lSize = ftell(pFile_r);
	rewind(pFile_r);

	buffer = (char*) malloc(sizeof(char)*lSize);
	if (buffer == NULL) {
		fputs("Reading error", stderr);
		exit(2);
	}

	result = fread (buffer, 1, lSize, pFile_r);
    fwrite(buffer, sizeof(char), lSize, pFile_w);
	if (result != lSize) {
		fputs("Reading error", stderr);
		exit(3);
	}
    


	fclose(pFile_r);
    fclose(pFile_w);

	free(buffer);

	return 0;
}
