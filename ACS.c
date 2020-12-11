//Hang Ruan
//V00923058

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<readline/readline.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/time.h>
#include<semaphore.h>

#define MAXLINE 20
#define NClerks 4
#define NQUEUE 2

struct customer_info{ /// use this struct to record the customer information read from customers.txt
    int user_id;
    int class_type;
    int service_time;
    int arrival_time;
    struct customer_info *next;
};
static struct timeval start_time;   // record the simulation start time
/*record customer inf for each queue*/
struct customer_info* businessQueue = NULL;
struct customer_info* economyQueue = NULL;


pthread_mutex_t start_time_mutex; // mutex for calculating time used
pthread_mutex_t equeuelock; /* a mutex for changing each queue(eco and business)*/
pthread_mutex_t bqueuelock;
pthread_mutex_t lengthlock; // lock for changes to queue_length
pthread_mutex_t servedlock; // lock for changing total customers served

pthread_mutex_t clerk1lock; // locks for each clerks
pthread_mutex_t clerk2lock;
pthread_mutex_t clerk3lock;
pthread_mutex_t clerk4lock;


pthread_cond_t convar0Empty = PTHREAD_COND_INITIALIZER; //to signal economy queue
pthread_cond_t convar1Empty = PTHREAD_COND_INITIALIZER; //to signal business queue
pthread_cond_t convar1 = PTHREAD_COND_INITIALIZER;  // convar for each clerks
pthread_cond_t convar2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t convar3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t convar4 = PTHREAD_COND_INITIALIZER;


double overall_waiting_time0;   //eco overwaiting time
double overall_waiting_time1;   //business waiting time
    
int queue_length[NQUEUE];   //to store each queue's length
int queue_status[NQUEUE];   //to store which clerk just take a person from the queue


static int count; //total customers
int blength = 0;    //business length
int elength = 0;    //economy length
int cus_served;      //number of customers that's served already

int cus_exit = 0;
int clerk_exit = 0;
//used when cus getting clerk's ID
sem_t mutex0;
sem_t mutex1;

/*inserting a customer to economy linkedlist*/
void eQueueinsert(struct customer_info *inf){
    if(economyQueue == NULL){
        economyQueue = inf;
       
    }else{
        struct customer_info *temp = economyQueue;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = inf;
    }
    return;
}
/*inserting a customer to business linkedlist*/
void bQueueinsert(struct customer_info *inf){
    if(businessQueue == NULL){
        businessQueue = inf;
    }else{
        struct customer_info *temp = businessQueue;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = inf;
    }
    return;
}


//given by the TA to get the current time
double getCurrentSimulationTime(){
    struct timeval cur_time;
    double cur_secs, init_secs;
    pthread_mutex_lock(&start_time_mutex);
    init_secs = (start_time.tv_sec + (double) start_time.tv_usec / 1000000);
    pthread_mutex_unlock(&start_time_mutex);
    gettimeofday(&cur_time, NULL);
    cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
    return cur_secs - init_secs;
}

