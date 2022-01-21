#include <stdlib.h>
#include <stdio.h>
#include "dlmall.h"
#include <math.h>
#include <stdint.h>
#include <time.h>

#define LOOP 1000
#define BUFFER 100
#define SIZE 16
#define MAX 5000
#define MIN 8

int request(){
    double k = log(((double)MAX)/MIN);
    double r = ((double)(rand()%(int)(k*1000)))/10000;
    int size = (int)((double)MAX/exp(r));
    return size;
}




int main(int argc, char *argv[]){
    // int length = 0;
    // void *buffer[BUFFER];
    // for(int i = 0; i < BUFFER; i++){
    //      buffer[i] = NULL;
    // }
    // int fail = 0;
    // int counter = 0;
    // while(counter < 50){
    //     for(int j = 0; j < LOOP; j++){
    //         int index = rand() % BUFFER;
    //         if(buffer[index] != NULL){
    //             dfree(buffer[index]);
    //             buffer[index]=NULL;
    //         }else{
    //             size_t size = (size_t)request();
    //             int *memory;
    //             memory = dalloc(size);
    //             if(memory == NULL){  
    //                 fail++;  
    //             }else{
    //             buffer[index] = memory;
    //             *memory = 123;
    //             }  
    //         }
    //         // length = checkLength();
    //         // printf("%d\t%d\n", j, length);
  
            
    //     }
    //     int avg = checkAvgLength();
    //     printf("%d\t%d\n",counter, avg);
    //     // int len = checkLength();
    //     // printf("length: %d\n", len);
    //     counter ++;
    // }

    int length = 0;
    int counter = 0;
    double avg = 0;
    void *buffer[LOOP];
    clock_t startClock = clock();
    for(int i = 0; i < LOOP; i++){ //Allocate 16 bytes 1000 times
        buffer[i] = dalloc(16);
    }
    while(counter < LOOP){ //write 1000 times...
        
        for(int j = 0; j < LOOP; j++){ //write 1000 times...
            int *mem;
            mem = buffer[j];
            *mem = 123;   
        }

        counter ++;
    }
    clock_t endClock = clock();
    double total_t = (double)(endClock-startClock)/CLOCKS_PER_SEC;
    double ms = total_t * 1000;
    printf("Avg time: %f ms\n", ms); 
}