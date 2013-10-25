#include <stdio.h>
main(){

	char buffer[100];
	int size;
	FILE *fp;
	fp = fopen("./input.txt", "r");
	if(fp == NULL){
		printf("Error opening file\n");
		return 1;
	}
	size = fread(buffer,1,100,fp);
	fwrite(buffer, 1,size,stdout);
	//printf("%s\n", buffer);
	fclose(fp);
	return 0;
}
