#include <stdio.h>

typedef struct{
		int first;
		char str[7];	
	}random;

int main(){
	int a = 1;
	int b = 4;
	int c = 6;
	char string[10] = "hello";
	
	random rnd={7, "more!"};

	printf("Value of a: %d ,", a);
	printf("address of its memory: %p\n",&a);

	printf("Value of b: %d ,", b);
        printf("address of its memory: %p\n",&b);
	a = 11 + 5;
	printf("Value of c: %d ,", c);
        printf("address of its memory: %p\n",&c);

	printf("Value of string: %s ,", string);
        printf("address of its memory: %p\n",&string);

	foo();

	return 0;
}
void foo(){
	int i = 5;
	int j = 7;
	printf("Value of i: %d ,", i);
	printf("address of its memory: %p\n", &i);

	printf("Value of j: %d ,", j);
        printf("address of its memory: %p\n", &j);
	
	

}
