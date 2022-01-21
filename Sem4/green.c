/**
 * getcontext(ucontext_t *ucp): initializes the structure pointed at by ucp to the currently active context.
 * 
 * makecontext(ucontext_t *ucp): Modifies the context pointed to by ucp(which was obtained
 * from a call to getcontext()).Before invoking makecontext(), the caller must allocated a new
 * stack for this context and assign its address to ucp->uc_stack and define a successor context
 * and assign its address to ucp->uc_link
 * 
 * setcontext(const ucontext_t *ucp): Restores the user context pointed at by ucp
 * 
 * swapcontext(ucontext_t *oucp, const ucontext_t *ucp): Saves the current context in the structure pointed to by oucp, and
 * then activates the context pointed to by ucp.
 * 
*/

#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include <stdio.h>
#include "green.h"
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1
#define PERIOD 100
#define STACK_SIZE 4096

//main context
static ucontext_t main_cntx = {0};
//first green thread, global.
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, NULL, FALSE};
static green_t *running = &main_green;

//The Ready-Queue
static green_t *front = NULL;
static green_t *rear = NULL;

static void init() __attribute__((constructor)); //called when the program is loaded

//Variables for interrupts

static sigset_t block;

void timer_handler(int);


void init(){ 
    getcontext(&main_cntx); 

    sigemptyset(&block);
    sigaddset(&block, SIGVTALRM);

    struct sigaction act = {0};
    struct timeval interval;
    struct itimerval period;
    act.sa_handler = timer_handler;
    assert(sigaction(SIGVTALRM,&act,NULL) == 0);

    interval.tv_sec = 0;
    interval.tv_usec = PERIOD;
    period.it_interval = interval;
    period.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &period, NULL);
}



//implement enqueue end dequeue here!!

void enqueue(green_t *new){

    if(front == NULL){
        front = new;
        rear = new;

    }else{
        rear->next = new;
        rear = new;
    }
    return;
}

void dequeue(){
    if(front != NULL){
        green_t *toRun = front;
        if(front == rear){
            front = NULL;
            rear = NULL;
        }else{
            front = front->next;
        }
        running = toRun;
        running->next = NULL;
        
    }else{
        write(1, "#queue is empty\n",16);
        running =  NULL;
    }
}


