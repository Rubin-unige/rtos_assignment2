
// Program to schedule periodic and aperiodic task
// Assignment file
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

// define driver path 
#define DRIVER_PATH "/dev/special_device"

// code for periodic tasks
void task1_code();
void task2_code();
void task3_code();
// code for aperiodic task
void task4_code();

// characterstics fucntion of periodic task for timing and sync
void *task1(void *);
void *task2(void *);
void *task3(void *);
// characterstics fucntion of aperiodic task for timing and sync
void *task4(void *);

// initialise mutex and condition for aperiodic task
pthread_mutex_t mutex_task_4 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_task_4 = PTHREAD_COND_INITIALIZER;

// define how much time to waste
#define INNERLOOP 100
#define OUTERLOOP 100

// define number of periodic and aperiodic tasks
#define NPERIODICTASKS 3
#define NAPERIODICTASKS 1
#define NTASKS NPERIODICTASKS + NAPERIODICTASKS

// initialise variables 
long int periods[NTASKS];
struct timespec next_arrival_time[NTASKS];
double WCET[NTASKS];
pthread_attr_t attributes[NTASKS];
pthread_t thread_id[NTASKS];
struct sched_param parameters[NTASKS];
int missed_deadlines[NTASKS];

int main(){

    // set task periods
    periods[0] = 300000000; // in nanoseconds, 300 miliseconds
    periods[1] = 500000000; // in nanoseconds, 500 miliseconds
    periods[2] = 800000000; // in nanoseconds, 800 miliseconds
    // for aperiodicn task it is set to zero
    periods[3] = 0;

    // assign name to max and min priority for convenience    
  	struct sched_param priomax;
  	priomax.sched_priority=sched_get_priority_max(SCHED_FIFO);
  	struct sched_param priomin;
  	priomin.sched_priority=sched_get_priority_min(SCHED_FIFO);

    if (getuid() == 0) 
        pthread_setschedparam(pthread_self(), SCHED_FIFO, &priomax);

    int i;
    for (i = 0; i < NTASKS; i++){

        // initialize time 1 and 2 to read clock
        struct timespec time_1, time_2;
        clock_gettime(CLOCK_REALTIME, &time_1);

        // execute each task
        // periodicn tasks
        if(i==0) task1_code();
        if(i==1) task2_code();
        if(i==2) task3_code();
        // aperiodic task
        if(i==3) task4_code();

        clock_gettime(CLOCK_REALTIME, &time_2);
                
        // compute worst case execution time
        WCET[i]= 1000000000*(time_2.tv_sec - time_1.tv_sec) +(time_2.tv_nsec-time_1.tv_nsec);
      	printf("\nWorst Case Execution Time %d=%f \n", i, WCET[i]);
    }
        
    // compute U
    double U = WCET[0]/periods[0]+WCET[1]/periods[1]+WCET[2]/periods[2];

    // compute Ulub 
    // double Ulub = 1; // for harmonic relationship case
    double Ulub = NPERIODICTASKS*(pow(2.0,(1.0/NPERIODICTASKS)) -1); // since the relation is not harmonic so use this

    // check sufficient condition: not satisfied, exit
    if (U > Ulub){
        printf("\n U=%lf Ulub=%lf Non schedulable Task Set", U, Ulub);
        return(-1);
    }

    printf("\n U=%lf Ulub=%lf Scheduable Task Set", U, Ulub);
    fflush(stdout);
    sleep(5);

    // set the minimum priority to the current thread
    if (getuid() == 0) 
        pthread_setschedparam(pthread_self(),SCHED_FIFO,&priomin);  

    // set the attributes of each task, including scheduling policy and priority
    for(i=0; i < NPERIODICTASKS; i++) // periodic task
    { 
        // initialise attribute structere of task i
        pthread_attr_init(&(attributes[i]));

        //set the attributes to tell the kernel that the priorities and policies are explicitly chosen,
		//not inherited from the main thread (pthread_attr_setinheritsched) 
        pthread_attr_setinheritsched(&(attributes[i]), PTHREAD_EXPLICIT_SCHED);

        // set the attributes to set the SCHED_FIFO policy (pthread_attr_setschedpolicy)
		pthread_attr_setschedpolicy(&(attributes[i]), SCHED_FIFO);

		//properly set the parameters to assign the priority inversely proportional to the period
        //parameters[i].sched_priority = priomin.sched_priority+NTASKS - i;
        parameters[i].sched_priority = sched_get_priority_max(SCHED_FIFO) - i;

		//set the attributes and the parameters of the current thread (pthread_attr_setschedparam)
        pthread_attr_setschedparam(&(attributes[i]), &(parameters[i]));
    }

    // aperiodic task
    for (int i =NPERIODICTASKS; i < NTASKS; i++){
        // printf(" nperiodic %d", i);
        pthread_attr_init(&(attributes[i]));
        pthread_attr_setschedpolicy(&(attributes[i]), SCHED_FIFO);

        //set minimum priority (background scheduling)
        parameters[i].sched_priority = 0;
        pthread_attr_setschedparam(&(attributes[i]), &(parameters[i]));
    }
    
    //delare the variable to contain the return values of pthread_create	
  	int iret[NTASKS];

	//declare variables to read the current time
	struct timespec time_1;
	clock_gettime(CLOCK_REALTIME, &time_1);

  	// set the next arrival time for each task. This is not the beginning of the first
	// period, but the end of the first period and beginning of the next one. 
  	for (i = 0; i < NPERIODICTASKS; i++){

        long int next_arrival_nanoseconds = time_1.tv_nsec + periods[i];
        //then we compute the end of the first period and beginning of the next one
        next_arrival_time[i].tv_nsec= next_arrival_nanoseconds%1000000000;
        next_arrival_time[i].tv_sec= time_1.tv_sec + next_arrival_nanoseconds/1000000000;
        missed_deadlines[i] = 0;
    }

	// create all threads(pthread_create)
  	iret[0] = pthread_create( &(thread_id[0]), &(attributes[0]), task1, NULL);
  	iret[1] = pthread_create( &(thread_id[1]), &(attributes[1]), task2, NULL);
  	iret[2] = pthread_create( &(thread_id[2]), &(attributes[2]), task3, NULL);
   	iret[3] = pthread_create( &(thread_id[3]), &(attributes[3]), task4, NULL);

  	// join all threads (pthread_join)
  	pthread_join( thread_id[0], NULL);
  	pthread_join( thread_id[1], NULL);
  	pthread_join( thread_id[2], NULL);
	// pthread_join( thread_id[3], NULL); // not needed for aperiodic task

  	// set the next arrival time for each task. This is not the beginning of the first
	// period, but the end of the first period and beginning of the next one. 
  	for (i = 0; i < NTASKS; i++){

      	printf ("\nMissed Deadlines Task %d=%d", i, missed_deadlines[i]);
		fflush(stdout);
    }

  	exit(0);
        
}

