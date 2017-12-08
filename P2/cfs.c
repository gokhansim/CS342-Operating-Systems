#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
static const int prios[40] = {
/* -20 */ 88761, 71755, 56483, 46273, 36291,
/* -15 */ 29154, 23254, 18705, 14949, 11916,
/* -10 */ 9548, 7620, 6100, 4904, 3906,
/* -5 */ 3121, 2501, 1991, 1586, 1277,
/* 0 */ 1024, 820, 655, 526, 423,
/* 5 */ 335, 272, 215, 172, 137,
/* 10 */ 110, 87, 70, 56, 45,
/* 15 */ 36, 29, 23, 18, 15,
};

struct burst {
	int mode; // 0 means I/O, 1 means CPU
	int length;
	struct burst *next_burst;
};

struct process{
	int id; 
	long starttime;
	long endtime;
	int nice_value; // 0-39
	long vruntime; 
	long ran_for;
	long used_timeslice;
	long total_ran_for;
	long total_io;
	double timeslice;
	int resp;
	int resp_occur;
	long resp_time;
	struct process *next_ready;
	struct process *next_general;
	struct process *next_io;
	struct process *next_finished;
	struct process *next_timeslice;
	struct burst *head;
	int current_state; // 1: ready queue, 2: i/o wait, 3: timeslice used/waiting for next latency period
};

struct process *process_new(int id, long starttime, int nice_value, struct burst *head) { 
	struct process *p = (process*)malloc(sizeof *p);
	p->id = id;
	p->starttime = starttime * 1000000;
	p->endtime = 0;
	p->vruntime = 0;
	p->ran_for = 0;
	p->total_ran_for = 0;
	p->total_io = 0;
	p->resp = 0;
	p->resp_occur = 0;
	p->resp_time = 0;
	p->nice_value = nice_value;
	p->used_timeslice = 0;
	p->next_general = NULL;
	p->next_ready = NULL;
	p->next_io = NULL;
	p->next_finished = NULL;
	p->next_timeslice = NULL;
	p->head = head;
	p->current_state = 1;
	return p;
}

struct process *add_p_general(struct process *p, struct process *head) {
	if (!head) {
		head = p;
	}
	else {
		struct process *curr = head;
		struct process *prev = NULL;
		while (curr && curr->starttime < p->starttime) {
			prev = curr;
			curr = curr->next_general;
		}
		if (!curr) {
			prev->next_general = p;
		}
		else {
			if (prev) {
				p->next_general = prev->next_general;
				prev->next_general = p;
			}
			else {
				p->next_general = head;
				head = p;
			}
		}
	}

	return head;
}

struct process *add_p_ready(struct process *p, struct process *head) {
	p->ran_for = 0;
	p->next_io = NULL;
	p->next_timeslice = NULL;
	if (!head) {
		head = p;
	}
	else {
		struct process *curr = head;
		struct process *prev = NULL;
		while (curr && curr->vruntime <= p->vruntime) {
			if ( curr->vruntime == p->vruntime && curr->id > p->id)
				break;
			prev = curr;
			curr = curr->next_ready;
		}
		if (!curr) {
			prev->next_ready = p;
			p->next_ready = NULL;
		}
		else {
			if (prev) {
				p->next_ready = prev->next_ready;
				prev->next_ready = p;
			}
			else {
				p->next_ready = head;
				head = p;
			}
		}
	}
	return head;
}

struct process *construct_queue(int *num_of_processes, char* arg) {
	// READ INPUT
	FILE * file;
    char line[300];
    int min_vruntime = 10;

	file = fopen( arg , "r");
	if (file == NULL) {
        exit(EXIT_FAILURE);
        printf("Exit\n");
	}

	struct process *head_ready = NULL;
	struct process *head_general = NULL;
    const char s[2] = " ";

    int current_id = -10;
    int id = -5;

    int count = 0;
    int start = 0;

    char* token;

    int starttime;
    int nice_value;

    struct burst *head_b = NULL;
    struct burst *curr_b = NULL;
    int mode = 0;
    int length = 0;

    while (fgets(line, sizeof(line), file)) {
        token = strtok(line,s);
        count = 0;
        id = atoi(token);
        if (current_id != id) {
        	current_id = id;
        	start = 1;
        	head_b = NULL;
        	curr_b = NULL;
        }
        //printf("%d %s\n",current_id,line);
        while( token != NULL ) {
        	//printf("token %s count %d\n", token,count);
      		if ( start == 1) { // first line for a process
      			if ( count == 2) {
      				starttime = atoi(token);
      			}
      			else if ( count == 4) {
      				nice_value = atoi(token);
      				start = 0;
      			}
      			count++;

      		}
      		else {
      			if (strncmp("end",token,3) == 0) {
      				struct process *p = process_new(id, starttime, nice_value, head_b);
      				head_general = add_p_general(p,head_general);
      				*num_of_processes+=1;
      			}

      			else if (strcmp("cpu",token) == 0) {
      				mode = 1;
      			} 

      			else if (strcmp("io", token) == 0) {
      				mode = 0;
      			}

      			else if (atoi(token) != id){
      				length = atoi(token);
      				struct burst *a= (burst*)malloc(sizeof(a));
      				a->mode = mode;
      				a->length = length * 1000000;
      				a->next_burst = NULL;
      				if (!head_b) {
      					head_b = a;
      					curr_b = head_b;
      				}
      				else {
      					curr_b->next_burst = a;
      					curr_b = a;
      				}
      			}
      		}
      		token = strtok(NULL, s);
   		}
    }


