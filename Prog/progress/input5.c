#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define CHAR 256 

   
typedef struct node {
	struct node *left, *right;
	char c;
	int freq;
} node;

int *counter(char *buf);
node** increase(node *trees[], int size, node* add);
node* get_smallest(node *trees[], int size);
void reduce_one(node *trees[], int size);


main(int argc, const char *argv[]){
	
	FILE *input;
	FILE *output;
	char *buffer;
	unsigned long file_len;
	//typedef int ratetable[CHAR];
	int i, j, k, not_zero;
	int help = 0;

	if(argc != 3){
		printf("Wrong number of arguments");
		exit(1);	
	}
	
	//Open file
	input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Unable to open file %s\n", argv[1]);
		return;
	}
	
	//Get file length
	fseek(input, 0, SEEK_END);
	file_len=ftell(input);
	fseek(input, 0, SEEK_SET);

	//Allocate memory
	buffer=(char *)malloc(file_len+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
        fclose(input);
		exit(EXIT_FAILURE);	
	}

	//Read file contents into buffer
	fread(buffer, file_len, 1, input);
	
	fclose(input);
	printf("Input reading complete!\n");
	//have read input, ready to do something
	int freq[CHAR] = {0};
	for (i = 0; i <file_len; i++){
		freq[(buffer[i])] = freq[(buffer[i])] +1;
	}
	not_zero = 0;
	for(i=0; i< CHAR; i++){
		//printf("%c --> %d\n",i, freq[i]);
		if(freq[i] > 0){
			not_zero++;		
		}
	}
	//printf("array size %d\n", k);
	//int* f[CHAR] = {0};
	//f = counter(buffer);
	
	printf("Frequency count complete\n");
	


	//node b = {0,0,'r', 5};
	node *forest[not_zero];
	node *b;
	j = 0;

	for (i = 0; i <CHAR; i++){
		if(freq[i] > 0){
			b = malloc(sizeof (node));
			b->left = 0;
			b->right = 0;
			b->c = i;
			b->freq = freq[i];
			forest[j] = b;
			//printf("latest array element %c freq %d\n",forest[j]->c, forest[j]->freq);
			j++;
		}
	}
		




	//printf("size of array: %d\n", sizeof(forest)/sizeof(forest[0]));
	//need to combine nodes
	//find with lowest occurance member
	int index = 0;
    for( i = 0; i < sizeof(forest)/sizeof(forest[0]); i++){
		if(forest[i]->freq < forest[index]->freq){
			index = i;		
		}

    }
	
	//printf("Fewest occurance rate %c with %d\n", forest[index]->c, forest[index]->freq);
	//remove element from array
	node *add = malloc(sizeof (node));
	add->left = 0;
	add->right = 0;
	//add->c = "b";
	add->freq = 5;
	

	printf("new element :%d\n", add->freq);

	//node **forest2;
	//forest2  = increase(forest, sizeof(forest)/sizeof(forest[0]), add);


	node *smallest = malloc(sizeof (node));
	smallest = get_smallest(forest, sizeof(forest)/sizeof(forest[0]));
	printf("Fewest occurance rate %c with %d\n", smallest->c, smallest->freq);
	reduce_one(forest, sizeof(forest)/sizeof(forest[0]));
	
	/*printf("new array size %d\n", sizeof(forest2)/sizeof(forest2[0]));
		
	for( i = 0; i < not_zero+1; i++){
    	printf("array element %c --> freq %d\n",forest2[i]->c, forest2[i]->freq);
    }*/

	/*for( j = 0; j < sizeof(forest)/sizeof(forest[0]); j++){
		printf("in forest %c\n", forest[j]->c);

    }*/

	output = fopen(argv[2], "wb");	
	if (!output)
	{
		printf("Some error\n");
		fprintf(stderr, "Unable to create file %s\n", argv[2]);
		exit(EXIT_FAILURE);	
	}
	fwrite(buffer, file_len, 1, output);
	fclose(output);	
	
	exit(EXIT_SUCCESS);

}

int *counter(char *buf){
	int i;
	int *freq_table[CHAR] = {0};
	for (i = 0; i <sizeof(buf); i++){
		freq_table[(buf[i])] = freq_table[(buf[i])] +1;
	}
	return *freq_table;
}

node** increase(node *trees[], int size, node* add){
	int i = 0;
	/*for( i = 0; i < size; i++){
    	printf("array element %c --> freq %d\n",trees[i]->c, trees[i]->freq);
    }*/
	int j = 0;
	node** forest = malloc(sizeof(node*) * (size+1));
	for (i=0; i<=size; i++)
    {
        forest[i] = malloc(sizeof(node*));
        *forest[i] = *trees[i];
    }
	forest[size+1] = add; 

	/*for( i = 0; i < size +1; i++){
    	printf("array element %c --> freq %d\n",forest[i]->c, forest[i]->freq);
    }*/
	return forest;

}
node* get_smallest(node *trees[], int size){
	int i;
	int index = 0;
	node * chosen;
    for( i = 0; i < size; i++){
		if(trees[i]->freq < trees[index]->freq){
			index = i;		
		}

    }
	
	chosen = trees[index];
	trees[index] = NULL;
	return chosen;
}

void reduce_one(node *trees[], int size){
	int i;
	int empty;
	for( i = 0; i < size +1; i++){
		
		if(trees[i] == NULL){
			printf("Found empty\n");
			empty = i;
		}else{
    		printf("array element %c --> freq %d\n",trees[i]->c, trees[i]->freq);
		}
    }
}


