#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h> 

#define CHAR 256
#define FAKECHAR 257 

typedef struct BITFILE {

	unsigned char buffer; /* intermediate store for bits */
	int counter; /* how many bits are waiting */
	FILE *f; /* byte file */
} BITFILE;

typedef struct node {
	struct node *left, *right;
	int c;
	unsigned int freq;
} node;

node** increase(node **trees, int size, node* add);
int getSmallest(node **trees, int size);
node** reduceOne(node **trees, int size, int skip);
//void traverse_tree(node * tree, char* path, char** table);
char** traverse_tree(node * tree, char* path, char** table);
int translate(char * fake_bin);
void encode(char *buffer, long size, char** table, BITFILE *bf);
unsigned int decode_one(BITFILE *file, node * tree);
void printfbits(int nbits, unsigned int word);
void readbits(BITFILE *file, int size);
void decode(FILE *target, node * tree, BITFILE *source);
node* make_tree(char *buffer, int file_len);
void put_tree(node* root, BITFILE *dest);
node* get_tree(BITFILE *source);
void encode_file(const char* in, const char* out);
void decode_file(const char* in, const char* out);
void traverse(node * tree);
long estimate_count(node * tree, char** table, long estimation);

BITFILE *bitOpen(FILE *f); /*initialise bit input or output*/
unsigned int getbit(BITFILE *file); /*get one bit from bf->buffer */
void putbits(int nbits, unsigned int word, BITFILE *bf); /* put nbits lower bits of word */
void bitClose(BITFILE *bf); /*finish writing. Flush the last bits to bf->file*/
void int2bin(int i);

int main(int argc, const char *argv[]) {
	if(argc == 1){
		printf("Wrong number of arguments\n");
		exit(EXIT_FAILURE);
	}
	if (!strcmp(argv[1], "-d")) {
		if(argc == 1){
			printf("Wrong number of arguments\n");
			exit(EXIT_FAILURE);
		}
		//printf("you wanted to unpack\n");
		if (argc != 4) {
			printf("Wrong number of arguments\n");
			exit(EXIT_FAILURE);
		}
		decode_file(argv[2], argv[3]);
		//printf("file decoding complete!\n");

	} else {
		if (argc != 3) {
			printf("Wrong number of arguments\n");
			exit(EXIT_FAILURE);
		}
		encode_file(argv[1], argv[2]);
		//printf("file encoding complete!\n");
	}

	exit(EXIT_SUCCESS);

}


/*Adds one node to array
 *@param **trees array of nodes to be increased
 *@param size size of said array
 * @param *add node to be added to array
 * return new array of nodes
 */
node** increase(node **trees, int size, node* add) {
	int i = 0;
	int j = 0;
	node** forest = malloc(sizeof(node*) * (size + 1));
	for (i = 0; i < size; i++) {
		forest[i] = malloc(sizeof(node*));
		*forest[i] = *trees[i];
		j++;
	}
	forest[j] = add;
	return forest;

}
/*
 * Finds tree with smallest char count and returns the array index of it
 * @param **trees array of nodes
 * @param size, size of tree array
 * return index of node with smallest char count
 */
int getSmallest(node **trees, int size) {
	int i;
	int index = 0;
	for (i = 0; i < size; i++) {
		if (trees[i]->freq < trees[index]->freq) {
			index = i;
		}
	}
	return index;
}
/*
 * Reduces tree array by one
 * @param **trees array of nodes
 * @param size size of tree array
 * @param index of element to skip
 * return new array of trees 
 */
node** reduceOne(node **trees, int size, int skip) {
	int i, j = 0;
	node** forest = malloc(sizeof(node*) * (size - 1));
	for (i = 0; i < size; i++) {
		if (i != skip) {
			forest[j] = malloc(sizeof(node*));
			*forest[j] = *trees[i];
			j++;
		}
	}
	return forest;
}
/*
 * Initializes bit buffer and counter
 * @param *f FILE pointer
 * return BITFILE pointer 
 */
BITFILE *bitOpen(FILE *f) {
	BITFILE *bf;
	bf = calloc(1, sizeof(BITFILE));
	if (!bf) {
		fprintf(stderr, "Memory error!");
		exit(EXIT_FAILURE);
	}
	bf->buffer = 0;
	bf->counter = 0;
	bf->f = f;
	return bf;

}
/*
 * Writes leftover bits to file and frees memory
 * @param *bf BITFILE pointer to be closed
 */
