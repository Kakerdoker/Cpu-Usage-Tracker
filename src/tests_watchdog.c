#include <string.h>

#include "../inc/watchdog.h"
#include "../inc/buffers.h"

#include "../inc/tests_watchdog.h"
#include "../inc/tests_basic.h"

//Test to see if watchdog still checks for responsiveness even if semEmpty == 0, which means that logger is waiting for new messages to log.
static void TEST_loggerBlockForUnresponsivness(void){
    updateBuffer[THREAD_AMOUNT-1] = time(NULL) - 3;
    myAssert(checkLoggerThread() == -1, "Logger was waiting for message but watchdog still checked to see if it was responsive. Did you sem_post(&messageBuffEmpty) anywhere in the test without sem_wait afterwards?");
}

//Checks if checkAllThreadResponses() returns appropriate value for every thread being responsive in the last 2 seconds.
static void TEST_twoSecondsResponse(void){
    sem_post(&messageBuffEmpty);//So logger doesn't get blocked

    for(int thread = 0; thread < THREAD_AMOUNT; thread++){
        updateBuffer[thread] = time(NULL) - 2;
    }
    myAssert(checkAllThreadResponses() == -1, "ERROR One of the threads being responsive returned an unresponsive value!");
    
    sem_wait(&messageBuffEmpty);
}

//Tests if checkAllThreadResponses() returns appropriate value for every thread being unresponsive for 3 seconds.
static void TEST_threeSecondsResponse(void){
    sem_post(&messageBuffEmpty);//So logger doesn't get blocked

    //Since checkAllThreadResponses() checks threads from 0 to THREAD_AMOUNT and returns the first thread that doesn't respond,
    //then the for loop below should check them in reverse order to make sure that every thread gets checked for not responding once, instead of the first thread getting checked THREAD_AMOUNT of times.
    for(int thread = THREAD_AMOUNT-1; thread >= 0; thread--){
        updateBuffer[thread] = time(NULL) - 3;
        myAssert(checkAllThreadResponses() == thread, "ERROR One of the threads being unresponsive for three seconds returned a responsive value!");
    }
    sem_wait(&messageBuffEmpty);
}

static void TEST_threadMessages(void){
    myAssert(strcmp(getMessageFromThread(0), "Reader thread stopped responding, closing program!") == 0, "ERROR got incorrect message from reader thread!");
    myAssert(strcmp(getMessageFromThread(1), "Analyzer thread stopped responding, closing program!") == 0, "ERROR got incorrect message from analyzer thread !");
    myAssert(strcmp(getMessageFromThread(2), "Printer thread stopped responding, closing program!") == 0, "ERROR got incorrect message from printer thread!");
    myAssert(strcmp(getMessageFromThread(999), "Unknown thread stopped responding, closing program!") == 0, "ERROR got incorrect message from unknown thread!");    
    myAssert(strcmp(getMessageFromThread(-999), "Unknown thread stopped responding, closing program!") == 0, "ERROR got incorrect message from unknown thread!");    
}

void TEST_watchdogMethods(void){
    //If logger is waiting for messages then watchdog shouldn't count logger as unresponsive.
    TEST_loggerBlockForUnresponsivness();
    TEST_twoSecondsResponse();
    TEST_threeSecondsResponse();
    TEST_threadMessages();
}
