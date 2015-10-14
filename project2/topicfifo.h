#ifndef __topicfifo__
#define __topicfifo__
#include "archiver.h"
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
#include <list>
#include <time.h>
#include "myTypes.h"

bool enqueue(TopicList &topicList, int publisherID, const char *data);

void writeAllToArchive(TopicList &topicList);

TopicList createTopicList(int topicID, Archiver archiver);

bool dequeue(TopicList &topicList,  std::string &data, size_t &readerIndex);

size_t willDequeue(TopicList &topicList); // called by each subscriber

#endif