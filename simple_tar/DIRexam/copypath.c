#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

void copy_file(char from[], char to[], char filename[])
{
	FILE * pFile_r, * pFile_w;
	long lSize;
	char * buffer;
	size_t result;
	char address_r[200], address_w[200];

	struct stat sfile;
	
	strcpy(address_r, from);
	strcat(address_r, "/");
	strcat(address_r, filename);
	strcpy(address_w, to);
	strcat(address_w, "/");
	strcat(address_w, filename);

	pFile_r = fopen(address_r, "rb");
	pFile_w = fopen(address_w, "wb");

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

	result = fread(buffer, 1, lSize, pFile_r);
	if (result != lSize) {
		fputs("Reading error", stderr);
		exit(3);
	}
    fwrite(buffer, sizeof(char), lSize, pFile_w);
	stat(address_r, &sfile);
	if (chmod(address_w, sfile.st_mode) == -1)
		fprintf(stderr, "chmod is failed..\n");

	fclose(pFile_r);
    fclose(pFile_w);
	free(buffer);
}
void copy_directory(char from[], char to[], char filename[])
{
	char from_address[200];
	char to_address[200];
	char new_file[100];

	DIR* dp_to = opendir(to);
	strcpy(to_address, to);
	strcat(to_address, "/");
	strcat(to_address, filename);
	
	if (mkdir(to_address, 0775) == -1) {
		fprintf(stderr, "Fail to make directory: %s\n", to_address);
		return;
	}
	(void) closedir(dp_to);


	strcpy(from_address, from);
	strcat(from_address, "/");
	strcat(from_address, filename);

	DIR* dp_from = opendir(from_address);

	struct dirent* ep;
	if (dp_from != NULL) {
		while (ep = readdir(dp_from)){
			if ((strcmp(".", ep->d_name) == 0) || (strcmp("..", ep->d_name) == 0))
				continue;
			// puts(ep->d_name);
			if (ep->d_type == 4){
				strcpy(new_file, ep->d_name);
				copy_directory(from_address, to_address, new_file);
			} else {
				copy_file(from_address, to_address, ep->d_name);
			}
		}
		(void) closedir(dp_from);
	} else {
		perror("Fail to access directory: %s\n");
	}
}

void copy (char from[], char to[])
{
	int i;
	char target_name[100];
	for (i = strlen(from); i >= 0; i--) {
		if (from[i] == '/') {
			strcpy(target_name, &from[i+1]);
			from[i] = '\0';
			break;
		}
	}

	DIR* target_from = opendir(from);
	DIR* target_to = opendir(to);
	if (target_from == NULL)
		perror("Couldn't open the target directory");
	if (target_to == NULL)
		perror("Couldn't open the destination directory");

	struct dirent* ep;
	while (ep = readdir(target_from)) {
		if (strcmp(target_name, ep->d_name) == 0) {
			if (ep->d_type == 4) {
				copy_directory(from, to, target_name);
				break;
			}
			else {
				copy_file(from, to, target_name);
				break;
			}
		}
	}
	(void) closedir(target_from);
	(void) closedir(target_to);

}



int
main (int argc, char * argv[])
{
	// target은 file 혹은 디렉토리의 path 와 이름
	// dest는 path
	char from[200];
	char to[200];

	if (argc < 3) {
		fprintf(stderr, "Input command is short..\n");
		fprintf(stderr, "=> copy <target> <dest>\n");
		return 0;
	}
	
	strcpy(from, argv[1]);
	strcpy(to, argv[2]);

	copy(from, to);

	return 0;
}
