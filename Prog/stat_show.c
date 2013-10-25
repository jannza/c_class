#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



int main(int argc, char ** argv) {
	struct stat *buf;
	long file_size;
	DIR *dir;
	struct dirent *ent;
	char *compare = "*";


	if(argc != 2){
		//display directory contents
		if ((dir = opendir(".")) == NULL) {
			printf("Couldnt open current directory\n");
       			exit(EXIT_FAILURE);	 
  		}
		
		printf("Didnt give arguments, printing current directory\n");
		while ((ent = readdir (dir)) != NULL) {				
			
			//printf("Beginning: %c", (ent->d_name)[0]);
   			printf ("%s\n", ent->d_name);
  		}


		

	closedir (dir);
	}
	if(access( argv[1], F_OK ) != 0){
		printf("No such file exists\n");
		exit(EXIT_FAILURE);
	}

	buf = malloc(sizeof(struct stat));
	stat(argv[1], buf);
	file_size = (long) buf->st_size;
	printf("Size of file: %lu bytes\n", file_size);
	printf("Permissions: %o\n", buf->st_mode & 0777); 
	printf("Great success\n");

	/*if((buf->st_mode & S_IRUSR)){
		printf("Owner can read\n");
	}else{
		printf("Owner cant read");
	}*/
	printf( (buf->st_mode & S_IRUSR) ? "Owner can read\n" : "Owner cant read\n");
 	printf( (buf->st_mode & S_IWUSR) ? "Owner can write\n" : "Owner cant write\n");
   	printf( (buf->st_mode & S_IXUSR) ? "Owner can execute\n" : "Owner cant execute\n");

	exit(EXIT_SUCCESS);
}