void bitClose(BITFILE *bf) {
	//everything needed is written
	//but buffer may still contain something
	//add 0's to end of fill buffer and force write
	if(bf->counter != 0){
		putbits(8 - (bf->counter), 0, bf);
	}
	free(bf);
}

/*
 * Gets a single bit from bitfile buffer
 * @param *file pointer to BITFILE
 * return int value of bit(0/1)
 */
unsigned int getbit(BITFILE *file) {
	unsigned int answer;
	if (file->counter == 0) {
		fread(&(file->buffer), sizeof(char), 1, file->f);
		//file->buffer = fgetc(file->f);
		file->counter = 8;
	}
	//some bitmagic
	answer = ((file->buffer) >> 0) & 1;
	file->buffer = file->buffer >> 1;
	file->counter--;
	return answer;
}
//cant use regular getc, something might be in buffer
//regulary binary start from left
//this reads from left and then converts
int getbits(int amount, BITFILE *source) {
	int temp, i;
	int answer = 0;
	for (i = 0; i < amount; i++) {
		temp = getbit(source);
		//printf("bit %d ", temp);
		answer = answer | (temp << i);
		//printfbits(9, anser);
	}
	//printfbits(9, answer);
	//printf("read bits to %d\n", answer);
	return answer;
}

/*
 * Sends x bits from char to BITFILE buffer
 * @param nbits amount of bits from char
 * @param word containter for bits
 * @param *bf BITFILE pointer where to send bits
 */
void putbits(int nbits, unsigned int word, BITFILE *bf) {
	int i;
	int current;
	for (i = 0; i < nbits; i++) {
		//take first bit from given input		
		current = (word >> 0) & 1;
		//printf("%d", current);
		//shift to right
		word = word >> 1;
		//write taken bit into next curently free slot(using OR)
		//bf->buffer = bf->buffer | (current << (bf->counter));

		bf->buffer = bf->buffer | (current << (bf->counter));
		bf->counter++;
		if (bf->counter == 8) {
			fwrite(&(bf->buffer), 1, sizeof(bf->buffer), bf->f);
			bf->buffer = 0;
			bf->counter = 0;
		}
	}
}
/*
 * Calculates how many bits! should packed file take
 * @param *tree root of nodes
 * @param **table contains bin representations of chars
 * @param estimation currently counted bits
 * return count after this call
 */
long estimate_count(node * tree, char** table, long estimation){
	int/* i,*/ len;

	if (tree->left == 0 && tree->right == 0 ) {
		//size of char in tree + 1 for prefix
		estimation = estimation +10;
		//freq * len in text part
		len = strlen(table[tree->c]);		
		estimation = estimation + (len * (tree->freq));


		//printf("found end of branch with %d and count %d\n", tree->c, tree->freq);
	}else{
		//+1 for tree node
		estimation++;
		estimation = estimate_count(tree->left, table, estimation);
		estimation = estimate_count(tree->right, table, estimation);
	}
	//~ if (tree->left != 0) {
		//~ estimation = estimate_count(tree->left, table, estimation);
	//~ }
	//~ if (tree->right != 0) {
		//~ estimation = estimate_count(tree->right, table, estimation);
	//~ }
	return estimation;

}


/*
 * Traverses tree and creates translation table
 * @param *tree root tree to start atraversal
 * @param *path already walked path
 * @param *table translation table to be filled
 */
char** traverse_tree(node * tree, char* path, char** table) {
	char* new;
	int help;

	if (tree->left == 0 && tree->right == 0) {
		//printf("encloded %s, with size %d\n", path, (int)strlen(path));
		help = tree->c;
		//printf("writing to location %d\n", help);
		//strcpy(table[help], path);
		table[help] = malloc(sizeof(char) * strlen(path));
		strcpy(table[help], path);
		//table[help] = path;
	}
	if (tree->left != 0) {
		new = malloc(strlen(path) + 2);
		if (!new) {
			printf("Memory error! in tree traversal");
			fprintf(stderr, "Memory error! in tree traversal");
			exit(EXIT_FAILURE);
		}
		new = strcpy(new, path);
		table = traverse_tree(tree->left, strcat(new, "0"), table);
	}
	if (tree->right != 0) {
		new = malloc(strlen(path) + 2);
		if (!new) {
			printf("Memory error! in tree traversal");
			fprintf(stderr, "Memory error! in tree traversal");
			exit(EXIT_FAILURE);
		}
		new = strcpy(new, path);
		table = traverse_tree(tree->right, strcat(new, "1"), table);
	}
	return table;
}


