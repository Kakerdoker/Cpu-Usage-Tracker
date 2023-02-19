#ifndef LOGGER_H
#define LOGGER_H

void makeLogsFileIfDidntExist(char*);
int waitForNewMessagesToLog();
void initializeLogger(char*,char*);
void closeLogger();
void logMessage(char*);

#endif