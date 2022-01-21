#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

void print_directory (DIR* dp, int n, char path[])
{
	char temp[200];
	// printf("::%p\n", dp);
	struct dirent* ep;
	if (dp != NULL) {
		while (ep = readdir(dp)){
			if ((strcmp(".", ep->d_name) == 0) || (strcmp("..", ep->d_name) == 0))
				continue;
			for (int i = 1 ; i < n; i++)
				printf("\t");
			printf("|____");
			puts(ep->d_name);
			if (ep->d_type == 4){
				strcpy(temp, path);
				strcat(temp, "/");
				strcat(temp, ep->d_name);
				DIR* new_dp = opendir(temp);
				print_directory(new_dp, n+1, temp);
			}
		}
		(void) closedir(dp);
	} else {
		perror("Couldn't open the directory");
	}
}

int
main (void)
{
	DIR* dp;
	char path[200] = "./";
	dp = opendir("./");
	struct dirent* ep;

	print_directory(dp, 1, path);
	return 0;
}
