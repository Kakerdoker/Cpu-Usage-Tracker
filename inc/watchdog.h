#ifndef WATCHDOG_H
#define WATCHDOG_H

//Used for testing
char* getMessageFromThread(int);
int checkResponseLongerThan2Seconds(int);
int checkAllThreadResponses();

//Used for other funcitons
int checkLastUpdate();

#endif