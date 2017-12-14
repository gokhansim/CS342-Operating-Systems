#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

int end;

struct p_arg {
	int number;
};

struct student{
	int sid;
	char firstname[64];
	char lastname[64];
	double cgpa;
	struct student *next;
	struct student *next_s;
};

struct buffer_q {
	struct student *head;
	struct student *tail;
	int count;
};

struct bounded_buffer {
	struct buffer_q *q;
	pthread_mutex_t th_mutex_q;
	pthread_cond_t th_cond_space;
};

/******************************************************************/
int bufsize;
struct bounded_buffer** buffers;
char *infilename; 
char *outfilename;
int number_of_buffers;
int full_buffers;
pthread_mutex_t item;
pthread_cond_t th_cond_item;
/******************************************************************/

void buffer_q_insert (struct buffer_q *q, struct student *s) {
	if(q->count == 0) {
		q->head = s;
		q->tail = s;
	}
	else {
		q->tail->next = s;
		q->tail = s;
	}
	q->count++;
}

struct student* buffer_q_retrieve (struct buffer_q *q) {
	struct student *s;

	if (q->count == 0)
		return NULL;

	s = q->head;
	q->head = q->head->next;
	if (q->head == NULL) {
		q->tail = NULL;
	}
	q->count--;

	s->next = NULL;
	return s;
}

struct student* sorted_insert (struct student *head, struct student *s)
{
	//printf("Inserting %d\n", s->sid);
	if (!head)
		head = s;
	else {
		struct student *curr = head;
		struct student *prev = NULL;

		while (curr && curr->sid < s->sid) {
			prev = curr;
			curr = curr->next_s;
		}

		if (!curr) {
			prev->next_s = s;
			s->next_s = NULL;
		}
		else {
			if (prev) {
				s->next_s = curr;
				prev->next_s = s;
			}
			else {
				s->next_s = head;
				head = s;
			}
		}
	}
	//printf("Inserted\n");

	return head;

}

void * produce(void * cont) {
	struct p_arg *arg = cont;
	FILE *fp;
	int id = arg->number;
	fp = fopen(infilename, "r");
	int in_id;
	int sid;
	char firstname[64];
	char lastname[64];
	double cgpa;
	char str[250];
	while (fgets (str, 250, fp)) {
		if (sscanf(str, "%d %d %s %s %lf", &in_id, &sid, firstname, lastname, &cgpa) != 5) {
			printf("sscanf error\n");
			exit(1);
		}
		if(in_id == id) {
			//printf("%d %s\n", in_id, firstname);
			struct student *s = (struct student *) malloc(sizeof (struct student));
			if (s == NULL) {
				perror("Malloc failed\n");
				exit(1);
			}

			s->sid = sid;
			strcpy(s->firstname, firstname);
			strcpy(s->lastname, lastname);
			s->cgpa = cgpa;
			s->next = NULL;
			s->next_s = NULL;

			pthread_mutex_lock(&buffers[in_id]->th_mutex_q);

			//CRITICAL SECTION BEGIN

			//if buffer is full, wait
			while(buffers[id]->q->count == bufsize) {
				//printf("Producer %d is full, going to sleep...zZz...\n", id);
				pthread_cond_wait(&buffers[id]->th_cond_space,&buffers[id]->th_mutex_q);
				//printf("Producer %d is awake!\n", id);
			}

			buffer_q_insert(buffers[id]->q,s);

			if (buffers[id]->q->count == 1) {
				pthread_mutex_lock(&item);
				full_buffers++;
				if (full_buffers == 1)
					pthread_cond_signal(&th_cond_item);
				pthread_mutex_unlock(&item);
				
			}

			pthread_mutex_unlock(&buffers[id]->th_mutex_q);

			//CRITICAL SECTION END

		}
	}

	end++;
	pthread_cond_signal(&th_cond_item);
	fclose(fp);
}

void *consume(void* cont) {
	//printf("Consumer has begun\n");
	int m;
	int emptied;
	struct student *head_result = NULL;
	while (end != number_of_buffers) {
		pthread_mutex_lock(&item);
		if (full_buffers == 0) {
			//printf("Consumer sleeping...zZz...\n");
			pthread_cond_wait(&th_cond_item,&item);
			//printf("Consumer is awake!\n");
		}
		pthread_mutex_unlock(&item);

		for (m = 0; m < number_of_buffers; m++) {
			emptied = 0;
			pthread_mutex_lock(&buffers[m]->th_mutex_q);
			struct student *s;
			while(buffers[m]->q->count > 0) {
				s = buffer_q_retrieve(buffers[m]->q);
				if (!s) {
					printf("ERROR");
					exit(1);
				}
				head_result = sorted_insert(head_result,s);
				if (buffers[m]->q->count == bufsize - 1)
					pthread_cond_signal(&buffers[m]->th_cond_space);
				emptied = 1;
			}
			pthread_mutex_lock(&item);
			if (emptied == 1) 
				full_buffers--;
			pthread_mutex_unlock(&item);
			pthread_mutex_unlock(&buffers[m]->th_mutex_q);
		}
	}

	struct student *curr = head_result;
	//printf("Finished\n");
	
	FILE *fp;
	fp = fopen(outfilename, "w");
	while (curr) {
		fprintf(fp, "%d %s %s %.2f\n", curr->sid, curr->firstname, curr->lastname, curr->cgpa);
		curr = curr->next_s;
	}
	fclose(fp);

}

int main(int argc, char *argv[])
{
	if (argc != 5) {
		printf("Usage: pcsync <number_of_threads> <buffersize> <infilename> <outputfilename>\n");
		return 1;
	}
	end = 0;
	full_buffers = 0;
	number_of_buffers = atoi(argv[1]);
	bufsize = atoi(argv[2]);
	infilename = argv[3];
	outfilename = argv[4];
	buffers = malloc( sizeof(struct bounded_buffer)*number_of_buffers);
	pthread_t threads[number_of_buffers];
	pthread_t consumer;
	struct p_arg* args[number_of_buffers];
	pthread_cond_init(&th_cond_item, NULL);
	int i;

	for ( i = 0; i < number_of_buffers; i++) {
		buffers[i] = (struct bounded_buffer *) malloc(sizeof (struct bounded_buffer));
		buffers[i]->q =  (struct buffer_q *) malloc(sizeof (struct buffer_q));
		buffers[i]->q->count = 0;
		buffers[i]->q->head = NULL;
		buffers[i]->q->tail = NULL;
		pthread_mutex_init(&buffers[i]->th_mutex_q, NULL);
	 	pthread_cond_init(&buffers[i]->th_cond_space, NULL);
	}


	int result;

	result = pthread_create(&consumer,NULL,consume,NULL);
	if ( result != 0) {
		perror("Thread creation failed!\n");
		exit(1);
	}

	for ( i = 0; i < number_of_buffers; i++) {
		args[i] = (struct p_arg *) malloc(sizeof (struct p_arg));
		args[i]->number = i;
		result = pthread_create (&threads[i],NULL,produce,args[i]);
		if (result != 0) {
			perror("Thread creation failed!\n");
			exit(1);
		}
	}

	for(i=0; i<number_of_buffers; i++)
	{
	    pthread_join(threads[i], NULL);
	}
	pthread_join(consumer,NULL);

}