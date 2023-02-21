#ifndef TESTS_BASIC_H
#define TESTS_BASIC_H

#include <threads.h>

//String used by multiple tests
extern char *tempString1, *tempString2, *realString, *correctString, *testedString;

//Thread used by multiple tests
extern thrd_t logger, tester;

//Cleans necessary variables used in tests
void cleanUp(void);

//Stops the program and prints provided message if given statement is false.
void myAssert(const int, const char*);

//Initialzies necessary variables for tests to be performed.
void initializeTestVariables(void);

#endif
