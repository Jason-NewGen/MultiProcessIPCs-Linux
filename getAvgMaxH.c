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

Results breadthFirstSearch(int *numbers, int l, int np){
    int* fd = (int*)malloc(np * 2 * sizeof(int)); // array to hold file descriptors for pipes

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

void depthFirstSearch(int *numbers, int segSize, int np, int l) {
  Results resRead = {0, -100, 0, "", ""};
  int fd[np][2]; 
  pid_t parent = getpid();
  int count = 0; 
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

    FILE *file = fopen("output.txt", "w");
    fprintf(file, "%s", resWrite.message);
    fprintf(file, "Max = %d, Avg = %f, Number of hidden keys = %d\n\n", resWrite.max, resWrite.avg, resWrite.hiddenKeys);
    fprintf(file, "%s", resWrite.foundhidden);
    
    fclose(file);
      
      exit(0);

    }
    else if (i == np){
        exit(0); 
    }
    else if (pid > 0 && i != np){
        exit(0); 
    }

    else if (pid == 0) {
        snprintf(resWrite.message, sizeof(resWrite.message), "Hi I'm Process %d and my parent is %d\n", getpid(), getppid()); 
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
          snprintf(temp, sizeof(temp), "Hi I'm Process %d and I found hidden key in position numbers[%d]\n", getpid(), j+1); 
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
    
    depthFirstSearch(numbers, l / np, np, l);
    free(numbers);
    return 0;
}