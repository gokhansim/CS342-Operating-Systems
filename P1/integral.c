

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "function.h"
#include <math.h>

int main(int argc, char *argv[])
{
    //checking arguments
    if(argc != 5){
        printf("Usage: integral lowerBound upperBound K-subIntervals N-children\n");
        return 1;
    }

    int lower = atoi(argv[1]);
    int upper = atoi(argv[2]);
    int subIntervals = atoi(argv[3]);
    int children = atoi(argv[4]);
    double step = (((upper-lower) * 1.0) / children ) / subIntervals;
    int i, k;
    int fd[children][2];
    double start,end,value_rec;
    double sum = 0.0;
    double child_lower,child_sum;
    pid_t pid;

    for(i=0;i<children;i++) {
	pipe(fd[i]);
        pid = fork();
        if (pid < 0){
            printf("Fork() not successful.\n");
            exit(1);
        }
        else if (pid == 0) {
            child_sum = 0.0;
            close(fd[i][0]);
            child_lower = lower + i * subIntervals * step;
            for (k = 0; k < subIntervals; k++) {
                start = child_lower + k * step;
                end = start + step;
                child_sum += (compute_f(start) + compute_f(end)) * step / 2;
            }
            write(fd[i][1], &child_sum, sizeof(child_sum));
            close(fd[i][1]);
            exit(0);
        }
        else {
            close(fd[i][1]);
            int returnStatus;
            waitpid(pid, &returnStatus, 0);
            read(fd[i][0], &value_rec, sizeof(value_rec));
            sum += value_rec;
            close(fd[i][0]);
        }
    }
    printf("%f\n",sum);
    return 1;
}