/*
 * Translates binary representation to real bits
 * @param fake_bin char array of 01 to be translated
 * return char containing real binary representation
 */
int translate(char * fake_bin) {
	int i, intified;
	int size = strlen(fake_bin);
	unsigned int real_bin = 0;
	for (i = 0; i < size; i++) {
		intified = fake_bin[i] - '0';
		real_bin = real_bin | (intified << i);
	}
	return real_bin;
}

/*
 * Encodes given buffer according to translation table
 * @param *buffer contains char to be translated
 * @param **table holds fake_bit value of chars
 * @param *bf BITFILE where encoded material is to be saved
 */
void encode(char *buffer, long size, char** table, BITFILE *bf) {
	long i;
	int total = 0;
	int len;
	char* fake;
	unsigned int real;
	unsigned char help;

	for (i = 0; i < size; i++) {
		help = buffer[i];
		//strcpy("");
		fake = table[help];
		len = strlen(fake);
		total = total + len;
		real = translate(fake);
		putbits(len, real, bf);
	}
	//at the end add my eof char to end of file
	fake = table[FAKECHAR - 1];
	len = strlen(fake);
	total = total + len;
	real = translate(fake);
	putbits(len, real, bf);
	//printf("in total wrote %d bits\n", total);
}

/*
 * Decodes source to target until my eof is found
 * @param *target where to write decoded chars
 * @param *tree root from where to start decoding
 * @param *source wrom where to read bits
 */
 void decode(FILE *target, node * tree, BITFILE *source) {
	unsigned int c;
	while ((c = decode_one(source, tree)) != (FAKECHAR - 1)) {
		//double check
		if(c != (FAKECHAR - 1)){
			fwrite(&c, 1, sizeof(unsigned char), target);
		}
	}
}

/*
 * Decodes variable amount of bits to on char
 * @param *file BITFILE on whitch to use getbit()
 * @param *tree translation tree to be traversed
 * return int represenation of char 
 */
unsigned int decode_one(BITFILE *file, node * tree) {
	int bit;
	unsigned int answer;
	while (tree->left && tree->right) {
		bit = getbit(file);
		if (bit) {
			tree = tree->right;
		} else {
			tree = tree->left;
		}
	}
	if (!tree) {
		printf("Translation error\n");
		fprintf(stderr, "Decoding tree error\n");
		exit(EXIT_FAILURE);
	}
	answer = tree->c;
	//printf("%d\n", answer);
	return answer;
}



/*
 * Makes translation tree from given buffer
 * @param *buffer contain char to be encoded
 * @param file_len size of said buffer
 * return *node root tree containing appropriate encoding  
 */
node* make_tree(char *buffer, int file_len) {
	int i, j, not_zero, forest_size, index;
	unsigned char current;
	//make frequency table
	int freq[FAKECHAR] = { 0 };
	for (i = 0; i < file_len; i++) {
		current = buffer[i];
		freq[current] = freq[current] + 1;
	}
	freq[FAKECHAR - 1] = 1;
	//count not zero characters in freq table
	not_zero = 0;
	for (i = 0; i < FAKECHAR; i++) {
		if (freq[i] > 0) {
			not_zero++;
		}
	}
	forest_size = not_zero;

	//create nodes for every nonzero charachter
	node **forest2 = malloc(sizeof(node*) * (not_zero));
	if (!forest2) {
		fprintf(stderr, "Memory error in tree creation\n");
		exit(EXIT_FAILURE);
	}
	//node *forest[not_zero];
	node *b;
	j = 0;

	for (i = 0; i < FAKECHAR; i++) {
		if (freq[i] > 0) {
			b = malloc(sizeof(node));
			b->left = 0;
			b->right = 0;
			b->c = i;
			b->freq = freq[i];
			forest2[j] = b;
			j++;
		}
	}

	node *smallest1 = malloc(sizeof(node*));
	node *smallest2 = malloc(sizeof(node*));
	node *new = malloc(sizeof(node*));
	if (!smallest1 || !smallest2 || !new) {
		fprintf(stderr, "Memory error in tree creation\n");
		exit(EXIT_FAILURE);
	}
	//start building tree from created nodes
	while (forest_size > 1) {
		node *new2 = malloc(sizeof(node));
		index = getSmallest(forest2, forest_size);
		smallest1 = forest2[index];
		forest2 = reduceOne(forest2, forest_size, index);
		forest_size--;
		index = getSmallest(forest2, forest_size);
		smallest2 = forest2[index];
		forest2 = reduceOne(forest2, forest_size, index);
		forest_size--;
		//combine them together
		new2->left = smallest1;
		new2->right = smallest2;
		//new2->c = -1;
		new2->freq = (smallest1->freq + smallest2->freq);

		forest2 = increase(forest2, forest_size, new2);
		forest_size++;
	}
	return forest2[0];
}

