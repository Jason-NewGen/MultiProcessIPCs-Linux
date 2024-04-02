#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){
    if(argc != 4){
        printf("Usage: %s <low> <high> <output file>\n", argv[0]);
        return 1;
    }

    // initialize variables from command line arguments and the populated array
    int l = atoi(argv[1]);
    int h = atoi(argv[2]);
    int np = atoi(argv[3]);
    int* fd = (int*)malloc(np * 2 * sizeof(int));
    if(fd == NULL){
        printf("Error: memory allocation failed\n");
        return 1;
    }

    // allocate memory to an array of size l
    int* numbers = (int*)malloc(l * sizeof(int));
    if(numbers == NULL){
        printf("Error: memory allocation failed\n");
        return 1;
    }

    // validate the amount of hidden keys to generate
    if(h < 30 || h > 60){
        printf("Error: high must be between 30 and 60\n");
        return 1;
    }
    
    // initialize the random number generator and populate the array with random numbers 1 to 100
    srand(time(NULL));
    for(int i = 0; i < l; i++){
        numbers[i] = rand() % 100 + 1;
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

    // find the average, maximum value, and hidden keys using different processes
    
    // create the pipes for the child processes
    for(int i = 0; i < np; i++){
        if(pipe(fd + i * 2) == -1){
            printf("Error: pipe creation failed\n");
            return 1;
        }
    }

    // create the child processes
    for(int i = 0; i < np; i++){
        int startSeg = i * (l / np);
        int endSeg = (i + 1) * (l / np);
        if(i == np - 1) endSeg = l;

        pid_t pid = fork();
        if(pid < 0){
            printf("Error: fork failed\n");
            return 1;
        }
        // child process calculates segment average
        else if(pid == 0){
            // close the read end of the pipe
            close(fd[i*2]);

            // [0] = sum, [1] = max, [2] = hidden keys
            int childReturn[3];

            // calculate the sum of the segment
            childReturn[0] = 0;
            for(int j = startSeg; j < endSeg; j++){
                childReturn[0] += numbers[j];
            }

            // calculate the maximum value of the segment
            childReturn[1] = -101;
            for(int j = startSeg; j < endSeg; j++){
                if(numbers[j] > childReturn[1]) childReturn[1] = numbers[j];
            }

            // find the number of hidden keys of the segment
            childReturn[2] = 0;
            for(int j = startSeg; j < endSeg; j++){
                if(numbers[j] < 0) childReturn[2]++;
            }

            // child process exits and writes the results to the pipe
            write(fd[i*2 + 1], &childReturn, sizeof(childReturn));
            return 1;
        }
        // parent closes the write end of the pipe
        else close(fd[i*2 + 1]);
    }

    // parent process reads the results from the child processes
    double avg = 0;
    int max = -101;
    int hiddenKeys = 0;
    for(int i = 0; i < np; i++){
        int childReturn[3];
        read(fd[i*2], &childReturn, sizeof(childReturn));
        avg += childReturn[0];
        max = childReturn[1] > max ? childReturn[1] : max;
        hiddenKeys += childReturn[2];
        close(fd[i*2]);
    }

    // wait for children to finish
    while (wait(NULL) > 0);

    avg /= l;
    
    printf("Average: %f\n, Maximum: %d\n Number of Hidden Keys: %d\n", avg, max, hiddenKeys);

    // close, clean up, and return
    fclose(file);
    free(numbers);
    printf("File created successfully\n");
    return 0;
}