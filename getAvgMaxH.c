#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct {
    double avg;
    int max;
    int hiddenKeys;
} Results;

Results breadthFirstSearch(int *numbers, int l, int np){
    int* fd = (int*)malloc(np * 2 * sizeof(int)); // array to hold file descriptors for pipes

    Results results = {0, -101, 0}; // Initialize results

    // Create the pipes for the child processes
    for(int i = 0; i < np; i++) {
        if(pipe(fd + i * 2) == -1) {
            printf("Error: pipe creation failed\n");
            exit(1); // Use exit here since we're not in main
        }
    }

    // Create the child processes
    for(int i = 0; i < np; i++) {
        int startSeg = i * (l / np);
        int endSeg = (i + 1) * (l / np);
        if(i == np - 1) endSeg = l;

        pid_t pid = fork();
        if(pid < 0) {
            printf("Error: fork failed\n");
            exit(1); // Use exit here since we're not in main
        }
        else if(pid == 0) {
            close(fd[i*2]); // Child writes, so close the read end

            int childReturn[3] = {0, -101, 0};
            for(int j = startSeg; j < endSeg; j++) {
                childReturn[0] += numbers[j]; // Sum
                if(numbers[j] > childReturn[1]) childReturn[1] = numbers[j]; // Max
                if(numbers[j] < 0) childReturn[2]++; // Count hidden keys
            }

            write(fd[i*2 + 1], &childReturn, sizeof(childReturn)); // Write results to the pipe
            close(fd[i*2 + 1]); // Close the write end
            exit(0); // Terminate child process
        }
        else {
            close(fd[i*2 + 1]); // Parent reads, so close the write end
        }
    }

    // Parent process reads the results from the child processes
    for(int i = 0; i < np; i++) {
        int childReturn[3];
        read(fd[i*2], &childReturn, sizeof(childReturn));
        results.avg += childReturn[0];
        if(childReturn[1] > results.max) results.max = childReturn[1];
        results.hiddenKeys += childReturn[2];
        close(fd[i*2]); // Close the read end
    }

    while(wait(NULL) > 0); // Wait for all children to finish

    results.avg /= l; // Calculate the average
    free(fd); 
    return results;
}

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Usage: %s <low> <high> <output file>\n", argv[0]);
        return 1;
    }

    // initialize variables from command line arguments 
    int np = atoi(argv[1]);

    // get the file integers
    FILE *file;
    int *numbers = malloc(sizeof(int)); // Dynamically allocate an array for one integer
    int count = 0;
    int temp;
    int l; // for the length of the array later

    file = fopen("numbers.txt", "r");
    if (file == NULL) {
        printf("Error opening file\n");
        free(numbers); // Free the allocated memory before exiting
        return 1;
    }

    while (fscanf(file, "%d", &temp) == 1) {
        numbers[count] = temp;
        count++;
        int *new_numbers = realloc(numbers, (count + 1) * sizeof(int)); // Resize the array to hold one more integer
        if (new_numbers == NULL) {
            printf("Error reallocating memory\n");
            free(numbers); // Free the original array before exiting
            fclose(file);
            return 1;
        }
        numbers = new_numbers;
    }
    l = count; 

    fclose(file);
    
    // find the average, maximum value, and hidden keys using breadth first search vs depth first search
    Results BFS = breadthFirstSearch(numbers, l, np);

    printf("Breadth First Search Results:\nAverage: %f\nMax: %d\nHidden Keys: %d\n", BFS.avg, BFS.max, BFS.hiddenKeys);

    // clean up, and return
    free(numbers);
    return 0;
}