/*
 * Writes given translationtree on destination
 * @param root contains pointer to root of nodes
 * @param dest pointer to BITFILE where to write tree
 */
void put_tree(node* root, BITFILE *dest) {
	if (root->left == 0 && root->right == 0) {
		//we are at the end of branch
		//write 0 infront of 9 bits for char representation
		putbits(1, 0, dest);
		//adding like this uses right to left order
		//but elsewhere its left to right
		putbits(9, (root->c), dest);
	} else {
		//still more to go, write 1 to file
		putbits(1, 1, dest);
		put_tree(root->left, dest);
		put_tree(root->right, dest);
	}
}

/*
 * Generates tree from read source
 * @param source pointer to BITFILE from where to read
 * return pointer to built node
 */
node* get_tree(BITFILE *source) {
	int prefix;
	unsigned int c;
	node *tree = malloc(sizeof(node));
	if (!tree) {
		fprintf(stderr, "Memory error!");
		exit(EXIT_FAILURE);
	}
	prefix = getbit(source);
	if (prefix == 1) {
		//tree->c = 0;
		tree->left = get_tree(source);
		tree->right = get_tree(source);
	} else {
		//got to end of branch, will read 9 bits to int
		c = getbits(9, source);
		//printf("got 9bits to %d\n", c);
		tree->left = 0;
		tree->right = 0;
		tree->c = c;
	}

	return tree;
}

void encode_file(const char* in, const char* out) {
	FILE *input;
	FILE *output;
	BITFILE *bfile;
	char *buffer;
	int i;
	long file_len, estim, rounded;
	float bytes;
	
	
	
	//trying to open input file
	input = fopen(in, "rb");
	if (!input) {
		printf("Error opening file\n");
		fprintf(stderr, "Unable to open file %s\n", in);
		exit(EXIT_FAILURE);
	}
	//Get file length
	fseek(input, 0, SEEK_END);
	file_len = ftell(input);
	fseek(input, 0, SEEK_SET);
	//check if maybe was given a empty file
	if(!file_len){
		//got empty file will not pack and make a new empty file
		//printf("Was given an empty file\n");
		fclose(input);
		output = fopen(out, "wb");
		if (!output) {
			printf("Some error\n");
			fprintf(stderr, "Unable to create file %s\n", out);
			exit(EXIT_FAILURE);
		}
		fclose(output);

	}else{
		//didnt get an empty file
		//Allocate memory for file buffer
		buffer = (char *) malloc(file_len + 1);
		if (!buffer) {
			fprintf(stderr, "Memory error! in file buffer allocation\n");
			fclose(input);
			exit(EXIT_FAILURE);
		}

		//Read file contents into buffer close input
		fread(buffer, file_len, 1, input);
		fclose(input);

		//make encoding tree from buffer
		node * forest = make_tree(buffer, file_len);
		//make table for translation
		char** table = malloc(FAKECHAR * sizeof(char*));
		//static char *table[FAKECHAR];
		table = traverse_tree(forest, "", table);


		//~ //get bit estimation
		//~ estim = estimate_count(forest, table, 0);
		//~ //to bytes and round up
		//~ bytes = (float)estim / 8;
		//~ bytes = ceil(bytes);
		//~ //back to long again
		//~ rounded = (long)bytes;
		
		//open output and init BITFILE
		output = fopen(out, "wb");
		if (!output) {
			printf("Some error\n");
			fprintf(stderr, "Unable to create file %s\n", out);
			exit(EXIT_FAILURE);
		}
		bfile = bitOpen(output);

		//~ if(rounded > file_len){
			//~ //will write file without packing
			//~ //will write 0 in first, because that is otherway impossibe
			//~ //always will be 2 nodes(...+ my_eof), so tree will start with 1 
			//~ //printf("packed file will be larger");
			//~ putbits(8, 0, bfile);
			//~ fwrite(buffer, sizeof(buffer), 1, bfile->f);
			//~ 
			//~ 
						//~ 
		//~ }else{
			//~ //packing is actually helpful
			//~ //write tree and encode content
			//~ putbits(8, 255, bfile);
			//~ put_tree(forest, bfile);
//~ 
			//~ encode(buffer,file_len, table, bfile);
			//~ //close bitfile, something may be still in buffer
		//~ }



		//write tree and encode content
		put_tree(forest, bfile);

		encode(buffer,file_len, table, bfile);

		//close output
		bitClose(bfile);
		fclose(output);
	}


}

