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
node** increase(node **trees, int size, node* add);
node* get_smallest(node **trees, int size);
node** reduce_one(node **trees, int size);


main(int argc, const char *argv[]){
	
	FILE *input;
	FILE *output;
	char *buffer;
	unsigned long file_len;
	//typedef int ratetable[CHAR];
	int i, j, k, not_zero;
	int forest_size;
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
	forest_size = not_zero;
	//printf("array size %d\n", k);
	//int* f[CHAR] = {0};
	//f = counter(buffer);
	
	printf("Frequency count complete\n");


	node **forest2 = malloc(sizeof(node*) * (not_zero));
	//node *forest[not_zero];
	node *b;
	j = 0;

	for (i = 0; i <CHAR; i++){
		if(freq[i] > 0){
			b = malloc(sizeof (node));
			b->left = 0;
			b->right = 0;
			b->c = i;
			b->freq = freq[i];
			forest2[j] = b;
			//printf("latest array element %c freq %d\n",forest2[j]->c, forest2[j]->freq);
			j++;
		}
	}
	node *smallest1 = malloc(sizeof (node*));
	node *smallest2 = malloc(sizeof (node*));
	node *new = malloc(sizeof (node*));
	
	printf("Forest size before loop %d\n", forest_size);
	smallest1 = get_smallest(forest2, forest_size);
	forest2= reduce_one(forest2, forest_size);
	forest_size--;
	smallest2 = get_smallest(forest2, forest_size);
	forest2= reduce_one(forest2, forest_size);
	forest_size--;
	printf("got 2 smallest\n");
	//printf("occurance rate %c with %d\n", smallest1->c, smallest1->freq);
	//printf("occurance rate %c with %d\n", smallest2->c, smallest2->freq);
	//combine them together

	new->left = smallest1;
	new->right = smallest2;
	new->c = 0;
	new->freq = (smallest1->freq + smallest2->freq);
	//node *new = new tree{smallest1, smallest2, 0, (smallest1->freq + smallest2->freq)};
	printf("new element :%d\n", new->freq);
	forest2 = increase(forest2, forest_size, new);
	forest_size++;
	printf("Forest size after loop %d\n", forest_size);






	printf("Forest size before loop %d\n", forest_size);
	smallest1 = get_smallest(forest2, forest_size);
	forest2= reduce_one(forest2, forest_size);
	forest_size--;
	smallest2 = get_smallest(forest2, forest_size);
	forest2= reduce_one(forest2, forest_size);
	forest_size--;
	printf("got 2 smallest\n");
	//printf("occurance rate %c with %d\n", smallest1->c, smallest1->freq);
	//printf("occurance rate %c with %d\n", smallest2->c, smallest2->freq);
	//combine them together
	new->left = smallest1;
	new->right = smallest2;
	new->c = 0;
	new->freq = (smallest1->freq + smallest2->freq);
	//node *new = new tree{smallest1, smallest2, 0, (smallest1->freq + smallest2->freq)};
	printf("new element :%d\n", new->freq);
	forest2 = increase(forest2, forest_size, new);
	forest_size++;
	printf("Forest size after loop %d\n", forest_size);


	//printf("size of array: %d\n", sizeof(forest)/sizeof(forest[0]));
	//need to combine nodes
	//find with lowest occurance member
	/*int index = 0;
    for( i = 0; i < sizeof(forest)/sizeof(forest[0]); i++){
		if(forest[i]->freq < forest[index]->freq){
			index = i;		
		}

    }*/
	
	//printf("Fewest occurance rate %c with %d\n", forest[index]->c, forest[index]->freq);
	//remove element from array
	

	//printf("new element :%d\n", add->freq);

		

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

node** increase(node **trees, int size, node* add){
	int i = 0;
	/*for( i = 0; i < size; i++){
    	printf("array element %c --> freq %d\n",trees[i]->c, trees[i]->freq);
    }*/
	int j = 0;
	node** forest = malloc(sizeof(node*) * (size+1));
	for (i=0; i<size; i++){
        forest[i] = malloc(sizeof(node*));
        *forest[i] = *trees[i];
    }
	forest[size+1] = add; 

	/*for( i = 0; i < size +1; i++){
    	printf("array element %c --> freq %d\n",forest[i]->c, forest[i]->freq);
    }*/
	return forest;

}
node* get_smallest(node **trees, int size){
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

node** reduce_one(node **trees, int size){
	int i, j = 0;
	int empty;
	node** forest = malloc(sizeof(node*) * (size));
	for( i = 0; i < size; i++){
		
		if(trees[i] == NULL){
			//printf("Found empty\n");
			empty = i;
		}
    }
	if(empty){
		for (i=0; i<size; i++){
			if(i != empty){
				forest[j] = malloc(sizeof(node*));
        		*forest[j] = *trees[i];
				j++;
			}        	
		}
	}

	return forest;
}


