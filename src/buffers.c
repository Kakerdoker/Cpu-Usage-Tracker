#include <stdlib.h>
#include <threads.h>
#include <semaphore.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "../inc/global.h"
#include "../inc/logger.h"

struct CpuInfo{
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
};

mtx_t cpuInfoMutex, cpuUsageMutex, watchdogUpdateMutex, messageMutex;
sem_t messageBuffEmpty, messageBuffFull;

struct CpuInfo* currentCpuInfoBuffer;
struct CpuInfo* previousCpuInfoBuffer;

float* cpuUsageBuffer;

/*
    Circular buffer for messages.
    Use semaphores before using readMessage() and writeMessage() so new messages don't overwrite old messages that haven't been already read.
*/
char** messageBuffer;
int readerIndex = 0, writerIndex = 0;
char* readMessage(){
    char* message = messageBuffer[readerIndex];
    readerIndex++;
    readerIndex %= 16;
    return message;
}

void writeMessage(char* message){
    strcpy(messageBuffer[writerIndex], message);
    writerIndex++;
    writerIndex %= 16;
}

//Four updates for four different threads (excluding watchdog). To make sure that if one thread doesn't work, the others won't keep updating the watchdog.
time_t updateBuffer[4];
void updateWatchdogBuffer(int threadIndex){

    mtx_lock(&watchdogUpdateMutex);//Lock the watchdog mutex so checkLastUpdate() doesn't check for updates while changing them.

    if(threadIndex < 4 && threadIndex >= 0){
        updateBuffer[threadIndex] = time(NULL);
    }
    else{
        logMessage("Tried to update index out of range (buffers.c)");
        exit(1);//todo:normal exit
    }
    mtx_unlock(&watchdogUpdateMutex);

}

void initializeSemaphores(){
    sem_init(&messageBuffEmpty, 0, 0);
    sem_init(&messageBuffFull, 0, 15);
}

void destroySemaphores(){
    sem_destroy(&messageBuffEmpty);
    sem_destroy(&messageBuffFull);
}

void initializeMutexes(){
    mtx_init(&cpuInfoMutex, mtx_plain);
    mtx_init(&cpuUsageMutex, mtx_plain);
    mtx_init(&watchdogUpdateMutex, mtx_plain); 
    mtx_init(&messageMutex, mtx_plain);   
}

void destroyMutexes(){
    mtx_destroy(&cpuInfoMutex);
    mtx_destroy(&cpuUsageMutex);
    mtx_destroy(&watchdogUpdateMutex);    
    mtx_destroy(&messageMutex);
}

void allocateBufferMemory(){
    currentCpuInfoBuffer = malloc(sizeof(struct CpuInfo)*cpuCoreAmount);
    previousCpuInfoBuffer = malloc(sizeof(struct CpuInfo)*cpuCoreAmount);
    for(int i = 0; i < cpuCoreAmount; i++){
        currentCpuInfoBuffer[i] = previousCpuInfoBuffer[i] = (struct CpuInfo){0,0,0,0,0,0,0,0};
    }
    
    cpuUsageBuffer = malloc(sizeof(float)*cpuCoreAmount);
    
    messageBuffer = malloc(sizeof(char*)*16);
    for(int i = 0; i < 16; i++){
        messageBuffer[i] = malloc(sizeof(char)*256);
    }
}

void collectBufferGarbage(){
    free(currentCpuInfoBuffer);
    free(previousCpuInfoBuffer);
    free(cpuUsageBuffer);

    for(int i = 0; i < 16; i++){
        free(messageBuffer[i]);
    }
    free(messageBuffer);
}