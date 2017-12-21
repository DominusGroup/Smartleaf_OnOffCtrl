#include <stdio.h>

#define KEYPAD_INPUTS 3

 
int main (int argc, char ** argv) {

	char word[64] ; 
	
	fgets(word, KEYPAD_INPUTS, stdin) ; 
	printf("keyboard = %s\n", word) ; 

	return 0 ; 
}
