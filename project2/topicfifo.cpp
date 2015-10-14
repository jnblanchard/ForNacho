#include "topicfifo.h"
#include <pthread.h>

size_t succ(size_t index);
bool isEmpty(const TopicList &topicList);
bool isFull(const TopicList &topicList);

/*
 * Create a TopicList for every topic
 * Initialize values inside the list, init key (mutex) for shared list resources.
 * Pass in arg[1] archiver initialized in main, hold reference of the archiver on each TopicList.
 */
TopicList createTopicList(int topicID, Archiver archiver)
{
    TopicList topicList;
    topicList.topicID = topicID;
    topicList.totalSubscribers = 0;
    topicList.curArchiveIndex = 0;

    topicList.readIndex = 0;
    topicList.writeIndex = 0;

    topicList.nodeBuffer = nullptr;
    topicList.archiver = archiver;
    int result = pthread_mutex_init(&topicList.mutex, NULL);
    assert(result == 0);

    return topicList;
}

/*
 * Thread safe queue operation for adding an entry to the TopicList using the key (mutex)
 * Lock the TopicList
 * Circular buffer, check for !full using helper function.
 * Create a node with the current time, the ID of the publisher, and data entry
 * Unlock the TopicList
 */
bool enqueue(TopicList &topicList, int publisherID, const char *data)
{
    bool success = false;
    int lockResult = pthread_mutex_lock(&topicList.mutex);
    assert(lockResult == 0);


    if (!isFull(topicList))
    {
        TopicNode &node = topicList.circularBuffer[topicList.writeIndex];

        time_t  timev;
        time(&timev);
        node.timeStamp = timev;

        node.pubID = publisherID;
        node.readCount = 0;
        size_t size = sizeof(node.data);
        strlcpy(node.data, data, size);

        topicList.writeIndex = succ(topicList.writeIndex);
        success = true;
    }

    pthread_mutex_unlock(&topicList.mutex);
    //fprintf(stderr, "enqueue result: %d\n", success);
    return success;
}

size_t willDequeue(TopicList &topicList) // called by each subscriber
{
    size_t readerIndex;

    int lockResult = pthread_mutex_lock(&topicList.mutex);
    assert(lockResult == 0);

    readerIndex = topicList.readIndex;
    topicList.totalSubscribers++; // number of subscriber incremented

    pthread_mutex_unlock(&topicList.mutex);

    return readerIndex;

}

/*
 * Thread safe queue operation for deleting an entry to the TopicList using the key (mutex)
 * Lock the TopicList
 * Circular buffer, check that the queue is not empty.
 * A subscriber has read the entry, increase TopicNode readcount and increase the reader index
 * If every subscriber has read the entry then add the node to the archiver.
 * Unlock the TopicList
 */
bool dequeue(TopicList &topicList,  std::string &data, size_t &readerIndex)
{
    bool success = false;
    int lockResult = pthread_mutex_lock(&topicList.mutex);
    assert(lockResult == 0);

    if (readerIndex != topicList.writeIndex) {

        TopicNode &node = topicList.circularBuffer[readerIndex];
        node.readCount++;
        data = node.data;

        readerIndex = succ(readerIndex);
        success = true; // Retrieved the node successfully

        if (node.readCount == topicList.totalSubscribers)
        {
            addToArchive(topicList, &node); // Every subscriber has read the node, archive the node.
            topicList.readIndex = succ(topicList.readIndex);
        }
    }

    pthread_mutex_unlock(&topicList.mutex);
    //fprintf(stderr, "dequeue result: %d\n", success);
    return success;
}


/* Write all of the remaining nodes from the topicList
 * to the archive. Call this when shutting down the server.
 */
void writeAllToArchive(TopicList &topicList)
{
    int result = pthread_mutex_lock(&topicList.mutex);
    assert(result == 0);

    while (!isEmpty(topicList)) {
        TopicNode &node = topicList.circularBuffer[topicList.readIndex];
        addToArchive(topicList, &node);
        topicList.readIndex = succ(topicList.readIndex);
    }

    flushArchive(topicList);

    pthread_mutex_unlock(&topicList.mutex);
}

/*
 * Only use this function when the caller has the TopicList mutex locked.
 */
size_t succ(size_t index)
{
    return (index + 1) % NUM_CIRCULAR_SLOTS;
}

/*
 * Only use this function when the caller has the TopicList mutex locked.
 */
bool isEmpty(const TopicList &topicList)
{
    return topicList.readIndex == topicList.writeIndex;
}

/*
 * Only use this function when the caller has the TopicList mutex locked.
 */
bool isFull(const TopicList &topicList)
{
    return succ(topicList.writeIndex) == topicList.readIndex;
}