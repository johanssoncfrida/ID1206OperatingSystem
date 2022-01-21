#include <stdio.h>
#include "green.h"
#include <unistd.h>
#include <pthread.h>

//For conditional variables
int flag = 0;
green_cond_t cond;
pthread_cond_t cond_p;


//For mutex
green_mutex_t mutex;
int globalcounter = 0;
pthread_mutex_t p_mutex;

/* Producer / Consumer */
#define MAX 10
int buffer[MAX];
int fill_ptr = 0; 
int use_ptr = 0;
int loops = 1000;
green_cond_t empty;
green_cond_t fill; 

int count = 0; 


//Test yield

void *test_yield(void *arg){
    int i = *(int*)arg;
    int loop = 1000;

    while(loop > 0){
        printf("Thread %d: %d\n", i, loop);
        green_yield();
        loop--;
    }
}

void testGreenYield(){
    green_t g0,g1;
    int a0 = 0;
    int a1 = 1;
    
    green_create(&g0, test_yield, &a0);
    green_create(&g1, test_yield, &a1);

    green_join(&g0, NULL);
    green_join(&g1, NULL);

    printf("done\n");
}



/***************************************
 * Test conditional variables
 * 
****************************************/
void *test_cond(void *arg){
    usleep(50000);
    int id = *(int*)arg;
    int loop = 10000;
    
    while(loop > 0){
        printf("thread %d: %d\n", id, loop);
        if(flag == id){
            loop--;
            flag = (id+1) % 2;
            printf("before signal\n");
            green_cond_signal(&cond);
            printf("after signal\n");
        }else{
           
            printf("before wait id: %d\n", id);
            green_cond_wait(&cond, NULL);
            printf("after wait id: %d\n", id);
            
        }
        
    }

}

void *test_cond_p(void *arg){
    int id = *(int*)arg;
    int loop = 1000;
    
    while(loop > 0){
        printf("thread %d: %d\n", id, loop);
        if(flag == id){
            
            loop--;
            flag = (id+1) % 2;
            pthread_cond_signal(&cond_p);
        }else{

            pthread_cond_wait(&cond_p, &p_mutex);
        }
        
    }

}

void runTestCond(){
    green_t g0,g1;

    int a0 = 0;
    int a1 = 1;

    
    green_cond_init(&cond);
    green_create(&g0, test_cond, &a0);
    green_create(&g1, test_cond, &a1);

    green_join(&g0, NULL);
    green_join(&g1, NULL);

    printf("done\n");
}

void runTestCond_p(){
    pthread_t p0,p1;

    int a0 = 0;
    int a1 = 1;

    
    pthread_cond_init(&cond_p,NULL);
    pthread_mutex_init(&p_mutex, NULL);
    pthread_create(&p0, NULL, test_cond_p, &a0);
    pthread_create(&p1, NULL, test_cond_p, &a1);

    pthread_join(p0, NULL);
    pthread_join(p1, NULL);

    printf("done\n");
}


/***********************************
 * Test Mutex
 * A simple test to verify that mutex works.
 * 
***********************************/
void *test_mutex(void *arg){
    int id = *(int*)arg;
    int loop = 1000;
    
    while(loop > 0){
        
        loop--;
        green_mutex_lock(&mutex);
        globalcounter ++;
        green_mutex_unlock(&mutex);
        
    }
    printf("Counter should be 2000\nCounter: %d\n", globalcounter);
    globalcounter = 0;
    return (void*) 0;
}

void *test_pmutex(void *arg){
    int id = *(int*)arg;
    int loop = 1000;
    while(loop > 0){
        
        loop--;
        pthread_mutex_lock(&p_mutex);
        globalcounter ++;
        pthread_mutex_unlock(&p_mutex);
        
    }
    printf("Counter should be 2000\nCounter: %d\n", globalcounter);
    globalcounter = 0;
    return (void*) 0;
}

void runTestMutex(){
    green_t g0,g1;

    int a0 = 0;
    int a1 = 1;

    
    green_mutex_init(&mutex);
    green_create(&g0, test_mutex, &a0);
    green_create(&g1, test_mutex, &a1);

    green_join(&g0, NULL);
    green_join(&g1, NULL);

    printf("done\n");
}



void runTestMutex_p(){
    pthread_t p0,p1;

    int a0 = 0;
    int a1 = 1;

    
    pthread_mutex_init(&p_mutex, NULL);
    pthread_create(&p0, NULL, test_pmutex, &a0);
    pthread_create(&p1, NULL, test_pmutex, &a1);

    pthread_join(p0, NULL);
    pthread_join(p1, NULL);

    printf("done\n");
}

/***********************************
 * Final touch
 * 
 * We could get an interrupt right after we release lock and before
 * we call wait. 
 * 
************************************/


void* test_cond_mutex(void* arg)
{
    int id = *(int*) arg; 
    int loop = 1000; 

    while (loop > 0)
    {
        //write("thread %d: %d\n", id, loop);
        green_mutex_lock(&mutex);
        while (flag != id)
        {
            green_cond_wait(&cond, &mutex);
        }
        flag = (id + 1) % 2;
        green_cond_signal(&cond);
        green_mutex_unlock(&mutex);
        loop--;
    }
    return (void*) 0;
}

void runTestCondMutex(){
    green_t g0,g1;

    int a0 = 0;
    int a1 = 1;
    flag = 0;
    green_cond_init(&cond);
    green_mutex_init(&mutex);
    green_create(&g0, test_cond_mutex, &a0);
    green_create(&g1, test_cond_mutex, &a1);

    green_join(&g0, NULL);
    green_join(&g1, NULL);

    printf("done\n");
}



int main(){
    //testGreenYield();
    //runTestCond();
    // int outerloop = 10;
    // while(outerloop > 0){
    //     testGreenYield();
    //     outerloop --;
    // }
    // while(outerloop > 0){
    //     testGreenYieldManyLoops();
    //     outerloop --;
    // }
    // while(outerloop > 0){
    //     runTestCond();
    //     outerloop --;
    // }
    // while(outerloop > 0){
    //     runTestCond_p();
    //     outerloop --;
    // }
    // while(outerloop > 0){
    //     runTestMutex();
    //     outerloop --;
    // }
    // while(outerloop > 0){
    //     runTestMutex_p();
    //     outerloop --;
    // }

    // while(outerloop > 0){
        runTestCondMutex();
    //      outerloop --;
    //  }
    return 0;
}