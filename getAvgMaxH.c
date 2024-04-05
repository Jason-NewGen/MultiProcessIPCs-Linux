#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

typedef struct {
    double avg;
    int max;
    int hiddenKeys;
    char message[1000]; 
    char foundhidden[1000];
} Results;

void breadthFirstSearch(int *numbers, int l, int np){
    printf("HI"); 
    int* fd = (int*)malloc(np * 2 * sizeof(int)); // array to hold file descriptors for pipes
    clock_t start = clock();

    Results results = {0, -101, 0, "", ""}; // Initialize results

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
          Results childReturn = {0, -101, 0, ""};
           snprintf(childReturn.message, sizeof(childReturn.message), "Hi I'm Process %d with return arg %d and my parent is %d\n", getpid(), i + 1, getppid()); 
            close(fd[i*2]); // Child writes, so close the read end
            char temp[100]; 

          
            for(int j = startSeg; j < endSeg; j++) {
                childReturn.avg += numbers[j]; // Sum
                if(numbers[j] > childReturn.max) childReturn.max = numbers[j]; // Max
                if(numbers[j] < 0) childReturn.hiddenKeys++; // Count hidden keys
                snprintf(temp, sizeof(temp), "Hi I'm Process %d with return arg %d and I found hidden key in position numbers[%d]\n", getpid(), i +1, j+1); 
                strcat(childReturn.foundhidden, temp);
            }

            write(fd[i*2 + 1], &childReturn, sizeof(childReturn)); // Write results to the pipe

            close(fd[i*2 + 1]); // Close the write end
            exit(i+1); // Terminate child process
        }
        else {
            close(fd[i*2 + 1]); // Parent reads, so close the write end
        }
    }

    // Parent process reads the results from the child processes
    for(int i = 0; i < np; i++) {
        Results childReturn = {0, -101, 0, ""}; 
        read(fd[i*2], &childReturn, sizeof(childReturn));
        results.avg += childReturn.avg;
        if(childReturn.max > results.max) results.max = childReturn.max;
        results.hiddenKeys += childReturn.hiddenKeys;
        strcat(results.message, childReturn.message);
        strcat(results.foundhidden, childReturn.foundhidden);

        close(fd[i*2]); // Close the read end
    }

    while(wait(NULL) > 0); // Wait for all children to finish
    results.avg /= l; // Calculate the average
    free(fd); 
    clock_t end = clock();
    FILE *file = fopen("outputbfs.txt", "w");
    fprintf(file, "BREADTH FIRST SEARCH RESULTS\n");
    fprintf(file, "List of size L = %d\n", l);
    fprintf(file, "Total time to finish: %f seconds\n\n", (double)(end-start)/CLOCKS_PER_SEC);
    fprintf(file, "%s", results.message);
    fprintf(file, "Max = %d, Avg = %f, Number of hidden keys = %d\n\n", results.max, results.avg, results.hiddenKeys);
    fprintf(file, "%s", results.foundhidden);
    fclose(file);
    
}

void depthFirstSearch(int *numbers, int segSize, int np, int l) {
  Results resRead = {0, -100, 0, "", ""};
  int fd[np][2]; 
  pid_t parent = getpid();
  clock_t start = clock();
  pid_t pid;

  for (int x = 0; x < np; x++){
    pipe(fd[x]); 
  }

  for (int i = 0; i <= np; i++) {
    Results resWrite = {0, -101, 0, "", ""};
    
    pid = fork(); 

    // parent process
    if (parent == getpid()) {;
      for (int j = 0; j < np ; j++) {;
        read(fd[j][0], &resRead, sizeof(resRead));
        resWrite.hiddenKeys += resRead.hiddenKeys;
        resWrite.avg += resRead.avg;
        if (resRead.max > resWrite.max) {
          resWrite.max = resRead.max;
        }
        strcat(resWrite.message, resRead.message);
        strcat(resWrite.foundhidden, resRead.foundhidden);
      }
      resWrite.avg /= l;

      clock_t end = clock();

    FILE *file = fopen("outputdfs.txt", "w");
    fprintf(file, "DEPTH FIRST SEARCH RESULTS\n");
    fprintf(file, "List of size L = %d\n", l);
    fprintf(file, "Total time to finish: %f seconds\n\n", (double)(end-start)/CLOCKS_PER_SEC);
    fprintf(file, "%s", resWrite.message);
    fprintf(file, "Max = %d, Avg = %f, Number of hidden keys = %d\n\n", resWrite.max, resWrite.avg, resWrite.hiddenKeys);
    fprintf(file, "%s", resWrite.foundhidden);
    
    fclose(file);
      
      exit(0);

    }
    else if (i == np){
        exit(i); 
    }
    else if (pid > 0 && i != np){
        exit(i+1); 
    }

    else if (pid == 0) {
        snprintf(resWrite.message, sizeof(resWrite.message), "Hi I'm Process %d with return arg %d and my parent is %d\n", getpid(), i + 1, getppid()); 
      int start = i * segSize;
      int end = (i + 1) * segSize;
      char temp[100]; 
      if (i == np - 1) {
        end = l;
      }

      // analyze the array
      for (int j = start; j < end; j++) {
        resWrite.avg += numbers[j];
        if (numbers[j] > resWrite.max) {
          resWrite.max = numbers[j];
        }
        if (numbers[j] < 0) {
          resWrite.hiddenKeys++;
          snprintf(temp, sizeof(temp), "Hi I'm Process %d with return arg %d and I found hidden key in position numbers[%d]\n", getpid(), i +1, j+1); 
          strcat(resWrite.foundhidden, temp);
        }
      }

      // write to the pipe (child to parent)
      write(fd[i][1], &resWrite, sizeof(resWrite));
      close(fd[i][1]); 
    }
  }

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

    int fd[np][2]; 
        breadthFirstSearch(numbers, l, np); 
    
    depthFirstSearch(numbers, l / np, np, l);
    free(numbers);
    return 0;
}