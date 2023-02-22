#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "../inc/logger.h"
#include "../inc/buffers.h"
#include "../inc/threads.h"

static struct stat st = {0};
static char logDir[32];
static FILE* logFile;
static time_t currentTime;

//What: Waits until there's room in the buffer for a new message and writes it into the buffer
//What for: So it can be read by the logger
void logMessage(const char* message){
    sem_wait(&messageBuffFull);//Wait until messageBufferFull is not empty (empty means there are 15 messages waiting to be logged)
    mtx_lock(&messageMutex);//Lock messageBuffer for loggerLoop()
    
    writeMessage(message);

    mtx_unlock(&messageMutex);
    sem_post(&messageBuffEmpty);//Increment the messageBufferEmpty so it knows there is one more message to log
}

static void makeLogsFileIfDidntExist(const char* dir){
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0777);
    }
}

//What: Logs the time the program has started
//What for: So logs.txt is easier to read
static void logProgramStart(void){
    logFile = fopen(logDir, "a");
    //Log time of program starting.
    currentTime = time(NULL);
    fprintf(logFile, "PROGRAM STARTED --- %s",asctime(localtime(&currentTime)));
    fclose(logFile);
}


//What: Creates the directory and file where messages are logged and logs the time the program has started
void initializeLogger(const char* dir, const char* file){
    if(sprintf(logDir, "%s/%s", dir, file) >= 32){
        printf("initializeLogger: logDir exceeds 32 characters (logger.c)");
        exit(1);//todo: better exit
    }

    makeLogsFileIfDidntExist(dir);
    logProgramStart();
}

//What: Logs the time the program has closed
//What for: So logs.txt is easier to read / to know if the program completed execution until the end
void logClose(void){
    logFile = fopen(logDir, "a");

    currentTime = time(NULL);
    //Log time of program closing.
    fprintf(logFile, "PROGRAM CLOSED --- %s",asctime(localtime(&currentTime)));

    fclose(logFile);
}

//What: Reads message from message buffer and writes it to logs.txt
static void logMessageFromBuffer(void){
    char msg[128];
    strcpy(msg, readMessage());

    //Don't log the message if the first char is garbage
    //(Spamming SIGINT would make logger print char -32 for some reason, this is a roundabout way of fixing that issue)
    if((int)msg[0] < 0)
        return;

    currentTime = time(NULL);
    logFile = fopen(logDir, "a");
    //Read the last message in the message buffer and combine it with the current time, then print it to the log file.
    fprintf(logFile, "%s --- %s",msg, asctime(localtime(&currentTime)));
    fclose(logFile);
}

//What: Calls logMessageFromBuffer() every (args) microseconds and uses mutexes and semaphores to know when it can log the message
int loggerLoop(void* args){
    unsigned int delay = *(unsigned int *)args;
    while(loggerActive){

        usleep(delay);
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
