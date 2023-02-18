#ifndef BUFFERS_H
#define BUFFERS_H

#include <threads.h>
#include <semaphore.h>

#define THREAD_AMOUNT 4

struct CpuInfo{
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
};

extern mtx_t cpuInfoMutex, cpuUsageMutex, watchdogUpdateMutex, messageMutex;
extern sem_t messageBuffFull, messageBuffEmpty;

extern struct CpuInfo* currentCpuInfoBuffer;
extern struct CpuInfo* previousCpuInfoBuffer;
extern float* cpuUsageBuffer; 
extern time_t updateBuffer[THREAD_AMOUNT];

char* readMessage();
void writeMessage(char*);

// void writeToMessageBuffer(char*);
// char* readFromMessageBuffer();

void updateWatchdogBuffer();

void initializeSemaphores();
void destroySemaphores();

void initializeMutexes();
void destroyMutexes();

void allocateBufferMemory();
void collectBufferGarbage();

#endif