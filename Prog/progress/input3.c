#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>


main(int argc, const char *argv[]){
	
	FILE *input;
	FILE *output;
	char *buffer;
	unsigned long fileLen;

	if(argc != 3){
		printf("Wrong number of arguments");
		exit(1);	
	}
	
	//Open file
	input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Unable to open file %s", argv[1]);
		return;
	}
	
	//Get file length
	fseek(input, 0, SEEK_END);
	fileLen=ftell(input);
	fseek(input, 0, SEEK_SET);

	//Allocate memory
	buffer=(char *)malloc(fileLen+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
        fclose(input);
		return;
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, input);
	
	fclose(input);	
	//printf("Input reading complete!\n");
	output = fopen(argv[2], "wb");
	
	fwrite(buffer, fileLen, 1, output);
	fclose(output);	

	return(EXIT_SUCCESS);

}

