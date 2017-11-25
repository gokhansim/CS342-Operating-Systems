#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* print_ints(void* ptr) {
    int a;
    for ( a = 1; a < 101; a++) {
        printf("%d\n",a);
    }
}


int main(int argc, char **argv){
    int noOfThreads;
    if (argc != 2) {
        printf("usage: ./T <numberOfThreads>\n");
        exit(1);
    }
    else if (atoi(argv[1]) > 0) {
        noOfThreads = atoi(argv[1]);
    }
    else {
	printf("Enter a valid positive value.\n");
        exit(1);
    }	

    pthread_t threads[noOfThreads];
    int ret;
    int i;
    int rc;
    for (i=0; i < noOfThreads; i++) {
        ret = pthread_create(&threads[i], NULL, print_ints, NULL);
        if (ret) {
            printf("ERROR; return code from pthread_create() is %d\n", ret);
            exit(1);
        }

    }
    for (i=0; i < noOfThreads; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
          printf("ERROR; return code from pthread_join() is %d\n", rc);
          exit(-1);
        }
    }
    pthread_exit(NULL);

}