// codes of tasks
void task1_code() { // periodic task 1
    int fd;
    // open driver
    fd = open(DRIVER_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Error opening driver");
        return;
    }
    // write start of message in driver and close it
    char message[4] = "[1";
    write(fd, message, 3);  // Write '[1'
    close(fd);

    // waste time
    int i, j;
    for (i = 0; i < OUTERLOOP; i++) {
        for (j = 0; j < INNERLOOP; j++) {
            sched_yield();
        }
    }

    // open file again
    fd = open(DRIVER_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Error opening driver");
        return;
    }
    // write the end of message and close it
    char end_message[4] = "1]";
    write(fd, end_message, 3);  // Write ']1'
    close(fd);
}

void *task1(void *ptr) {
    cpu_set_t cset;
    CPU_ZERO(&cset);
    CPU_SET(0, &cset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

    int i = 0;
    for (i = 0; i < 100; i++) {
        // Execute application-specific code (open, write, close)
        task1_code();

        // Sleep until the end of the current period (start of next period)
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[0], NULL);
        
        // Update the next arrival time for the next iteration
        long int next_arrival_nanoseconds = next_arrival_time[0].tv_nsec + periods[0];
        next_arrival_time[0].tv_nsec = next_arrival_nanoseconds % 1000000000;
        next_arrival_time[0].tv_sec = next_arrival_time[0].tv_sec + next_arrival_nanoseconds / 1000000000;
    }
}


