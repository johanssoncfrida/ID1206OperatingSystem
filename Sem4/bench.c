#include "green.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#define RUNS 26

int flag = 0; 
volatile int count = 0; 
volatile int countP = 0;
green_cond_t filled,empty; 
green_mutex_t mutex; 
pthread_cond_t filled_p,empty_p; 
pthread_mutex_t mutex_p;

/* Producer / Consumer */
int volatile productions = 100;
int runs = 0;
void* producer(void* arg);
void* consumer(void* arg);

void* test_p_condv_mutex(void* arg);
void* p_producer(void* arg);
void* p_consumer(void* arg);

int main()
{
    
        green_t g0, g1; 
        int ag0 = 0; 
        int ag1 = 1; 

        pthread_t p0, p1;
        int ap0 = 0;
        int ap1 = 1; 


        
        
        
        green_cond_init(&filled);
        green_cond_init(&empty);
        green_mutex_init(&mutex);

        pthread_cond_init(&filled_p, NULL); 
        pthread_cond_init(&empty_p, NULL);
        pthread_mutex_init(&mutex_p, NULL); 

        for(int i = 0; i < RUNS; i++){
            double time_green = 0;
            double time_pthread = 0;

            count = 0;
            countP = 0;

            /* Start the timer (Green Threads) */
            // clock_t green_begin = clock();

            // green_create(&g0, producer, &ag0);
            // green_create(&g1, consumer, &ag1);
            // green_join(&g0, NULL);
            // green_join(&g1, NULL);

            // clock_t green_end = clock();
            // time_green = ((double)(green_end-green_begin))/ ((double)CLOCKS_PER_SEC/1000); //ms

            /* For pthread tests */
            /* Start the timer (Phreads) */
            clock_t pthread_begin = clock();

            pthread_create(&p0, NULL, p_producer, &ap0);
            pthread_create(&p1, NULL, p_consumer, &ap1);

            pthread_join(p0, NULL);
            pthread_join(p1, NULL); 
            
            double pthread_end = clock();
            time_pthread = ((double)(pthread_end-pthread_begin))/ ((double)CLOCKS_PER_SEC/1000); //ms
            
            printf("%d\t%f\t%f\n",productions, time_green, time_pthread);
            if(productions < 500)
                productions +=100;
            else{
                productions +=500;
            }

        }
    

    return 0; 
}


/* */
void* producer(void* arg){
    int id =*(int*)arg;
    int i;
    for (i = 0; i < productions; i++)
    {
        green_mutex_lock(&mutex);
        //waiting for the consumer before we produce more
        while (count == 1)
        {
            green_cond_wait(&empty, &mutex);
        }
        count = 1;
        green_cond_signal(&filled); 
        green_mutex_unlock(&mutex);
    }
    return (void*) 0; 
}

/**/
void* consumer(void* arg){
    int id =*(int*)arg;
    int i;
    for (i = 0; i < productions; i++)
    {
        green_mutex_lock(&mutex);
        //Waiting for the producer before we consume more
        while (count == 0)
        {
            green_cond_wait(&filled, &mutex);
        }
        count = 0;
        green_cond_signal(&empty); 
        green_mutex_unlock(&mutex);
     
    }
    return (void*) 0; 
}

/* */
void* p_producer(void* arg){
    int id =*(int*)arg;
    int i;
    for (i = 0; i < productions; i++)
    {
        pthread_mutex_lock(&mutex_p);
        while (count == 1)
        {
            pthread_cond_wait(&empty_p, &mutex_p);
        }
        //printf("produced nr %d by %d\n", i, id);
        count = 1;
        pthread_cond_signal(&filled_p); 
        pthread_mutex_unlock(&mutex_p);
    }
    return (void*) 0; 
}

/**/
void* p_consumer(void* arg){
    int id =*(int*)arg;
    int i;
    for (i = 0; i < productions; i++)
    {
        pthread_mutex_lock(&mutex_p);
        while (count == 0)
        {
            pthread_cond_wait(&filled_p, &mutex_p);
        }
        //printf("consumed nr %d by %d\n", i, id);
        count = 0; 
        pthread_cond_signal(&empty_p); 
        pthread_mutex_unlock(&mutex_p);

    }
    return (void*) 0; 
}