#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define BUF_SIZE 1024

main(int argc, const char *argv[]){
	
	FILE *in;
	FILE *out;
	char buffer[BUF_SIZE];
	size_t contentSize = 1; // includes NULL
	/* Preallocate space.  We could just allocate one char here, 
	but that wouldn't be efficient. */
	char *content = malloc(sizeof(char) * BUF_SIZE);
	if(content == NULL)
	{
	    perror("Failed to allocate content");
	    exit(1);
	}
	content[0] = '\0'; // make null-terminated



	if(argc != 3){
		printf("Wrong number of arguments");
		exit(1);	
	}


	in = fopen(argv[1],"rb"); // read mode
	while(fgets(buffer, BUF_SIZE, in)){
	    char *old = content;
	    contentSize += strlen(buffer);
	    content = realloc(content, contentSize);
	    if(content == NULL)
	    {
		perror("Failed to reallocate space for content");
		free(old);
		exit(2);
	    }
	    strcat(content, buffer);
	}
	out = fopen(argv[2],"wb"); // write mode
	fprintf(out, content);

	//printf("%s", content);
	fclose(in);
	fclose(out);
	return(EXIT_SUCCESS);

}