void task2_code() { // periodic task 2
    int fd;
    // open driver
    fd = open(DRIVER_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Error opening driver");
        return;
    }
    // write start of message in driver and close it
    char message[4] = "[2";
    if (write(fd, message, 3) < 0) {
        perror("Error writing to driver");
        close(fd);
        return;
    }
    close(fd);

    // waste time
    int i, j;
    double uno;
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < INNERLOOP; j++) {
            sched_yield();
            uno = (rand() % 10) * (rand() % 10) % 10; // here this value is used to wake up aperiodic task 4
        }
    }

    if ((int)uno % 2 != 0) { // check if uno value is odd (to execute task 4 often)
        // Signal Task 4 (J4) to execute
        // pthread_mutex_lock(&mutex_task_4);
        pthread_cond_signal(&cond_task_4);  // Wake up Task 4
        // pthread_mutex_unlock(&mutex_task_4);
    }

    // open the driver again
    fd = open(DRIVER_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Error opening driver");
        return;
    }
    // write the end of message in driver and close it
    char end_message[4] = "2]";
    if (write(fd, end_message, 3) < 0) {
        perror("Error writing to driver");
        close(fd);
        return;
    }
    close(fd);
}

void *task2(void *ptr) {
    cpu_set_t cset;
    CPU_ZERO(&cset);
    CPU_SET(0, &cset);  // Bind to CPU core 0
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

    int i;
    for (i = 0; i < 100; i++) {
        // Execute Task 2 specific code (open, write, close)
        task2_code();

        // Sleep until the end of the current period (start of next period)
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[1], NULL);

        // Update the next arrival time for the next iteration
        long int next_arrival_nanoseconds = next_arrival_time[1].tv_nsec + periods[1];
        next_arrival_time[1].tv_nsec = next_arrival_nanoseconds % 1000000000;
        next_arrival_time[1].tv_sec = next_arrival_time[1].tv_sec + next_arrival_nanoseconds / 1000000000;
    }

    return NULL; // Explicit return for thread function
}

void task3_code() {
    int fd;
    // open driver
    fd = open(DRIVER_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Error opening driver");
        return;
    }
    // write start of message in driver and close it
    char message[4] = "[3";
    write(fd, message, 3);  // Write '[3'
    close(fd);

    // waste time
    int i, j;
    for (i = 0; i < OUTERLOOP; i++) {
        for (j = 0; j < INNERLOOP; j++) {
            sched_yield();
        }
    }
    // open driver again
    fd = open(DRIVER_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Error opening driver");
        return;
    }
    // write end of message in driver and close it
    char end_message[4] = "3]";
    write(fd, end_message, 3);  // Write ']3'
    close(fd);
}

void *task3(void *ptr) {
    cpu_set_t cset;
    CPU_ZERO(&cset);
    CPU_SET(0, &cset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

    int i = 0;
    for (i = 0; i < 100; i++) {
        // Execute application-specific code (open, write, close)
        task3_code();

        // Sleep until the end of the current period (start of next period)
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[2], NULL);
        
        // Update the next arrival time for the next iteration
        long int next_arrival_nanoseconds = next_arrival_time[2].tv_nsec + periods[2];
        next_arrival_time[2].tv_nsec = next_arrival_nanoseconds % 1000000000;
        next_arrival_time[2].tv_sec = next_arrival_time[2].tv_sec + next_arrival_nanoseconds / 1000000000;
    }
}
void task4_code() {
    int fd;
    // open driver
    fd = open(DRIVER_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Error opening driver");
        return;
    }
    // write start of message in driver and close it
    char message[4] = "[4";
    write(fd, message, 3);  // Write '[4'
    close(fd);

    // waste time
    int i, j;
    for (i = 0; i < OUTERLOOP; i++) {
        for (j = 0; j < INNERLOOP; j++) {
            sched_yield();
        }
    }
    // open driver again
    fd = open(DRIVER_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Error opening driver");
        return;
    }
    // write end of message in the driver and close it 
    char end_message[4] = "4]";
    write(fd, end_message, 3);  // Write ']4'
    close(fd);
}

void *task4(void *ptr) {
    // Set thread affinity (bind to CPU core 0)
    cpu_set_t cset;
    CPU_ZERO(&cset);
    CPU_SET(0, &cset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

    while (1) {
        // Wait for the signal from Task 2 (J2)
        // pthread_mutex_lock(&mutex_task_4);  // Lock the mutex before waiting on the condition
        pthread_cond_wait(&cond_task_4, &mutex_task_4);  // Wait until J2 signals
        // pthread_mutex_unlock(&mutex_task_4);  // Unlock the mutex

        // Execute Task 4 code when signaled
        task4_code();
    }
    return NULL;
}

