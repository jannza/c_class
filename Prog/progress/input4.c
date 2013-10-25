#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define BUF_SIZE 1024

main(int argc, const char *argv[]){


	char buffer[BUF_SIZE];
	size_t contentSize = 0; // includes NULL
	/* Preallocate space.  We could just allocate one char here, 
	but that wouldn't be efficient. */
	char *content = malloc(sizeof(char) * BUF_SIZE);
	if(content == NULL)
	{
	    perror("Failed to allocate content");
	    exit(1);
	}
	//content[0] = '\0'; // make null-terminated

	while(fread(buffer, 1, BUF_SIZE, stdin)){
		char *old = content;
		contentSize += strlen(buffer);
		content = realloc(content, contentSize);
		if(content == NULL){
			perror("Failed to reallocate space for content");
			free(old);
			exit(2);
	    }
		strcat(content, buffer);

		fprintf( stdout, buffer );
	}
	
	//size_t bytes = fread(buffer, 1, BUF_SIZE, stdin);
	

	/*while(fgets(buffer, BUF_SIZE, stdin)){
	    char *old = content;
	    contentSize += strlen(buffer);
	    content = realloc(content, contentSize);
	    if(content == NULL){
			perror("Failed to reallocate space for content");
			free(old);
			exit(2);
	    }
	    strcat(content, buffer);
	}

	if(ferror(stdin)){
	    free(content);
	    perror("Error reading from stdin.");
	    exit(3);
	}*/
	//fprintf( stdout, buffer );


	/*do something*/

	//fprintf( stdout, content );
	//printf("%d", contentSize );
	fflush(stdout);
	//free space
	free(content);
	return(EXIT_SUCCESS);

}

