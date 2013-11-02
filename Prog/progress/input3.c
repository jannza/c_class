#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define CHAR 256 

typedef struct BITFILE{
    
    unsigned char buffer;             /* intermediate store for bits */
    int counter;            /* how many bits are waiting */
    FILE *f;                      /* byte file */ 
} BITFILE;


   
typedef struct node {
	struct node *left, *right;
	char c;
	int freq;
} node;

int *counter(char *buf);
node** increase(node **trees, int size, node* add);
int getSmallest(node **trees, int size);
node** reduceOne(node **trees, int size, int skip);
void free_tree(node * tree);

BITFILE *bitOpen(FILE *f);        /*initialise bit input or output*/
int     getbit(BITFILE *file);     /*get one bit from bf->buffer */
void    putbits(int nbits, unsigned char word, BITFILE *bf);        /* put nbits lower bits of word */
void    bitClose(BITFILE *bf);        /*finish writing. Flush the last bits to bf->file*/
void getbit2();
void  int2bin(int i);

main(int argc, const char *argv[]){
	
	FILE *input;
	FILE *output;
	char *buffer;
	unsigned long file_len;
	//typedef int ratetable[CHAR];
	int i, j, k, not_zero, index;
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


	while(forest_size > 1){

	
		//printf("Forest size before loop %d\n", forest_size);
		index = getSmallest(forest2, forest_size);
		smallest1 = forest2[index];
		forest2= reduceOne(forest2, forest_size, index);
		forest_size--;
		index = getSmallest(forest2, forest_size);
		smallest2 = forest2[index];
		forest2= reduceOne(forest2, forest_size, index);
		forest_size--;	
		//printf("got 2 smallest\n");
		//printf("occurance rate %c with %d\n", smallest1->c, smallest1->freq);
		//printf("occurance rate %c with %d\n", smallest2->c, smallest2->freq);
		//combine them together

		new->left = smallest1;
		new->right = smallest2;
		new->c = 0;
		new->freq = (smallest1->freq + smallest2->freq);
		//printf("new element :%d\n", new->freq);
		forest2 = increase(forest2, forest_size, new);
		forest_size++;
		//printf("Forest size after loop %d\n", forest_size);
	
	}
	printf("Making tree complete\n");
	printf("tree info: %d\n", forest2[0]->c);
	

	//getbit2();
	/*input = fopen(argv[1], "rb");
	
	BITFILE *bf;
	bf = bitOpen(input);

	for (i = 0; i <8; i++){
		printf("got -> %d <- from file\n", getbit(bf));
	}
	bitClose(bf);
	fclose(input);

*/
	

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
		j++;
    }
	forest[j] = add; 
	
	/*for( i = 0; i < size +1; i++){
    	printf("array element %c --> freq %d\n",forest[i]->c, forest[i]->freq);
    }*/
	//printf("forest size after adding %d\n", (size+1));
	return forest;

}
int getSmallest(node **trees, int size){
	int i;
	int index = 0;
	node * chosen;
    for( i = 0; i < size; i++){
		if(trees[i]->freq < trees[index]->freq){
			index = i;		
		}

    }
	
	//chosen = trees[index];
	//trees[index] = NULL;
	return index;
}

node** reduceOne(node **trees, int size, int skip){
	int i, j = 0;
	int empty;
	node** forest = malloc(sizeof(node*) * (size -1));
	for( i = 0; i < size; i++){
		if(i != skip ){
			forest[j] = malloc(sizeof(node*));
			*forest[j] = *trees[i];
			j++;
			
		}
		//free_tree(trees[i]);
    }
	
	//printf("forest size after reduce %d\n", (size -1));
	return forest;
}

void free_tree(node * tree){
	if(tree)
    {
        free_tree(tree->left);
        free_tree(tree->right);
        free(tree);
    }
}

BITFILE *bitOpen(FILE *f){
	BITFILE *bf;
	bf = malloc(sizeof(BITFILE));
	if(!bf){
		fprintf(stderr, "Memory error!");
		exit(EXIT_FAILURE);		
	}
	bf->buffer=0;
	bf->counter=0;
	bf->f = f;
	return bf;

}

void    bitClose(BITFILE *bf){
	//fwrite (&(bf->buffer) , sizeof(unsigned int), 1, bf->f);
	free(bf);
}
int getbit(BITFILE *file) {
	int answer;
	if(file->counter == 0){
		file->buffer = fgetc(file->f);
		file->counter = 8;
	}
	//some bitmagic
	answer = ((file->buffer) >> 0) & 1;
	//answer = (file->buffer) & 1;
	//printf("buffer value %c", file->buffer);
	file->buffer = file->buffer >> 1;
	file->counter--;
	//printf("buffer value %c", file->buffer);
	return  answer;
}
void putbits(int nbits, unsigned char word, BITFILE *bf){
	int i;
	int current;
	for( i = 0; i < nbits; i++){
		//take first bit from given input		
		current = (word >> 0) & 1;
		//shift to right
		word = word >> 1;
		//write taken bit into next curently free slot(using OR)		
		bf->buffer = bf->buffer | (current << (bf->counter));
		bf->counter++;
		if(bf->counter == 8){
			fwrite(&(bf->buffer),1,sizeof(bf->buffer),bf->f);
			//fflush (bf->f);
			//putc(bf->buffer, bf->f);
			bf->counter = 0;
		}
	}
	//should add case when input in not even with 8	

}


void getbit2(){
	int i;
	char c = 255;
	int answer;
	int wanted = 4;
	int2bin(c);

	for( i = 0; i < 8; i++){
		printf("%d th bit -- %d\n", i, (c >> i) & 1);
	}
	//printf("buffer value %c\n", c);
	answer = (c >> wanted) & 1;
	

	//first = c & (1 << 0);

	c = c >> 1;
	int2bin(c);
	//printf("buffer value after %c\n", c);

}


void  int2bin(int i)
{
    size_t bits = sizeof(int) * 8;

    char * str = malloc(bits + 1);
    //if(!str) return NULL;
    str[bits] = 0;

    // type punning because signed shift is implementation-defined
    unsigned u = *(unsigned *)&i;
    for(; bits--; u >>= 1)
    	str[bits] = u & 1 ? '1' : '0';

	printf("binary representation: %s\n", str);
    //return str;
}
