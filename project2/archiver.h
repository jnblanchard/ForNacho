#include "myTypes.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <getopt.h>
#include <vector>
#include <stdarg.h>
#include <string>
#include <sys/select.h>
#include <pthread.h>
#include <assert.h>
#include <cstdlib>
#include <cstdbool>
#include <stdbool.h>
#include "threadUtilities.h"
#include <sstream>


void readBuffer(TopicNode* buffer);

void *archiver(void *archiveP);

ReadResult readBytes(int fd, char *buffer, size_t bufferSize);

void addToArchive(TopicList &topicList, const TopicNode *topicNode);

void flushArchive(TopicList &topicList);

void terminateArchiver(Archiver &archiver);




