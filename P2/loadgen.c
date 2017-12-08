#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>

double expo(double lambda){
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}

int main( int argc, char * argv[] ){
	if (argc != 7){
		printf("Enter 6 arguments!\n");
		exit(1);
	}

	int n = atoi(argv[1]);
	int avg_s = atoi(argv[2]);
	int avg_n_b = atoi(argv[3]);
	int avg_cpu = atoi(argv[4]);
	int avg_io = atoi(argv[5]);

	FILE * f = fopen(argv[6], "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    int i, k;
    srand(time(NULL));
    for( i = 1; i <= n; i++){
        fprintf(f,"%d start %d prio %d\n", i, (int) expo( 1 / (double)avg_s) + 1, rand() % 41 - 1 );
        fprintf(f, "%d cpu %d\n", i, (int) expo( 1 / (double) avg_cpu) + 1);
        for( k=1; k < 2 * (int)expo( 1 / (double) avg_n_b) + 1; k++){
            fprintf(f, "%d io %d\n", i, (int)expo( 1 / (double) avg_cpu) + 1);
            fprintf(f, "%d cpu %d\n", i, (int)expo( 1 / (double) avg_io) + 1);
        }
        fprintf(f, "%d end\n", i);
	}
}
