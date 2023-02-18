#ifndef LOGGER_H
#define LOGGER_H

int waitForNewMessagesToLog();
void initializeLogger();
void closeLogger();
void logMessage(char*);

#endif