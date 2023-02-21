#include <stdlib.h>
#include <stdio.h>

#include "../inc/buffers.h"

#include "../inc/tests_basic.h"

char *tempString1, *tempString2, *realString, *correctString, *testedString;

thrd_t logger, tester;

static void collectTestGarbage(void){
    free(correctString);
    free(testedString);
    free(tempString1);
    free(tempString2);
    free(realString);
}

void cleanUp(void){
    collectTestGarbage();
    collectBufferGarbage();
    destroySemaphores();
}

//Works like an assert but cleans up everything and prints a specified message before exiting
void myAssert(const int statement, const char* message){
    if(statement != 1){
        cleanUp();
        printf("TEST FAILED: %s\n", message);
        exit(1);
    }
    
}

void initializeTestVariables(void){
    correctString = malloc(sizeof(char)*128);
    testedString = malloc(sizeof(char)*128);
    tempString1 = malloc(sizeof(char)*128);
    tempString2 = malloc(sizeof(char)*128);
    realString = malloc(sizeof(char)*128);
}