	return head_general;
}

void printqueue(struct process *head, int type) {
	struct process *curr_test = head;
	while(curr_test) {
		printf("%d\n",curr_test->id);
		if (type == 0) { // general queue
			curr_test = curr_test->next_general;
		}
		else if (type == 1) { // runqueue 
			curr_test = curr_test->next_ready;
		}
		else { // io queue
			curr_test = curr_test->next_io; 
		}
	}
}

int main(int argc, char *argv[]) {

	int num_of_general = 0; // number of processes
	int num_of_ready = 0; // number of processes in the ready queue

	struct process *head_q = construct_queue(&num_of_general, argv[1]); // general queue
	struct process *head_ready = NULL; // ready queue
	struct process *current_running = NULL; // current running process
	struct process *head_timeslice = NULL;

    struct process *temp = head_q;
    while (temp) {
    	struct burst *temp_b = temp->head;
    	while(temp_b) {
    		if (temp_b->mode == 0)
    			temp->total_io += temp_b->length;
    		else 
    			temp->total_ran_for += temp_b->length;
    		temp_b = temp_b->next_burst;
    	}
    	temp = temp->next_general;
    }

	int min_vruntime = 10000000; // ini

	int prev_id = -10;
	long prev_ran = 0;

	int flag_timeslice = 0;

	struct process *head_finished = NULL;
	struct process *head_io = NULL;

	long latency = 20000000;
	if (num_of_general <= 20) {
		latency = 200000000;
	}
	else {
		latency = num_of_general * 10000000L;
	}
	int idle_for = 0;
	int decision = 0;
	long time;
	int num_of_active = num_of_general; // number of processes that have not finished

	FILE *f = fopen(argv[2],"w");
	if ( f == NULL) {
		printf("Error opening file\n");
		exit(1);
	}

	for (time = 0; num_of_active != 0; time += 1000000) {
		decision = 0;

		//if (head_io) 
		//	printqueue(head_io,2);
		
		temp = head_ready; 
		while(temp) {
			if (temp->resp == 1) {
				temp->resp_time += 1000000;
			}
			temp = temp->next_ready;
		}

		if (current_running) {
			//printf("%d %ld %ld %d\n", current_running->id,time/1000000,current_running->ran_for/1000000, num_of_active);
			if (idle_for != 0) {
				fprintf(f, "idle %d\n", idle_for/1000000);
				idle_for = 0;
			}
			current_running->used_timeslice += 1000000;
			current_running->head->length -= 1000000;
			long add = 1000000L * 1024L / prios[current_running->nice_value];
			current_running->vruntime += add;
			min_vruntime = current_running->vruntime;
			current_running->ran_for += 1000000;
		}
		else if (time != 0){
			idle_for += 1000000;
		}
		
		// NEW PROCESS ARRIVAL
		if (head_q) {
			while (time >= head_q->starttime) {  
				decision = 1;
				head_q->vruntime = min_vruntime - 10000000LL;
				head_q->resp = 1;
				head_q->resp_occur += 1;
				head_ready = add_p_ready(head_q, head_ready);
				head_q = head_q->next_general; // deleting it from general queue
				if (!head_q) 
					break;
			}
		}
		
		// Decrementing length of I/O bursts of processes
		struct process *curr_io = head_io;
		struct process *prev = NULL;
		while(curr_io) {
			curr_io->head->length -= 1000000;
			struct process *temp = curr_io;
			curr_io = curr_io->next_io;
			if (temp->head->length <= 0) { //PROCESS IS READY TO GET OUT OF THE I/O QUEUE
				decision = 1;
				//printf("%d got out of I/O at %ld\n", temp->id, time/1000000);
				if (temp->head->next_burst->mode == 1) {
					temp->head = temp->head->next_burst;
				}
				else {
					printf("CPU does not follow I/O\n");
					exit(1);
				}

				if (temp->vruntime < min_vruntime - latency) {
					temp->vruntime = min_vruntime - latency;
				}
				temp->resp = 1;
				temp->resp_occur += 1;
				temp->next_io = NULL;
				temp->next_ready = NULL;
				head_ready = add_p_ready(temp,head_ready); // adding to ready queue

				// deleting from i/o queue
				if (!prev) { // deleting head
					head_io = curr_io;
					prev = NULL;
				}
				else {
					prev->next_io = curr_io;
				}
			}else {
				prev = temp;
			}
		}

		//CPU BURST FINISH
		if (current_running){
			if (current_running->head->length <= 0) {
				decision = 1;
				current_running->head = current_running->head->next_burst;
				//PROCESS FINISH
				if (!current_running->head) {
			
					fprintf(f, "%d %ld\n", current_running->id, current_running->ran_for/1000000);
				
					num_of_active--;
					// adding to finished queue
					if (!head_finished) {
						head_finished = current_running;
						head_finished->next_finished = NULL;
					}
					else {
						temp = head_finished;
						struct process *prev = NULL;
						while (temp) {
							if (temp->id > current_running->id)
								break;
							prev = temp;
							temp = temp->next_finished;
						}
						if (!prev) {
							head_finished = current_running;
							head_finished->next_finished = temp;
						}
						else {
							prev->next_finished = current_running;
							current_running->next_finished = temp;
						}
					}

					current_running->endtime = time;
					current_running = NULL;
				}
				else if(current_running->head->mode == 0){ // I/O burst - adding to I/O queue
					//printf("%d ran for %ld before going into I/O\n", current_running->id, current_running->ran_for/1000000);
					
					fprintf(f, "%d %ld\n", current_running->id, current_running->ran_for / 1000000);						
					prev_id = current_running->id;
					prev_ran = current_running->ran_for;
					
					current_running->next_ready = NULL;
					current_running->ran_for = 0;
					if (!head_io) {
						head_io = current_running;
						head_io->next_io = NULL;
					}
					else {
						struct process *temp = head_io->next_io;
						head_io->next_io = current_running;
						current_running->next_io = temp;
					}
					current_running = NULL;
				}
			}
		}

		if (current_running) {
			if (time % 10000000 == 0 && current_running->used_timeslice >= current_running->timeslice) {
				//printf("%d %ld timeslice expired\n", current_running->id, 
				//		current_running->ran_for/1000000);
				flag_timeslice = 1;
				prev_id = current_running->id;
				prev_ran = current_running->ran_for;
				current_running->ran_for = 0;
				current_running->next_ready = NULL;
				current_running->next_timeslice = head_timeslice;
				head_timeslice = current_running;
				current_running = NULL;
				decision = 1;
			}
		}

		if ( time % latency == 0 ) {
			while(head_timeslice) {
				struct process *temp = head_timeslice;
				head_timeslice = head_timeslice->next_timeslice;
				temp->used_timeslice = 0;
				temp->next_timeslice = NULL;
				temp->ran_for = 0;
				head_ready = add_p_ready(temp,head_ready);
				decision = 1;
			}
			head_timeslice = NULL;
			struct process *temp = head_ready;
			while(temp) {
				temp->used_timeslice = 0;
				temp->next_timeslice = NULL;
				temp = temp->next_ready;
			}
			//printqueue(head_ready, 1);
			temp = head_io;
			while(temp) {
				temp->used_timeslice = 0;
				temp->next_timeslice = NULL;
				temp = temp->next_io;
			}
			if (current_running)
				current_running->used_timeslice = 0;
		}

		// DECISION - CURRENT RUNNING MUST NOT BE FINISHED
		if (decision == 1) {
			if (head_ready) {
				if (!current_running) {
					current_running = head_ready;
					head_ready = head_ready->next_ready;
				}
				else if (min_vruntime >= head_ready->vruntime) {
					fprintf(f, "%d %ld\n", current_running->id, current_running->ran_for / 1000000);	
					head_ready = add_p_ready(current_running,head_ready); // putting it back into ready queue					
					min_vruntime = head_ready->vruntime;
					current_running = head_ready;
					current_running->ran_for = 0;
					head_ready = head_ready->next_ready;
				}
				current_running->resp = 0;
				int total_load = 0;
				num_of_ready = 0;
				struct process *curr = head_ready;
				while(curr) {
					total_load += prios[curr->nice_value];
					curr = curr->next_ready;
				}
				curr = head_timeslice;
				while(curr) {
					total_load += prios[curr->nice_value];
					curr = curr->next_timeslice;
				}
				total_load += prios[current_running->nice_value];
				current_running->timeslice = latency * (double)prios[current_running->nice_value] / total_load;
				if (flag_timeslice == 1 && prev_id == current_running->id) {
					current_running->ran_for = prev_ran; 
				}
				else if (flag_timeslice == 1) {
					fprintf(f, "%d %ld\n", prev_id, prev_ran / 1000000);			
				}
				flag_timeslice = 0;
			}
			else {
				current_running = NULL;
			}
		}
	}

	temp = head_finished;
	while (temp) {
		printf("%d %d %ld %ld %ld %ld %ld\n", temp->id, temp->nice_value, temp->starttime / 1000000, temp->endtime / 1000000,
				(temp->endtime - temp->starttime) / 1000000, 
				(temp->endtime - temp->starttime - temp->total_ran_for - temp->total_io) / 1000000,
				(temp->resp_time / 1000000 / temp->resp_occur));
		temp = temp->next_finished;
	}

	fclose(f);
	return 1;
}