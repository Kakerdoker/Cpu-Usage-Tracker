#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

#include "../inc/logger.h"
#include "../inc/buffers.h"
#include "../inc/threads.h"

#include "../inc/tests_logger.h"
#include "../inc/tests_basic.h"

static char logF[20] = "testLogs/logs.txt";

static void TEST_initializeLogger(void){
    //Delete files
    remove("testLogs/logs.txt");
    rmdir("testLogs");

    initializeLogger("testLogs","logs.txt");//Create again the just deleted files
    myAssert(access(logF, F_OK) == 0, "initializeLogger() hasn't sucesfully created the logs.txt file");//Check if they exist

    //Read first 2 strings of the newly created file and put them inside testedString
    FILE *loggerFile = fopen(logF, "r");
    fscanf(loggerFile, "%s %s %*s %*s %*s %*s %*s %*s", tempString1, tempString2);    

    //Make sure combined string doesn't go over allocated size of 128
    myAssert(sprintf(testedString, "%s %s",tempString1, tempString2) < 128, "testedString too big in TEST_initializeLogger()");
    fclose(loggerFile);

    //Compare read string with what should be in logs.txt
    myAssert(strcmp("PROGRAM STARTED",testedString) == 0, "initializeLogger() didn't log the message PROGRAM STARTED");
}

//Asks the logger to print 40 messages faster than it can log.
static void spamFourtyLogMessages(void){
    //Buffer size for messages is 16, so 40 will be enough to test if it leaks/overwrites/ignores messages.
    for(int i = 0; i < 40; i++){
        //Make sure combined string doesn't go over allocated size of 128
        myAssert(sprintf(testedString, "testmessage %i", i) < 128, "testedString too big in spamTwentyLogMessages()");
        logMessage(testedString);
    }
}

//Exits from the logger without creating memory leaks and makes sure it isn't stuck on a semaphore
static void exitLogger(void){
    loggerActive = 0;
    sem_post(&messageBuffEmpty);
}

static int putMessagesIntoLogger(void* args){
    (void)args;//To stop unused variable warning

    spamFourtyLogMessages();
    sleep(1);//Give logger one second to process the messages
    
    exitLogger();
    thrd_exit(0);
}

//Tests if the spammed variables were logged correctly and in order
static void TEST_readMessagesFromLogger(void){

    FILE *loggerFile = fopen(logF, "r");

    //Skip first line (program started message)
    fscanf(loggerFile, "%*s %*s %*s %*s %*s %*s %*s %*s");

    //Go through all the spammed messages
    for(int i = 0; i < 40; i++){
        fscanf(loggerFile, "%s %s %*s %*s %*s %*s %*s %*s", tempString1, tempString2);

        //Make sure combined strings don't go over allocated size of 128
        myAssert(sprintf(testedString, "%s %s",tempString1, tempString2) < 128, "testedString too big in TEST_readMessagesFromLogger()");
        myAssert(sprintf(realString, "testmessage %i", i) < 128, "realString too big in TEST_readMessagesFromLogger()");

        //Check if the message was logged correctly, if it wasn't then break so we can close the file, after which we assert the untrue statement.
        if(strcmp(realString,testedString) != 0){
            break;
        }
    }
    fclose(loggerFile);
    myAssert(strcmp(realString,testedString) == 0, "Logger didn't log a spammed message correctly.");
}

//Asynchronously ask 40 messages to be logged while also logging these messages at the same time
static void TEST_overflowingMessageBuffer(void){
    
    unsigned int delay = 1000;//Make the logger wait only 1000 microseconds between logging messages, if this value changes then the sleep(1) inside putMessagesIntoLogger() needs to be adjusted properly
    thrd_create(&tester, putMessagesIntoLogger, NULL);//Function to ask for the messages to be logged
    thrd_create(&logger, loggerLoop, &delay);//Function from logger.c to be tested
   
    thrd_join(tester, NULL);
    thrd_join(logger, NULL);
    
    //After logger logged the messages, check if they were correct.
    TEST_readMessagesFromLogger();
}

//Check if logger correctly logs that the program has closed
static void TEST_closing(void){
    //Call the function we are testing
    logClose();
    
    FILE *loggerFile = fopen(logF, "r");
    //Skip first 41 lines
    for(int i = 0; i < 41; i++){
        fscanf(loggerFile, "%*s %*s %*s %*s %*s %*s %*s %*s");
    }
    fscanf(loggerFile, "%s %s %*s %*s %*s %*s %*s %*s", tempString1, tempString2);   
    fclose(loggerFile);

    //Make sure combined string doesn't go over allocated size of 128
    myAssert(sprintf(testedString, "%s %s",tempString1, tempString2) < 128, "testedString too big in TEST_closing()");
    
    myAssert(strcmp(testedString,"PROGRAM CLOSED") == 0, "logClose() didn't log the message \"PROGRAM CLOSED\"");
}


void TEST_loggerMethods(void){
    TEST_initializeLogger();
    TEST_overflowingMessageBuffer();
    TEST_closing();
}
