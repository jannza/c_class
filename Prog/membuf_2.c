#include <stdio.h>

int main(){
	int a = 1;
	int b = 4;
	int c = 6;
	char string[10] = "hello";
	printf("Value of a: %d ,", a);
	printf("address of its memory: %p\n",&a);

	//printf("Value of b: %d ,", b);
        //printf("address of its memory: %p\n",&b);

	//printf("Value of c: %d ,", c);
        //printf("address of its memory: %p\n",&c);

	printf("Value of string: %s ,", string);
        printf("address of its memory: %p\n",&string);

	foo();

	return 0;
}
void foo(){
	int i = 5;
	int j = 7;
	int *ia = &i;
	int k;

	printf("Size of int: %d\n", sizeof(int));
	printf("Value of i: %d ,", i);
	printf("address of its memory: %p\n", &i);
	printf("address of previous: %p\n", ia-1);
	printf("address of next: %p\n", ia+1);
	printf("Located there: %d\n", *(ia+1));
	for(k = 0; k<30;k++){
		printf("Found something: %d\n", *(ia+k));
	}
	//printf("Value of j: %d ,", j);
        //printf("address of its memory: %p\n", &j);
	
	

}