void decode_file(const char* in, const char* out) {
	FILE *input;
	FILE *output;
	BITFILE *bin;
	char *buffer;
	long file_len;
	int decide;
	int i;
	//trying to open input file
	input = fopen(in, "rb");
	if (!input) {
		printf("Error opening file\n");
		fprintf(stderr, "Unable to open file %s\n", in);
		exit(EXIT_FAILURE);
	}
	//Get file length
	fseek(input, 0, SEEK_END);
	file_len = ftell(input);
	fseek(input, 0, SEEK_SET);
	
	
	
	
	if(!file_len){
		//packed file was empty, will create a new empty file
		fclose(input);
		output = fopen(out, "wb");
		if (!output) {
			printf("Error opening file\n");
			fprintf(stderr, "Unable to open file %s\n", out);
			exit(EXIT_FAILURE);
		}
		fclose(output);
		
		
	}else{
		//didnt get an empty file
		//init BITFILEs
		bin = bitOpen(input);
		
		
		//get bittree from file
		node *root = malloc(sizeof(node));
		root = get_tree(bin);

		//make table for translation
		char** table2 = malloc(FAKECHAR * sizeof(char*));
		table2 = traverse_tree(root, "", table2);

		output = fopen(out, "wb");
		if (!output) {
			printf("Error opening file\n");
			fprintf(stderr, "Unable to open file %s\n", out);
			exit(EXIT_FAILURE);
		}


		decode(output, root, bin);
		
		
		
	
		
		
		//~ decide = getbits(8, bin);
		//~ //printf("decision byte was %d\n", decide);
		//~ if(decide == 255){
			//~ //get bittree from file
			//~ node *root = malloc(sizeof(node));
			//~ root = get_tree(bin);
			//~ //traverse(root);
//~ 
			//~ //make table for translation
			//~ char** table2 = malloc(FAKECHAR * sizeof(char*));
			//~ table2 = traverse_tree(root, "", table2);
//~ 
			//~ decode(output, root, bin);
			//~ 
		//~ }else if(decide == 0){
			//~ buffer = (char *) malloc(file_len +1);
			//~ fread(buffer, file_len , 1, input);
			//~ 
			//~ fwrite(buffer, file_len -1, 1, output);
		//~ }else{
			//~ fprintf(stderr, "Wrong file beginning", out);
			//~ exit(EXIT_FAILURE);
		//~ }
		
		fclose(input);
		fclose(output);
	}

}


//debug functions, not really needed

void traverse(node * tree) {

	if (tree->left == 0 && tree->right == 0 ) {

		printf("found end of branch with %d and count %d\n", tree->c, tree->freq);
	}
	if (tree->left != 0) {

		traverse(tree->left);
	}
	if (tree->right != 0) {

		traverse(tree->right);
	}
}


void readbits(BITFILE *file, int size) {
	int bit, i;
	for (i = 0; i < size; i++) {
		bit = getbit(file);
		printf("%d", bit);
	}

}


void int2bin(int i) {
	size_t bits = sizeof(char) * 8;

	char * str = malloc(bits + 1);
	//if(!str) return NULL;
	str[bits] = 0;

	// type punning because signed shift is implementation-defined
	unsigned u = *(unsigned *) &i;
	for (; bits--; u >>= 1)
		str[bits] = u & 1 ? '1' : '0';

	printf("binary representation: %s\n", str);
	//return str;
}

void printfbits(int nbits, unsigned int word) {
	int i;
	int current;
	//starting from right
	printf("printing  %d bits from int\n", nbits);
	for (i = 0; i < nbits; i++) {
		//take first bit from given input		
		current = (word >> 0) & 1;
		printf("%d", current);
		//shift to right
		word = word >> 1;

	}
	printf("\n");

}
