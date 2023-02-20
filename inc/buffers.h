#ifndef BUFFERS_H
#define BUFFERS_H

#include <threads.h>
#include <semaphore.h>

//Amount of threads used by program (excluding watchdog)
#define THREAD_AMOUNT 4

//Useful information from proc/stat
struct CpuInfo{
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
};

extern mtx_t cpuInfoMutex;
extern mtx_t cpuUsageMutex;
extern mtx_t watchdogUpdateMutex;
extern mtx_t messageMutex;
extern sem_t messageBuffFull;
extern sem_t messageBuffEmpty;

//Buffer holding the information last read from proc/stat
extern struct CpuInfo* currentCpuInfoBuffer;

//Buffer holding the information read before the last read from proc/stat
extern struct CpuInfo* previousCpuInfoBuffer;

//Buffer holding information of % cpu usage for every active core
extern double* cpuUsageBuffer; 

//Buffer holding information of the last time every thread updated it's activity (excluding watchdog)
extern time_t updateBuffer[THREAD_AMOUNT];

//Reads message from a circular buffer
char* readMessage(void);

//Writes messages to a circular buffer
void writeMessage(const char*);

//Updates the watchdog buffer with the current time
void updateWatchdogBuffer(const int);

//Initializes all the semaphores
void initializeSemaphores(void);

//Destroys all the semaphores
void destroySemaphores(void);

//Initializes all the mutexes
void initializeMutexes(void);

//Destroys all the mutexes
void destroyMutexes(void);

//Allocates memory for all the buffers
void allocateBufferMemory(void);

//Frees memory from all the buffers
void collectBufferGarbage(void);

#endif
