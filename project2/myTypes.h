#ifndef __myTypes__
#define __myTypes__
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

#define NUM_ARCHIVE_SLOTS 3
#define MAX_ENTRIES 5
#define ENTRY_SIZE 100

typedef struct
{
    char data[ENTRY_SIZE];
    time_t timeStamp;
    int pubID;
    int readCount;
} TopicNode;

typedef struct
{
    int numTopics;
    int toArchiver;
    int readArchiver;
} Archiver;

#define CIRCULAR 1
#if CIRCULAR

#define NUM_CIRCULAR_SLOTS (MAX_ENTRIES + 1)

typedef struct {
    TopicNode circularBuffer[NUM_CIRCULAR_SLOTS];
    size_t readIndex;
    size_t writeIndex;
    pthread_mutex_t mutex;
    int topicID;
    int totalSubscribers;

    int curArchiveIndex;
    TopicNode* nodeBuffer;
    Archiver archiver;
} TopicList;

#else
typedef struct
{
    std::list<TopicNode> list;
    pthread_mutex_t mutex;
    std::string* buffer;
    TopicNode* nodeBuffer;
    int topicID;
    int totalSubscribers;
    int curArchiveIndex;
    Archiver archiver;
} TopicList;
#endif

typedef enum {
    kUnknownChild = 0,
    kPublisherChild,
    kSubscriberChild
}ChildType;

typedef enum {
    kNoConnection = 0,
    kPublisherConnection,
    kSubscriberConnection
} ConnectionType;

typedef enum
{
    kReadTimeout = -1,
    kReadOK = 0,
    kReadEndOfPipe = 1
} ReadResult;

typedef struct
{
    int topicID;
    int numNodes;
    TopicNode *nodeBuffer;

} MessageContainer;

typedef struct
{
    ConnectionType connectionType; // Type of connection
    pid_t pid; // Process ID of child
    ChildType childType;    // Publisher or Subscriber
    int toChild; // File descriptor to send messages to child
    int fromChild; // FIle descriptor to receive messages from child
    pthread_t thread;               // The server side thread handling the child
    std::vector<int> *topics; // The topics for the connection.
    TopicList* topicLists;
    int numTopics;
    int pubSubID;
} ChildInfo;

#endif