#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

typedef enum {
	archive,
	list,
	extract,
	N_md
} Modetype;

char* md_str[3] = {
	"archive",
	"list",
	"extract"
};

Modetype get_md_type (char* mode)
{
	int i;
	for (i = 0; i < N_md; i++) {
		if (strcmp(mode, md_str[i]) == 0)
			return i;
	}
	return N_md;
}

void write_archive_file (FILE* pFile_w, char curr_dir[], char target_file[], int parsing_point)
{
	FILE* pFile_r ;
	char filepath_on_tar[400] = "./" ;
	char filepath[400] ;
	char* buffer ;
	size_t result ;
	struct stat sfile ;
	unsigned int filepath_len ;
	unsigned int size_of_file ;
	int kind ;

	strcpy(filepath, curr_dir) ;
	if (filepath[strlen(filepath)-1] != '/')
		strcat(filepath, "/") ;
	strcat(filepath, target_file) ;
	strcat(filepath_on_tar, &filepath[parsing_point]) ;

	if (stat(filepath, &sfile) == -1) {
		fprintf(stderr, "Error occurred\n") ;
	}

	printf("filepath: %s\n", filepath_on_tar) ;

	kind = sfile.st_mode & S_IFMT ;
	filepath_len = (unsigned int)strlen(filepath_on_tar) ;
	size_of_file = (kind != S_IFDIR)? (unsigned int)sfile.st_size : 0 ;

	// write the head.
	fwrite(&filepath_len, sizeof(unsigned int), 1, pFile_w) ;
	fwrite(filepath_on_tar, sizeof(char), filepath_len, pFile_w) ;
	fwrite(&(sfile.st_mode), sizeof(mode_t), 1, pFile_w) ;
	fwrite(&size_of_file, sizeof(unsigned int), 1, pFile_w) ;

	// write the body.
	pFile_r = fopen(filepath, "rb") ;
	if (pFile_r == NULL) {
		fputs("File error", stderr) ;
		goto _error_file_read_ ;
	}

	buffer = (char*) malloc(sizeof(char)*size_of_file) ;
	if (buffer == NULL) {
		fputs("File error", stderr) ;
		goto _error_file_read_ ;
	}
	result = fread(buffer, sizeof(char), size_of_file, pFile_r) ;
	fwrite(buffer, sizeof(char), size_of_file, pFile_w) ;
	free(buffer) ;

	if (kind == S_IFDIR) {
		DIR* dp ;
		struct dirent* ep ;

		dp = opendir(filepath) ;
		if (dp != NULL) {
			while (ep = readdir(dp)){
				if ((strcmp(".", ep->d_name) == 0) || (strcmp("..", ep->d_name) == 0))
					continue ;
				write_archive_file(pFile_w, filepath, ep->d_name, parsing_point) ;
			}
			(void) closedir(dp) ;
		}
		else 
			perror("Couldn't open the directory") ;
	}


_error_file_read_:
	if (pFile_r != NULL)
		fclose(pFile_r) ;
}

void archive_mode(char ar_filename[], char target_path[])
{
	FILE* pFile_w ;
	struct stat sfile ;
	char curr_dir[400] ;
	char filename[100] ;
	int i ; 

	for (i = strlen(target_path)-1 ; i >= 0 ; i--) {
		if (target_path[i-1] == '/') {
			strcpy(filename, &target_path[i]) ;
			strncpy(curr_dir, target_path, i) ;
			curr_dir[i] = '\0' ;
			break ;
		}
	}

	pFile_w = fopen(ar_filename, "wb") ; 
	if (pFile_w == NULL) {
		fprintf(stderr, "Error occurred\n") ;
		return ;
	}
	

	if (stat(target_path, &sfile) == -1) {
		fprintf(stderr, "Error occurred\n") ;
		goto _error_stat_call_ ;
	}
	
	int kind = sfile.st_mode & S_IFMT ;
	switch (kind)
	{
		case S_IFDIR:
			write_archive_file(pFile_w, curr_dir, filename, i) ;
			break ;
		
		default:
			fprintf(stderr, "This is not a directory\n") ;
			break ;
	}
	
_error_stat_call_:
	fclose(pFile_w) ;
}