//thread function
void * clerk_entry(void* clerk_id){
    
    int temp;
    int id = *(int *) clerk_id; //cast void * to int
    
    while(1){
        
        pthread_mutex_lock(&servedlock);
        if(cus_served >= count){    //test whether all customers are served
            pthread_mutex_unlock(&servedlock);
            break;
        }
            pthread_mutex_unlock(&servedlock);
            //find whether a customers in queue needs to be served
            temp = 0;
            pthread_mutex_lock(&lengthlock);
            if(queue_length[1] != 0){   //take a customer from queue 1
                cus_served++;
                queue_length[1]--;
                temp = 2;
               
            }else if(queue_length[0] != 0){ //take a customer from queue 0
                cus_served++;
                queue_length[0]--;
                temp = 1;
            }
            pthread_mutex_unlock(&lengthlock);
                
            if (temp == 0){
                continue;
            }
        
        /*If a customer in business queue, then signal him/her*/
        if(temp == 2){
            
            pthread_mutex_lock(&bqueuelock);
            queue_status[1] = id; // save clerk id
            pthread_cond_signal(&convar1Empty); //signal business queue
            sem_wait(&mutex1);
            pthread_mutex_unlock(&bqueuelock);
            
            //find which mutex to lock
            if(id==1){
                pthread_mutex_lock(&clerk1lock);
            }else if(id == 2){
                pthread_mutex_lock(&clerk2lock);
            }else if(id == 3 ){
                pthread_mutex_lock(&clerk3lock);
            }else{
                pthread_mutex_lock(&clerk4lock);
            }
            
            //wait for the customer thread to finish
            if(id==1){
                pthread_cond_wait(&convar1,&clerk1lock);
            }else if(id == 2){
                pthread_cond_wait(&convar2,&clerk2lock);
            }else if(id == 3){
                pthread_cond_wait(&convar3,&clerk3lock);
            }else{
                pthread_cond_wait(&convar4,&clerk4lock);
            }
            if(id==1){
                pthread_mutex_unlock(&clerk1lock);
            }else if(id == 2){
                pthread_mutex_unlock(&clerk2lock);
            }else if(id == 3 ){
                pthread_mutex_unlock(&clerk3lock);
            }else{
                pthread_mutex_unlock(&clerk4lock);
            }
            
            /*Else if a customer in economy queue, then signal him/her*/
        }else if(temp == 1){
          
            
            pthread_mutex_lock(&equeuelock);
            queue_status[0] = id;  // save clerk id
            pthread_cond_signal(&convar0Empty);//signal economy queue
            sem_wait(&mutex0);
            pthread_mutex_unlock(&equeuelock);
            if(id==1){
                pthread_mutex_lock(&clerk1lock);
            }else if(id == 2){
                pthread_mutex_lock(&clerk2lock);
            }else if(id == 3 ){
                pthread_mutex_lock(&clerk3lock);
            }else{
                pthread_mutex_lock(&clerk4lock);
            }
            
            //wait for the customer thread to finish
            if(id==1){
                pthread_cond_wait(&convar1,&clerk1lock);
            }else if(id == 2){
                pthread_cond_wait(&convar2,&clerk2lock);
            }else if(id == 3){
                pthread_cond_wait(&convar3,&clerk3lock);
            }else{
                pthread_cond_wait(&convar4,&clerk4lock);
            }
            if(id==1){
                pthread_mutex_unlock(&clerk1lock);
            }else if(id == 2){
                pthread_mutex_unlock(&clerk2lock);
            }else if(id == 3 ){
                pthread_mutex_unlock(&clerk3lock);
            }else{
                pthread_mutex_unlock(&clerk4lock);
            }
            
           
        }
    }
    
   
    pthread_exit(NULL);
    return NULL;
}

