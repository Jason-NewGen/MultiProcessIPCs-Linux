#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Usage: %s <low> <high> <output file>\n", argv[0]);
        return 1;
    }

    // initialize variables from command line arguments and the populated array
    int l = atoi(argv[1]);
    int h = atoi(argv[2]);

    // allocate memory to an array of size l
    int* numbers = (int*)malloc(l * sizeof(int));
    if(numbers == NULL){
        printf("Error: memory allocation failed\n");
        return 1;
    }

    // validate the amount of total integers within the list
    if(!(l >= 10000)){
        printf("Error: list must consist of 10,000 or more integers\n");
        return 1;
    }

    // validate the amount of hidden keys to generate
    if(h < 30 || h > 60){
        printf("Error: number of hidden keys must be between 30 and 60\n");
        return 1;
    }
    
    // initialize the random number generator and populate the array with random numbers 1 to 1000
    srand(time(NULL));
    for(int i = 0; i < l; i++){
        numbers[i] = rand() % 1000 + 1;
    }
    
    // populate the array with hidden keys
    int i = 0;
    while(i < h){
        int index = rand() % l;
        // check if new index is already a hidden key
        if(numbers[index] < 0) continue;

        // populate the key with the hidden key
        else{
            numbers[index] = (rand() % 60 + 1) * -1;
            i++;
        }
    }

    // write the array to a file
    FILE *file = fopen("numbers.txt", "w");
    if(file == NULL){
        printf("Error: file could not be opened\n");
        free(numbers);
        return 1;
    }

    for(int i = 0; i < l; i++){
        fprintf(file, "%d\n", numbers[i]);
    }
    fclose(file);

    printf("File created successfully\n");
    free(numbers); 
    return 0; 
}

