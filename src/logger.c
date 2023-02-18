#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "../inc/buffers.h"
#include "../inc/threads.h"

struct stat st = {0};
char logDir[16] = "logs/logs.txt";
FILE* logFile;
time_t currentTime;

void makeLogsFileIfDidntExist(){
    if (stat("logs", &st) == -1) {
        mkdir("logs", 0777);
    }
}

void logProgramStart(){
    logFile = fopen(logDir, "a");
    //Log time of program starting.
    currentTime = time(NULL);
    fprintf(logFile, "PROGRAM STARTED --- %s",asctime(localtime(&currentTime)));
    fclose(logFile);
}

void initializeLogger(){
    makeLogsFileIfDidntExist();
    logProgramStart();
}

void closeLogger(){
    logFile = fopen(logDir, "a");

    currentTime = time(NULL);
    //Log time of program closing.
    fprintf(logFile, "PROGRAM CLOSED --- %s",asctime(localtime(&currentTime)));

    fclose(logFile);
}

void logMessageFromBuffer(){
    currentTime = time(NULL);
    logFile = fopen(logDir, "a");
    //Read the last message in the message buffer and combine it with the current time, then print it to the log file.
    fprintf(logFile, "%s --- %s",readMessage(), asctime(localtime(&currentTime)));
    fclose(logFile);
}

int waitForNewMessagesToLog(){
    while(1){

        usleep(500000);//Check every half a second.
        sem_wait(&messageBuffEmpty);//Wait until buffer is not empty (until it gets at least one message)
        mtx_lock(&messageMutex);//Lock messageBuffer for logMessage()

        //Update watchdog after sem_wait so it gets updated right away, making it so that watchdog doesn't terminate the program for not responding because it was waiting for a messsage.
        updateWatchdogBuffer(3);
        logMessageFromBuffer();

        mtx_unlock(&messageMutex);
        sem_post(&messageBuffFull);//Increment the messageBufferFull so it knows there is one less message in the buffer.
    }
    thrd_exit(0);
}

void logMessage(char* message){
    sem_wait(&messageBuffFull);//Wait until messageBufferFull is not empty (until there are 15 messages waiting to be logged)
    mtx_lock(&messageMutex);//Lock messageBuffer for waitForNewMessagesToLog()
    
    writeMessage(message);

    mtx_unlock(&messageMutex);
    sem_post(&messageBuffEmpty);//Increment the messageBufferEmpty so it knows there is one more message to log
}