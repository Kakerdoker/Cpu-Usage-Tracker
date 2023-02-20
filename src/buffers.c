#include <stdlib.h>
#include <threads.h>
#include <semaphore.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "../inc/buffers.h"
#include "../inc/global.h"
#include "../inc/logger.h"


mtx_t cpuInfoMutex, cpuUsageMutex, watchdogUpdateMutex, messageMutex;
sem_t messageBuffEmpty, messageBuffFull;

struct CpuInfo* currentCpuInfoBuffer;
struct CpuInfo* previousCpuInfoBuffer;

double* cpuUsageBuffer;

static char** messageBuffer;
static int readerIndex = 0, writerIndex = 0;

//What: Reads message from a circular buffer
//What for: So it can be logged
char* readMessage(void){
    char* message = messageBuffer[readerIndex];
    readerIndex++;
    readerIndex %= 16;
    return message;
}

//What: Writes message to a circular buffer
//What for: So it can be read by the logger
void writeMessage(const char* message){
    strcpy(messageBuffer[writerIndex], message);
    writerIndex++;
    writerIndex %= 16;
}


time_t updateBuffer[THREAD_AMOUNT];

//What: Updates the given threadIndex inside updateBuffer with the current time
//What for: So the latest update can be read by the watchdog
void updateWatchdogBuffer(const int threadIndex){

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

void initializeSemaphores(void){
    sem_init(&messageBuffEmpty, 0, 0);
    sem_init(&messageBuffFull, 0, 15);
}

void destroySemaphores(void){
    sem_destroy(&messageBuffEmpty);
    sem_destroy(&messageBuffFull);
}

void initializeMutexes(void){
    mtx_init(&cpuInfoMutex, mtx_plain);
    mtx_init(&cpuUsageMutex, mtx_plain);
    mtx_init(&watchdogUpdateMutex, mtx_plain); 
    mtx_init(&messageMutex, mtx_plain);   
}

void destroyMutexes(void){
    mtx_destroy(&cpuInfoMutex);
    mtx_destroy(&cpuUsageMutex);
    mtx_destroy(&watchdogUpdateMutex);    
    mtx_destroy(&messageMutex);
}

void allocateBufferMemory(void){
    currentCpuInfoBuffer = malloc(sizeof(struct CpuInfo)*cpuCoreAmount);
    previousCpuInfoBuffer = malloc(sizeof(struct CpuInfo)*cpuCoreAmount);
    for(unsigned int core = 0; core < cpuCoreAmount; core++){
        //Fill both buffers with 0's to prevent junk data being read by the analyzer.
        currentCpuInfoBuffer[core] = previousCpuInfoBuffer[core] = (struct CpuInfo){0,0,0,0,0,0,0,0};
    }
    
    cpuUsageBuffer = malloc(sizeof(double)*cpuCoreAmount);
    
    messageBuffer = malloc(sizeof(char*)*16);
    for(int i = 0; i < 16; i++){
        messageBuffer[i] = malloc(sizeof(char)*256);
    }
}

void collectBufferGarbage(void){
    free(currentCpuInfoBuffer);
    free(previousCpuInfoBuffer);
    free(cpuUsageBuffer);

    for(int i = 0; i < 16; i++){
        free(messageBuffer[i]);
    }
    free(messageBuffer);
}
