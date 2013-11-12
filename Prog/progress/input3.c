#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

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
void traverse_tree(node * tree, char* path, char** table);
int translate(char * fake_bin);
void encode(char *buffer, char** table, BITFILE *bf);
unsigned int decode_one(BITFILE *file, node * tree);
void printfbits(int nbits, unsigned int word);
void readbits(BITFILE *file, int size);
void decode(FILE *target, node * tree, BITFILE *source);
node* make_tree(char *buffer, int file_len);
void put_tree(node* root, BITFILE *dest);
node* get_tree(BITFILE *source);
void encode_file(const char* in, const char* out);
void decode_file(const char* in, const char* out);
void traverse(node * tree1, node* tree2);



BITFILE *bitOpen(FILE *f); /*initialise bit input or output*/
unsigned int getbit(BITFILE *file); /*get one bit from bf->buffer */
void putbits(int nbits, unsigned int word, BITFILE *bf); /* put nbits lower bits of word */
void bitClose(BITFILE *bf); /*finish writing. Flush the last bits to bf->file*/
void int2bin(int i);

int main(int argc, const char *argv[]) {

	if (!strcmp(argv[1], "-d")) {
		//printf("you wanted to unpack\n");
		if (argc != 4) {
			printf("Wrong number of arguments\n");
			exit(EXIT_FAILURE);
		}
		decode_file(argv[2], argv[3]);
		printf("file decoding complete!\n");

	} else {
		if (argc != 3) {
			printf("Wrong number of arguments\n");
			exit(EXIT_FAILURE);
		}
		encode_file(argv[1], argv[2]);
		printf("file encoding complete!\n");
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
	putbits(8 - (bf->counter), 0, bf);
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
	int answer, temp, i;
	for (i = 0; i < amount; i++) {
		temp = getbit(source);
		answer = answer | (temp << i);
	}
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
 * Traverses tree and creates translation table
 * @param *tree root tree to start atraversal
 * @param *path already walked path
 * @param *table translation table to be filled
 */
void traverse_tree(node * tree, char* path, char** table) {
	char* new;

	if (tree->left == 0 && tree->right == 0) {
		table[tree->c] = path;
	}
	if (tree->left != 0) {
		new = malloc(strlen(path) + 2);
		if (!new) {
			printf("Memory error! in tree traversal");
			fprintf(stderr, "Memory error! in tree traversal");
			exit(EXIT_FAILURE);
		}
		new = strcpy(new, path);
		traverse_tree(tree->left, strcat(new, "0"), table);
	}
	if (tree->right != 0) {
		new = malloc(strlen(path) + 2);
		if (!new) {
			printf("Memory error! in tree traversal");
			fprintf(stderr, "Memory error! in tree traversal");
			exit(EXIT_FAILURE);
		}
		new = strcpy(new, path);
		traverse_tree(tree->right, strcat(new, "1"), table);
	}
}

void traverse(node * tree1, node* tree2) {

	if (tree1->left == 0 && tree1->right == 0 ) {
		if(tree1->c != tree2->c){
			printf("something wrong with tree\n");
		}
		printf("found end of branch with %d\n", tree1->c);
		printf("found end of branch with %d\n", tree2->c);
	}
	if (tree1->left != 0) {

		traverse(tree1->left, tree2->left);
	}
	if (tree1->right != 0) {

		traverse(tree1->right, tree2->right);
	}
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
void encode(char *buffer, char** table, BITFILE *bf) {
	int total = 0;
	int i, len;
	char* fake;
	unsigned int real;
	unsigned char help;
	int size = strlen(buffer);
	for (i = 0; i < size; i++) {
		help = buffer[i];
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

//decode until
void decode(FILE *target, node * tree, BITFILE *source) {
	unsigned int c;
	while ((c = decode_one(source, tree)) != (FAKECHAR - 1)) {
		if(c != (FAKECHAR - 1)){
			fwrite(&c, 1, sizeof(unsigned char), target);
		}
		//fwrite(&c, 1, sizeof(unsigned int), target);
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
	return answer;
}


void decode_all(FILE *target, node * tree, BITFILE *source) {
	node *tmp_ptr;
	while(1) {

	tmp_ptr = tree;
	while(tmp_ptr->c==-1) {
		if(getbit(source))
			tmp_ptr = tmp_ptr->right;
		else
			tmp_ptr = tmp_ptr->left;
	};

	if(tmp_ptr->c==256)
		break;
	//fwrite(tmp_ptr->c, 1, sizeof(unsigned char), target);
	if(tmp_ptr->c == 44){
		printf("peaks lisama A\n");
	}
	fputc(tmp_ptr->c,target);

	   }
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
		new2->c = -1;
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
		putbits(9, root->c, dest);
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
	int c;
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
		tree->left = 0;
		tree->right = 0;
		tree->c = c;
	}

	return tree;
}

void encode_file(const char* in, const char* out) {
	FILE *input;
	FILE *output;
	char *buffer;
	int i;
	unsigned long file_len;
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

	//Allocate memory for file buffer
	buffer = (char *) malloc(file_len + 1);
	if (!buffer) {
		fprintf(stderr, "Memory error! in file buffer allocation");
		fclose(input);
		exit(EXIT_FAILURE);
	}

	//Read file contents into buffer close input
	fread(buffer, file_len, 1, input);
	fclose(input);

	//make encoding tree from buffer
	node * forest = make_tree(buffer, file_len);

	//make table for translation
	static char *table[FAKECHAR];
	traverse_tree(forest, "", table);
	//~ for (i = 0; i <FAKECHAR; i++){
	 //~ if(table[i]){
	 //~ printf("old %d translates to %s\n", i, table[i]);
	 //~ }
	 //~ }

	//open output and init BITFILE
	output = fopen(out, "wb");
	if (!output) {
		printf("Some error\n");
		fprintf(stderr, "Unable to create file %s\n", out);
		exit(EXIT_FAILURE);
	}
	BITFILE *bfile;
	bfile = bitOpen(output);

	//write tree and encode content
	put_tree(forest, bfile);

	encode(buffer, table, bfile);

	//close output
	bitClose(bfile);
	fclose(output);

}

void decode_file(const char* in, const char* out) {
	FILE *input;
	FILE *output;
	char *buffer;
	unsigned long file_len;
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

	//init BITFILEs
	BITFILE *bin;
	bin = bitOpen(input);

	//get bittree from file
	node *root = malloc(sizeof(node));
	root = get_tree(bin);


	//make table for translation
	static char *table2[FAKECHAR];
	traverse_tree(root, "", table2);
	printf("table made OK\n");
	//~ for (i = 0; i <FAKECHAR; i++){
	 //~ if(table2[i]){
	 //~ printf("new %d translates to %s\n", i, table2[i]);
	 //~ }
	 //~ }

	output = fopen(out, "wb");
	if (!output) {
		printf("Error opening file\n");
		fprintf(stderr, "Unable to open file %s\n", out);
		exit(EXIT_FAILURE);
	}

	decode(output, root, bin);
	printf("decode OK\n");

}


//debug functions, not really needed

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


