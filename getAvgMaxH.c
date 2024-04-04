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

void dfs(int *numbers, int segSize, int np, int l){
    Results resRead = {0, -100, 0};
    Results resWrite = {0, -101, 0};
    Results temp = {0, 0, 0};
    int* fd = (int*)malloc(np * 2 * sizeof(int));
    pid_t parent = getpid(); 
    for(int i = 0; i <= np; i++){
        if(i == np){
            exit(0);
        }
        pipe(fd + i * 2);
        pid_t pid = fork();
    
        // parent process
        if(parent == getpid()){
            waitpid(pid, NULL, 0);
            read(fd[0], &resRead, sizeof(resRead));
            printf("parent %d\n", i); 
            resWrite.hiddenKeys += resRead.hiddenKeys;
            resWrite.avg += resRead.avg;
            if(resRead.max > resWrite.max){
                resWrite.max = resRead.max;
            }
            resWrite.avg /= l;
        
            printf("Depth First Search Results:\nAverage: %f\nMax: %d\nHidden Keys: %d\n\n", resWrite.avg, resWrite.max, resWrite.hiddenKeys);
            exit(0);
        }
        // child process
        else if(pid > 0){
            waitpid(pid, NULL, 0);
            read(fd[(i)*2], &resRead, sizeof(resRead)); // read from grandchild (2, 3)
            read(fd[(i - 1) * 2], &temp, sizeof(temp));
            printf("child %d\n", i); 
            resWrite.avg += resRead.avg;
            printf("thing%f\n", resWrite.avg); 
            if(resRead.max > resWrite.max){
                resWrite.max = resRead.max;
            }
            resWrite.hiddenKeys = resRead.hiddenKeys;
            printf("%d\n", temp.hiddenKeys);
            write(fd[(i-1)*2+1], &resWrite, sizeof(resWrite)); // write to parent (0,1)
            exit(0);
        }
        // grandchild process
        else{
            int start = i * segSize;
            int end = (i + 1) * segSize;
            if(i == np - 1){
                end = l;
            }

            // analyze the array
            for(int j = start; j < end; j++){
                resWrite.avg += numbers[j];
                if(numbers[j] > resWrite.max){
                    resWrite.max = numbers[j];
                }
                if(numbers[j] < 0){
                    resWrite.hiddenKeys++;
                }
            }
            printf("gchild %d\n", i); 

            // write to the pipe (child to parent)
            write(fd[(i+1)*2-1], &resWrite, sizeof(resWrite)); // [2, 3]

        }
    }
}

void depthFirstSearch(int *numbers, int segSize, int depth, int np){
    pid_t pid;
    if(depth == np){
        // analyze the segment
        Results res = {0, -101, 0};
        for(int i = 0; i < segSize; i++){
            res.avg += numbers[i];
            if(numbers[i] > res.max){
                res.max = numbers[i];
            }
            if(numbers[i] < 0){
                res.hiddenKeys++;
            }
        }

        // write(fd[1], &res, sizeof(res));
        // close(fd[1]);
        exit(0);
    }
    int* fd = (int*)malloc(2 * sizeof(int));
    pipe(fd);
    pid = fork();
    int status;
    

    Results res;
    // if (depth == 0 && pid == 0){
    //     firstchild = getpid();
    // }
    // if (pid == parent){
    //     printf("bruh"); 
    //     parent = getpid();
    // }


    if(pid < 0){
        // error
        printf("Error: fork failed\n");
        exit(1);
    }
    else if(depth != np && pid == 0){
        pid_t childPID = getpid();
        pid_t parentPID = getppid();
        // print out

        printf("Child %d: PID: %d, Parent PID: %d\n", depth+1, childPID, parentPID);
        depthFirstSearch(numbers + segSize, segSize, depth + 1, np);
        // read the read end
        waitpid(pid, &status, 0);
        read(fd[0], &res, sizeof(res));
        // close the read end
        close(fd[0]);
        // analyze the segment
        for(int i = 0; i < segSize; i++){
            res.avg += numbers[i];
            if(numbers[i] > res.max){
                res.max = numbers[i];
            }
            if(numbers[i] < 0){
                res.hiddenKeys++;
            }
        }

        // write to the pipe
        write(fd[1], &res, sizeof(res));
        // close the write end
        close(fd[1]);
        // exit the child process
        exit(0);
    }

    // parent process
    // wait for all child processes to finish
    printf("hi"); 
    waitpid(pid, &status, 0);
    read(fd[0], &res, sizeof(res));
    // close the read end
    close(fd[0]);

    // results
    res.avg /= np;
    printf("Depth First Search Results:\nAverage: %f\nMax: %d\nHidden Keys: %d\n", res.avg, res.max, res.hiddenKeys);
    exit(0);
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
    // Results BFS = breadthFirstSearch(numbers, l, np);
    dfs(numbers, l / np, np, l);

    // printf("Breadth First Search Results:\nAverage: %f\nMax: %d\nHidden Keys: %d\n", BFS.avg, BFS.max, BFS.hiddenKeys);

    // clean up, and return
    free(numbers);
    return 0;
}