void green_thread(){
    
    green_t *this = running;
    void *result = (*this->fun)(this->arg);

    sigprocmask(SIG_BLOCK, &block, NULL);
    //This if is needed otherwise segmentation fault. Last loop runs x times, 
    //ones per thread when running is null since we reached the end.
    if(this->join != NULL){ 
        this->join->next = NULL;
        enqueue(this->join); //place the joining/waiting thread in ready queue
    }
    
    this->retval = result; //Save the result from execution
    this->zombie = TRUE; //Become a zombie

    dequeue(); //dequeue next thread to run
    setcontext(running->context);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

int green_create(green_t *new, void *(*fun)(void*), void *arg){
    
    ucontext_t *cntx = (ucontext_t *)malloc(sizeof(ucontext_t));
    getcontext(cntx);

    void *stack = malloc(STACK_SIZE);
    cntx->uc_stack.ss_sp = stack;
    cntx->uc_stack.ss_size = STACK_SIZE;
    makecontext(cntx, green_thread, 0);

    new->context = cntx;
    new->fun = fun;
    new->arg = arg;
    new->next = NULL;
    new->join = NULL;
    new->retval = NULL;
    new->zombie = FALSE;
    sigprocmask(SIG_BLOCK, &block, NULL);

    enqueue(new);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_yield(){
    sigprocmask(SIG_BLOCK, &block, NULL);
    green_t *susp = running;
    susp->next = NULL;
    enqueue(susp);
    dequeue();
    swapcontext(susp->context, running->context);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_join(green_t *thread, void **res){
    
    if(!thread->zombie){
        sigprocmask(SIG_BLOCK, &block, NULL);
        green_t *susp = running;
        thread->join = susp;
        dequeue();
        swapcontext(susp->context, running->context);
        sigprocmask(SIG_UNBLOCK, &block, NULL);
    }
    res = thread->retval; //collect the result
    free(thread->context); //free context
    
    return 0;
}

/***********************************
 * Conditional variables implementation
 * 
 * 
************************************/
void green_cond_init(green_cond_t *cond){
    cond->head = cond->rear = NULL;
    
    return;
}

//Move the first suspended thread to the ready queue
void green_cond_signal(green_cond_t *cond){

    sigprocmask(SIG_BLOCK, &block, NULL);
    green_t *thread = cond->head;
    if(cond->head == cond->rear){
        cond->rear = cond->head = NULL;
        
    }else{
        cond->head = cond->head->next;
    }
    if(thread != NULL){
        thread->next = NULL;
        enqueue(thread);
    }

    sigprocmask(SIG_UNBLOCK, &block, NULL);

    return;
}

//Suspend the current thread on the condition
void green_cond_wait(green_cond_t *cond, green_mutex_t *mutex){
    
    sigprocmask(SIG_BLOCK, &block, NULL);
    
    green_t *runToSusp = running;
    //append suspended thread to list
    if(cond->rear != NULL){

        cond->rear->next = runToSusp;
        cond->rear = runToSusp;
        runToSusp->next = NULL;

    }else{ //if the condition susp-list is empty
        cond->rear = runToSusp;
        cond->head = runToSusp;
        runToSusp->next = NULL;
    }
    if(mutex != NULL){
        if(mutex->taken){
            mutex->taken = FALSE;
            if(mutex->head != NULL){
                green_t *thread = mutex->head;
                if(mutex->head == mutex->rear){
                    mutex->head = mutex->rear = NULL;
                    
                }else{
                    mutex->head = thread->next;
                }
                thread->next = NULL;
                enqueue(thread); 
            }
        } 
    }
    dequeue();
    
    swapcontext(runToSusp->context,running->context);

    if(mutex != NULL){
        //try to take the lock
        if(mutex->taken){
            runToSusp->next = NULL;
            //bad luck suspend
            if(mutex->rear != NULL){
                mutex->rear->next = runToSusp;
                mutex->rear = runToSusp;
                
            }else{
                mutex->head = runToSusp;
                mutex->rear = runToSusp;
            }
            dequeue();
            swapcontext(runToSusp->context, running->context);
        }else{
            //Take the lock
           mutex->taken = TRUE;
        }
    }

    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return;
}

/********************************************
 * For interrupts
 * Timer handler that schedules the next thread
 * 
*********************************************/
void timer_handler(int sig){
    write(1, "TIME\n",6);
    green_t *susp = running;
    susp->next = NULL;
    enqueue(susp);
    dequeue();
    swapcontext(susp->context, running->context);
}

/***************************************
 * Mutex implementation
 * 
****************************************/
int green_mutex_init(green_mutex_t *mutex){
    sigprocmask(SIG_BLOCK, &block, NULL);
    mutex->taken = FALSE;
    mutex->head = mutex->rear = NULL;
    
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_mutex_lock(green_mutex_t *mutex){
    sigprocmask(SIG_BLOCK, &block, NULL);
    green_t *susp = running;
    if(mutex->taken){
        
        //Suspend the running thread
        if(mutex->rear != NULL){
            mutex->rear->next = susp;
            mutex->rear = susp;
         
        }else{
            mutex->rear = susp;
            mutex->head = susp;
        }
        //find next thread
        dequeue();
        swapcontext(susp->context, running->context);
        
    }else{
        //take the lock
        mutex->taken = TRUE;
       
    }
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_mutex_unlock(green_mutex_t *mutex){
    sigprocmask(SIG_BLOCK, &block, NULL);
    if(mutex->head != NULL){
        
        green_t *thread = mutex->head;
        if(mutex->head == mutex->rear){
            mutex->head = mutex->rear = NULL;
            
        }else{
            mutex->head = thread->next;
        }
        
        thread->next = NULL;
        enqueue(thread); 
    }else{
        
        mutex->taken = FALSE;
    }

    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