void list_mode(char ar_filename[]) 
{
	FILE* pFile_r ;
	long lSize ;
	char * filename = NULL ;
	unsigned int n ;
	unsigned int m ;
	mode_t file_mode ;
	size_t result ;
	int i, count ;

	pFile_r = fopen(ar_filename, "rb") ;
	if (pFile_r == NULL) {
		fputs("File error", stderr) ;
		exit(1) ;
	}

	while (1)
	{
		result = fread(&n, sizeof(unsigned int), 1, pFile_r) ;
		if (result != 1) {
			if (feof(pFile_r))
				break ;
			fputs("Reading error", stderr) ;
			goto _error_read_file_ ;
		}
		filename = (char*)malloc(sizeof(char) * (n+1)) ;
		result = fread(filename, sizeof(char), n, pFile_r) ;
		if (result != n) {
			fputs("Reading error", stderr) ;
			goto _error_read_file_ ;
		}
		filename[n] = '\0' ;
		result = fread(&file_mode, sizeof(mode_t), 1, pFile_r) ;
		if (result != 1) {
			fputs("Reading error", stderr) ;
			goto _error_read_file_ ;
		}
		result = fread(&m, sizeof(unsigned int), 1, pFile_r) ;
		if (result != 1) {
			fputs("Reading error", stderr) ;
			goto _error_read_file_ ;
		}
		
		if (fseek(pFile_r, m, SEEK_CUR))
			fputs("Reading seek error", stderr) ;

		count = -1 ;
		for (i = 0 ; i < strlen(filename) ; i++) {
			if (filename[i] == '/')
				count++ ;
		}
		for (i = strlen(filename)-1 ; i >= 0 ; i--) {
			if (filename[i-1] == '/') {
				while (count--)
					putchar('\t') ;
				if ((file_mode & S_IFMT) == S_IFDIR)
					printf("|___%s(D)\n", &filename[i]) ;
				else
					printf("|___%s\n", &filename[i]) ;
				break;
			}
		}
		// printf("filename: %s\n", filename) ;

		free(filename) ;
	}

_error_read_file_:
	fclose(pFile_r) ;
}

void extract_file (FILE* pFile_r, char filepath[], int file_size, int file_mode)
{
	FILE* pFile_w ;
	char* buffer ;
	size_t result ;

	switch (file_mode & S_IFMT)
	{
		case S_IFDIR:
			if (mkdir(filepath, 0775) == -1) {
				fprintf(stderr, "Fail to make directory: %s\n", filepath) ;
				return ;
			}
			break ;
		
		default:
			printf("%s\n", filepath) ;
			printf("sizse:%d\n", file_size) ;
			pFile_w = fopen(filepath, "wb") ;
			if (pFile_w == NULL) {
				fputs("File error", stderr) ;
			}
			buffer = (char*) malloc(sizeof(char)*file_size) ;
			if (buffer == NULL) {
				fputs("Allocation error", stderr) ;
				break ;
			}
			result = fread(buffer, 1, file_size, pFile_r) ;
		
			if (result != file_size) {
				fputs("Reading error", stderr) ;
				free(buffer) ;
				break ;
			}
			fwrite(buffer, sizeof(char), file_size, pFile_w) ;
			if (chmod(filepath, file_mode) == -1)
				fprintf(stderr, "chmod is failed..\n") ;

			fclose(pFile_w) ;
			free(buffer) ;
			break ;
	}
}

void extract_mode(char ar_filename[]) 
{
	FILE* pFile_r ;
	long lSize ;
	char * filename = NULL ;
	unsigned int n ;
	unsigned int m ;
	mode_t file_mode ;
	size_t result ;

	pFile_r = fopen(ar_filename, "rb") ;
	if (pFile_r == NULL) {
		fputs("File error", stderr) ;
		goto _error_file_open_ ;
	}
	while (1)
	{
		result = fread(&n, sizeof(unsigned int), 1, pFile_r) ;
		if (result != 1) {
			if (feof(pFile_r))
				break ;
			fputs("Reading error", stderr) ;
			goto _error_file_open_ ;
		}
		filename = (char*)malloc(sizeof(char) * (n+1)) ;
		result = fread(filename, sizeof(char), n, pFile_r) ;
		if (result != n) {
			fputs("Reading error", stderr) ;
			goto _error_file_open_ ;
		}
		filename[n] = '\0' ;
		result = fread(&file_mode, sizeof(mode_t), 1, pFile_r) ;
		if (result != 1) {
			fputs("Reading error", stderr) ;
			goto _error_file_open_ ;
		}
		result = fread(&m, sizeof(unsigned int), 1, pFile_r) ;
		if (result != 1) {
			fputs("Reading error", stderr) ;
			goto _error_file_open_ ;
		}
		

		int kind = file_mode & S_IFMT ;
		extract_file(pFile_r, filename, m, file_mode) ;

		
		free(filename) ;
	}

_error_file_open_:
	if (pFile_r == NULL)
		fclose(pFile_r) ;
}


int main ()
{
	char star[16], mode[16] ;
	char archive_filename[200] ;
	char target_path[200] ;
	char temp[200] ;

	scanf("%s %s", star, mode);
	if (strcmp(star, "star") != 0) {
		fprintf(stderr, "wrong command\n") ;
		return 0 ;
	}
	switch (get_md_type(mode))
	{
		case archive:
			scanf("%s", archive_filename) ;
			scanf("%s", target_path) ;
			if (target_path[0] != '.') {
				strcpy(temp, "./") ;
				strcat(temp, target_path) ;
				strcpy(target_path, temp) ;
			}
			printf("archive\n");
			archive_mode(archive_filename, target_path) ;
			break ;
		case list:
			scanf("%s", archive_filename) ;
			list_mode(archive_filename) ;
			break ;
		case extract:
			scanf("%s", archive_filename) ;
			extract_mode(archive_filename) ;
			break;
		case N_md:
			fprintf(stderr, "wrong archive mode\n") ;
			break;
	}

	return 0;
}
