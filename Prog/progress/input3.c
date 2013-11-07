#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define CHAR 256
#define FAKECHAR 257 

typedef struct BITFILE{
    
    unsigned char buffer;             /* intermediate store for bits */
    int counter;            /* how many bits are waiting */
    FILE *f;                      /* byte file */ 
} BITFILE;


   
typedef struct node {
	struct node *left, *right;
	int c;
	int freq;
} node;

int *counter(char *buf);
node** increase(node **trees, int size, node* add);
int getSmallest(node **trees, int size);
node** reduceOne(node **trees, int size, int skip);
void substitutes(node * tree);
void traverse_tree(node * tree, char* path, char** table);
char translate(char * fake_bin);
void encode(char *buffer, char** table, BITFILE *bf);
int decode_one(BITFILE *file, node * tree);
void printfbits(int nbits, unsigned char word);
int readbits(BITFILE *file);
void decode(FILE *target, node * tree, BITFILE *source);

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
	if (!input){
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

	//make frequency table
	int freq[FAKECHAR] = {0};
	for (i = 0; i <file_len; i++){
		freq[(buffer[i])] = freq[(buffer[i])] +1;
	}
	freq[FAKECHAR -1] = 1;

	//count not zero characters in freq table
	not_zero = 0;
	for(i=0; i< FAKECHAR; i++){

		if(freq[i] > 0){
			not_zero++;		
		}
	}
	forest_size = not_zero;	
	printf("Frequency count complete\n");

	//create nodes for every nonzero charachter
	node **forest2 = malloc(sizeof(node*) * (not_zero));
	//node *forest[not_zero];
	node *b;
	j = 0;

	for (i = 0; i <FAKECHAR; i++){
		if(freq[i] > 0){
			b = malloc(sizeof (node));
			b->left = 0;
			b->right = 0;
			b->c = i;
			b->freq = freq[i];
			forest2[j] = b;
			j++;
		}
	}
	node *smallest1 = malloc(sizeof (node*));
	node *smallest2 = malloc(sizeof (node*));
	node *new = malloc(sizeof (node*));	
	//printf("Forest size before loop %d\n", forest_size);
	//start building tree from created nodes
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
	printf("tree info: %d\n", forest2[0]->freq);
	
	//make table for translation
	static char *table[FAKECHAR];
	traverse_tree(forest2[0], "", table);
	/*for (i = 0; i <FAKECHAR; i++){
		if(table[i]){
			printf("%c translates to %s\n", i, table[i]);
		}
	}*/


	printf("bitcode for my eof char %s\n", table[FAKECHAR -1]);


	//printf("translated charbin to realbin -- %c\n", *translate("01000100"));
	
	

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

	
	if (!output){
		printf("Some error\n");
		fprintf(stderr, "Unable to create file %s\n", argv[2]);
		exit(EXIT_FAILURE);	
	}

	BITFILE *bfile;
	bfile = bitOpen(output);






	
	encode(buffer, table, bfile);
	bitClose(bfile);

	//fwrite(buffer, file_len, 1, output);


	fclose(output);
	output = fopen(argv[2], "rb");

	bfile = bitOpen(output);
	decode(stdin,forest2[0], bfile);


	//readbits(bfile);	
	
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
	int j = 0;
	node** forest = malloc(sizeof(node*) * (size+1));
	for (i=0; i<size; i++){
        forest[i] = malloc(sizeof(node*));
        *forest[i] = *trees[i];
		j++;
    }
	forest[j] = add; 	
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

BITFILE *bitOpen(FILE *f){
	BITFILE *bf;
	bf = calloc(1, sizeof(BITFILE));
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
	//everything needed is written
	//but buffer may still contain something
	//add 0's to end of fill buffer and force write
	//printf("buffer size during closing %d\n", (bf->counter));
	putbits(8 - (bf->counter), '0', bf);
	//printf("buffer size after trying %d\n", (bf->counter));


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
void printfbits(int nbits, unsigned char word){
	int i;
	int current;
	printf("printing  %d bits from char\n", nbits);
	for( i = 0; i < nbits; i++){
		//take first bit from given input		
		current = (word >> 0) & 1;
		printf("%d", current);
		//shift to right
		word = word >> 1;
		
	}
	printf("\n");

}
//this is buggy, now better
void putbits(int nbits, unsigned char word, BITFILE *bf){
	int i;
	int current;
	//printf("sending these %d bits to buffer\n", nbits);
	for( i = 0; i < nbits; i++){
		//take first bit from given input		
		current = (word >> 0) & 1;
		//printf("adding %d\n", current);
		//shift to right
		word = word >> 1;
		//write taken bit into next curently free slot(using OR)
		//damn you mind, why did we start bits from right		
		//bf->buffer = bf->buffer | (current << (bf->counter));
		//this is shit, now maybe better



		bf->buffer = bf->buffer | (current <<(bf->counter));
		//printfbits(8,bf->buffer);
		bf->counter++;
		if(bf->counter == 8){
			fwrite(&(bf->buffer),1,sizeof(bf->buffer),bf->f);
			bf->buffer =0;
			bf->counter = 0;
		}
	}
	//should add case when input in not even with 8	

}



void traverse_tree(node * tree, char* path, char** table){
	int i;
	char* new;
	if(tree->left ==0 && tree->right == 0){
		table[tree->c] = path;		
	}
	if(tree->left != 0){
		new = malloc(strlen(path) + 2);
		new = strcpy(new, path);
		traverse_tree(tree->left, strcat(new, "0"), table);
	}
	if(tree->right != 0){
		new = malloc(strlen(path) + 2);
		new = strcpy(new, path);
		traverse_tree(tree->right, strcat(new, "1"), table);
	}
}


//now working OK
char translate(char * fake_bin){
	int i, intified, j;
	int size = strlen(fake_bin);
	//printf("%s\n", fake_bin);
	unsigned char real_bin =0;
	//unsigned char *real2 = calloc(1, sizeof(unsigned char));
	//printf("after allocation\n");
	//int2bin(real_bin);
	//real beginning in on the other end
	
	for( i = 0; i < size ; i++){
		intified = fake_bin[i] - '0';
		//printf("\nnext value %d\n", intified);
		//printf("%d\n", (intified<< (size- (i+1))) );
		//*real2 = *real2 | (intified >> i) ;
		//real_bin = real_bin | (intified<< (size- (i+1))) ;
		//real_bin = real_bin | (intified>>i);

		real_bin = real_bin | (intified << i);
	}
	//printf("end of for\n");
	return real_bin;
}
//this should work too
void encode(char *buffer, char** table, BITFILE *bf){
	int total = 0;
	int i, len;
	char* fake;
	unsigned char real;
	int size = strlen(buffer);
	for (i = 0; i <size; i++){
		fake = table[buffer[i]];
		len = strlen(fake);
		total = total + len;
		real = translate(fake);
		//printf("adding %s to file\n", fake);
		putbits(len, real, bf);
	}
	//at the end add my eof char to end of file
	fake = table[FAKECHAR-1];
	len = strlen(fake);
	total = total + len;
	real = translate(fake);
	//printf("adding %s eof to file\n", fake);
	putbits(len, real, bf);
	//printf("in total wrote %d bits\n", total);
}
int readbits(BITFILE *file){
	int bit, i;
	for( i = 0; i < 8; i++){
		bit = getbit(file);
		printf("%d", bit);
	}

}
//decode until
void decode(FILE *target, node * tree, BITFILE *source){
	int c;
	while((c = decode_one(source, tree)) != (FAKECHAR-1)){
		printf("%d\n", c);
		//fwrite(c,1,sizeof(char),target);
	}
}



int decode_one(BITFILE *file, node * tree){
	int bit;
	int answer;
	while(tree->left || tree->right){
		bit = getbit(file);
		//printf("%d", bit);
		if(bit){
			tree = tree->right;
		}else{
			tree = tree->left;
		}
	}
	if(!tree){
		printf("Translation error\n");
		fprintf(stderr, "Decoding tree error\n");
		exit(EXIT_FAILURE);	
	}
	answer = tree->c;
	return answer;
}



//debug functions, not really needed

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
    size_t bits = sizeof(char) * 8;

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