void *cus_entry(void* cus_info){
    
    //convert void* argument to struct *
    struct customer_info *myInfo =  (struct customer_info *) cus_info;
    usleep(myInfo->arrival_time * 100000);
    // sleep to its arrial_time
    int clerk;
    fprintf(stdout,"A customer arrives: customer ID %2d. \n", myInfo->user_id);
    //economy thread
    if (myInfo->class_type == 0){
        pthread_mutex_lock(&lengthlock);
        queue_length[0]++;
        fprintf(stdout,"A customer enters a queue: the queue ID 0, and length of the queue %2d. \n",queue_length[0]);
        pthread_mutex_unlock(&lengthlock);
      
        double end, start;
      
        pthread_mutex_lock(&equeuelock);
        start = getCurrentSimulationTime();
        pthread_cond_wait(&convar0Empty, &equeuelock);
        
        
        clerk = queue_status[myInfo->class_type];   //find out which customer is serving
        sem_post(&mutex0);
        pthread_mutex_unlock(&equeuelock);
        
          //waiting end time
        end = getCurrentSimulationTime();
        overall_waiting_time0 += (end - start);//total waiting time for this customer
       
        
        clerk = queue_status[myInfo->class_type];   //find out which customer is serving
        start = getCurrentSimulationTime();
        fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %d. \n", start ,myInfo->user_id, clerk);
        
       
        /*lock the serving clerk's mutex*/
        
       
        if(clerk==1){
            pthread_mutex_lock(&clerk1lock);
        }else if(clerk == 2){
            pthread_mutex_lock(&clerk2lock);
        }else if(clerk == 3 ){
            pthread_mutex_lock(&clerk3lock);
        }else{
            pthread_mutex_lock(&clerk4lock);
        }
        
        usleep(myInfo->service_time*100000);
        end = getCurrentSimulationTime(); //find when the clerk ends serving
        fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %d. \n", end,myInfo->user_id,clerk);
        //signaling the clerk that this cus is done
        if(clerk==1){
            pthread_cond_signal(&convar1);
        }else if(clerk == 2){
            pthread_cond_signal(&convar2);
        }else if(clerk == 3){
            pthread_cond_signal(&convar3);
        }else{
            pthread_cond_signal(&convar4);
        }
        if(clerk==1){
            pthread_mutex_unlock(&clerk1lock);
        }else if(clerk == 2){
            pthread_mutex_unlock(&clerk2lock);
        }else if(clerk == 3 ){
            pthread_mutex_unlock(&clerk3lock);
        }else{
            pthread_mutex_unlock(&clerk4lock);
        }
        
       
      //business  thread
    }else if(myInfo->class_type == 1){
        pthread_mutex_lock(&lengthlock);
        queue_length[1]++;
        fprintf(stdout,"A customer enters a queue: the queue ID 1, and length of the queue %2d. \n",queue_length[1]);
        pthread_mutex_unlock(&lengthlock);
        
        double start, end; //waiting start time, end time
        pthread_mutex_lock(&bqueuelock);
        start = getCurrentSimulationTime();
        pthread_cond_wait(&convar1Empty, &bqueuelock);
       
      
        
        clerk = queue_status[myInfo->class_type];   //find out which customer is serving
        sem_post(&mutex1);
        pthread_mutex_unlock(&bqueuelock);
        
        end = getCurrentSimulationTime();
        overall_waiting_time1 += (end - start);//total waiting time for this customer
       
        start = getCurrentSimulationTime();
      
        fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %d. \n", start,myInfo->user_id, clerk);
        
        
        /*lock the serving clerk's mutex*/
        if(clerk==1){
            pthread_mutex_lock(&clerk1lock);
        }else if(clerk == 2){
            pthread_mutex_lock(&clerk2lock);
        }else if(clerk == 3 ){
            pthread_mutex_lock(&clerk3lock);
        }else{
            pthread_mutex_lock(&clerk4lock);
        }
        
        usleep(myInfo->service_time*100000);
        end = getCurrentSimulationTime(); //find when the clerk ends serving
        fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %d. \n", end,myInfo->user_id,clerk);
        //signaling the clerk that this cus is done
        if(clerk==1){
            pthread_cond_signal(&convar1);
        }else if(clerk == 2){
            pthread_cond_signal(&convar2);
        }else if(clerk == 3){
            pthread_cond_signal(&convar3);
        }else{
            pthread_cond_signal(&convar4);
        }
        if(clerk==1){
            pthread_mutex_unlock(&clerk1lock);
        }else if(clerk == 2){
            pthread_mutex_unlock(&clerk2lock);
        }else if(clerk == 3 ){
            pthread_mutex_unlock(&clerk3lock);
        }else{
            pthread_mutex_unlock(&clerk4lock);
        }
        
      
    }
    
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char **argv){
    
    FILE *fp;
    char fileName[25];
    
    if(argc !=2){
        printf("Invalid argument\n");
        return 1;
    }
    // copy the filename into FileName
    strcpy(fileName, argv[1]);
    
    // open the file with read
    fp = fopen(fileName, "r");
    if (fp == NULL){
        perror("Unable to open the file.\n");
        exit(EXIT_FAILURE);
    }
    char line[MAXLINE];
    int first = 1;
    blength = 0;
    elength = 0;
    
   
    fgets(line, MAXLINE, fp);
        
    count = atoi(line); //get the count number from the file
   
    
        // read each line and use strtok to get each attribute from the line
    for (int i=0 ; i<count ; i++){
        fgets(line, MAXLINE, fp);
            char s[2] = ":";
            char s2[2] = ",";
            char *token;
            struct customer_info *temp = (struct customer_info*)malloc(sizeof(struct customer_info));
            //store the attribute into struct attribute
            token = strtok(line,s);
            //saving user_id
            temp->user_id = atoi(token);
            //saving user type
            token = strtok(NULL, s2);
            int type = atoi(token);
            temp->class_type = type;
            //saving user arrival_time
            token = strtok(NULL,s2);
            temp->arrival_time = atoi(token);
            //saving user service time
            token = strtok(NULL,s2);
            temp->service_time = atoi(token);
        
             // inserting the customer struct into either economy queue or business queue
            if (type == 0){
                eQueueinsert(temp);
                elength++;
            }else if(type == 1){
                bQueueinsert(temp);
                blength++;
            }
        }
    
    fclose(fp); //close the file
   
    
    pthread_t clerkThreads[NClerks+1]; //clerk thread
    pthread_t cus0Threads[count];   //economy customer thread
    pthread_t cus1Threads[count];   //business customer thread
    
    
    //initializing all mutex locks
    pthread_mutex_init(&start_time_mutex,NULL);
    pthread_mutex_init(&equeuelock, NULL);
    pthread_mutex_init(&bqueuelock, NULL);
    pthread_mutex_init(&lengthlock, NULL);
    pthread_mutex_init(&servedlock, NULL);
    
    pthread_mutex_init(&clerk1lock, NULL);
    pthread_mutex_init(&clerk2lock, NULL);
    pthread_mutex_init(&clerk3lock, NULL);
    pthread_mutex_init(&clerk4lock, NULL);
    
    sem_init(&mutex0,0,0);
    sem_init(&mutex1,0,0);
    cus_served = 0;
    

    for (int i=0;i<NQUEUE;i++){
        queue_length[i] = 0;
    }
    gettimeofday(&start_time,NULL); //find initial time when the simulation starts
    
   
    
    struct customer_info *ptr;
    struct customer_info *temp1 = economyQueue;
    struct customer_info *temp2 = businessQueue;
    //create business customer thread
    int err;
    for (int j = 0; j < blength; j++){
            err = pthread_create(&cus1Threads[j],NULL, cus_entry, (void *)temp2);
        
        if(err != 0){
            printf("Can't create thread.\n");
        }
        if(j!=blength-1){
            temp2 = temp2->next;
        }
    }
   
    //create ecnonmy customer thread
    for (int i = 0; i < elength; i++){
        err = pthread_create(&cus0Threads[i],NULL, cus_entry,(void *)temp1);
       
        if(err != 0){
            printf("Can't create thread.\n");
        }
        if(i != elength-1){
            temp1 = temp1->next;
        }
        
    }
    
    //create clerk thread
    for (int i = 1; i < NClerks+1; i++){
        int *dummy = malloc(sizeof(*dummy));
        *dummy = i;
        
        pthread_create(&clerkThreads[i-1], NULL, clerk_entry, (void*) dummy);
    }
   
    
    /*
   //wait for threads to finish
    */
    
    for (int i = 0; i<elength; i++){
        err = pthread_join(cus0Threads[i], NULL);
        if(err != 0 ){
           printf("Can't join economy customer thread.\n");
        }
    }
 
    for(int i=0; i<blength; i++){
        err = pthread_join(cus1Threads[i],NULL);
        if(err != 0 ){
           printf("Can't join business customer thread.\n");
        }
    }
    
  
   
   //print out the average waiting times
    double waitall,wait0,wait1;
    if(overall_waiting_time0 == 0){
        
        wait0 = 0;
    }else{
        wait0 = overall_waiting_time0/elength;
    }
    if(overall_waiting_time1 == 0){
        wait1 = 0;
    }else{
        wait1 = overall_waiting_time1/blength;
    }
    if((overall_waiting_time1 == 0)&&(overall_waiting_time0 == 0)){
        waitall = 0;
    }else{
        waitall =
            (overall_waiting_time0+overall_waiting_time1)/count;
    }
    
    
   
    
    
    printf("The average waiting time for all customers in the system is: %.2f seconds. \n",waitall);
    printf("The average waiting time for all business-class customers is: %.2f seconds. \n",wait1);
    printf("The average waiting time for all economy-class customers is: %.2f seconds. \n",wait0);
    
    
    pthread_mutex_destroy(&start_time_mutex);
    pthread_mutex_destroy(&equeuelock);
    pthread_mutex_destroy(&bqueuelock);
    pthread_mutex_destroy(&lengthlock);
    pthread_mutex_destroy(&servedlock);
    pthread_mutex_destroy(&clerk1lock);
    pthread_mutex_destroy(&clerk2lock);
    pthread_mutex_destroy(&clerk3lock);
    pthread_mutex_destroy(&clerk4lock);

    
    pthread_cond_destroy(&convar0Empty);
    pthread_cond_destroy(&convar1Empty);
    pthread_cond_destroy(&convar1);
    pthread_cond_destroy(&convar2);
    pthread_cond_destroy(&convar3);
    pthread_cond_destroy(&convar4);
    
    sem_destroy(&mutex0);
    sem_destroy(&mutex1);

    return 0;
     
}


