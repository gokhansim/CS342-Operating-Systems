

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "function.h"

struct integralInput {
    int inputLower;
    double inputStep;
    double inputSubIntervals;
    int inputI;
};

double sum;

void* calculate(void* args) {
    //printf("Entered thread\n");
    struct integralInput *input = args;
    int lower = input->inputLower;
    double step = input->inputStep;
    double subIntervals = input->inputSubIntervals;
    int i = input->inputI;

    double child_lower,start,end,child_sum;
    int k;
    child_lower = lower + i * subIntervals * step;
    for (k = 0; k < subIntervals; k++) {
        start = child_lower + k * step;
        end = start + step;
        child_sum += (compute_f(start) + compute_f(end)) * step / 2;
    }
    sum += child_sum;
    free(input);
    return NULL;
}

int main(int argc, char *argv[])
{
    sum = 0.0;
    if(argc != 5){
        printf("Usage: integral lowerBound upperBound K-subIntervals N-children\n");
        return 1;
    }

    int lower = atoi(argv[1]);
    int upper = atoi(argv[2]);
    int subIntervals = atoi(argv[3]);
    int noOfThreads = atoi(argv[4]);
    double step = (((upper-lower) * 1.0) / noOfThreads ) / subIntervals;

    pthread_t threads[noOfThreads];

    int i,ret;
    for (i=0; i < noOfThreads; i++) {

        struct integralInput *data = malloc(sizeof(struct integralInput));
        data->inputI = i;
        data->inputLower = lower;
        data->inputStep = step;
        data->inputSubIntervals = subIntervals;

        ret = pthread_create(&threads[i], NULL, calculate, data);

        if (ret) {
            free(data);
            printf("ERROR; return code from pthread_create() is %d\n", ret);
            exit(1);
        }
    }
    int m,rc;
    for (m=0; m < noOfThreads; m++) {
        rc = pthread_join(threads[m], NULL);
        if (rc) {
          printf("ERROR; return code from pthread_join() is %d\n", rc);
          exit(-1);
        }
    }

    printf("%f\n",sum);
}
