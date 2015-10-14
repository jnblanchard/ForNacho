#include "archiver.h"

#include <stdlib.h>

/*
 * The archiver thread process, pass in the archiver => reference in main from TopicLists.
 * Open a write file for each topic, this will hold data pushed to each individual topic.
 * Handle MessageContainers read from the Archiver's read pipe, be ready for a special MessageContatiner that will terminate the archiver.
 * Upon termination command, free the archiver, and close open topic write files.
 */
void *archiver(void *archiveP)
{
    Archiver archiver = *(Archiver*)archiveP;
    FILE *archiveFiles[archiver.numTopics];
//    fprintf(stderr, "Archiver: working directory = %s\n", getwd(NULL));
    for (int i = 0; i < archiver.numTopics; i++) {
        std::ostringstream filename;
        filename << "Topic " << i + 1;
        FILE* aFile = fopen(filename.str().c_str(), "w");
        archiveFiles[i] = aFile;
    }
    bool done = false;
    MessageContainer msgContainer;
    msgContainer.numNodes = archiver.numTopics;
    while (!done)
    {
        int result = readBytes(archiver.readArchiver, (char *) &msgContainer, sizeof(msgContainer));
        if (result == kReadOK)
        {
            int numNodes = msgContainer.numNodes;
            if (numNodes > 0) {
                int topicIndex = msgContainer.topicID - 1;
                for (int i = 0; i < numNodes; i++)
                {
                    TopicNode node = msgContainer.nodeBuffer[i];
                    size_t numWrites = fwrite(node.data, strlen(node.data), 1, archiveFiles[topicIndex]);
                    assert(numWrites == 1);
                    fputc('\n', archiveFiles[topicIndex]);
                }
                readBuffer(msgContainer.nodeBuffer);
                free(msgContainer.nodeBuffer);

                /* If we see a MsgContainer with numNodes == 0 then that tells the
                 * the archiver to quit.
                 */
            } else {
                done = true;
            }
        }
        else if (result == kReadEndOfPipe)
        {
            done = true;
        }
    }
    for ( int j = 0; j < archiver.numTopics; j++)
    {
        fclose(archiveFiles[j]);
    }
//    fprintf(stderr, "here - archiver || toArchiver FD - %i || readArchiver FD - %i\n", archiver.toArchiver, archiver.readArchiver);
    return nullptr;
}

ReadResult readBytes(int fd, char *buffer, size_t bufferSize)
{
    ReadResult result = kReadOK;
    size_t bytesLeft = bufferSize;
    ssize_t bytesRead = 0;
    char *inBuffer = buffer;

    while (bytesLeft)
    {
//        fprintf(stderr, "fd being read %i \nwith byte size: %zu\n", fd, bufferSize);
        bytesRead = read(fd, inBuffer, bytesLeft);
//        fprintf(stderr, "%s: archiver read %zd bytes.\n", __func__, bytesRead);

        if (bytesRead > 0)
        {
            bytesLeft -= bytesRead;
            inBuffer += bytesRead;

        } else if (bytesRead == 0)
        {
            result = kReadEndOfPipe;
            break;
        } else
        {
            fprintf(stderr, "read error: %s\n", strerror(errno));
            assert(bytesRead > 0);
        }

    }

    return result;
}

void readBuffer(TopicNode* buffer)
{
//    fprintf(stderr, "starting to read, first element: %s\n entire buffer data: ", buffer[0].data);
    for (int i = 0; i < NUM_ARCHIVE_SLOTS; i++)
    {
        fprintf(stderr, "%s\n", buffer[i].data);
    }
//    fprintf(stderr, "\nend read\n" );
}

/*
 * The TopicList holds a buffer of TopicNodes that will be pushed to the archiver upon filling up.
 * If the buffer is empty ( pointing to null ) allocate space for 5 TopicNodes. ( NUM_ARCHIVE_SLOTS = 5 )
 * Use curArchiveIndex to place arg[1] into the topicList buffer.
 * If the buffer is full, flush to the archiver.
 */
void addToArchive(TopicList &topicList, const TopicNode *topicNode)
{
//    fprintf(stderr, "entering add to archive\n");
    if ( topicList.nodeBuffer == nullptr )
    {
//        fprintf(stderr, "allocating space for an archive\n");
        topicList.curArchiveIndex = 0;
        topicList.nodeBuffer = (TopicNode*) malloc(sizeof(TopicNode) * NUM_ARCHIVE_SLOTS);
    }

    topicList.nodeBuffer[topicList.curArchiveIndex] = *topicNode;

    topicList.curArchiveIndex++;
    if ( topicList.curArchiveIndex == NUM_ARCHIVE_SLOTS )
    {
//        fprintf(stderr, "no room for more data\n");
        flushArchive(topicList);
    }
}

/* 
 * Flush the arg[0] TopicList's nodeBuffer and pass to the archiver through a MessageContatiner.
 * Push all of the archived nodes to the archiver.
 * Point the arg[0] TopicList's nodebuffer to null.
 */
void flushArchive(TopicList &topicList)
{
    Archiver archiver = topicList.archiver;
    MessageContainer msgContainer;
    msgContainer.nodeBuffer = topicList.nodeBuffer;
    msgContainer.numNodes = topicList.curArchiveIndex;
    msgContainer.topicID = topicList.topicID;
    if (msgContainer.numNodes > 0) {
        fprintf(stderr, "Writing %d nodes for topic %d to archive\n", msgContainer.numNodes, topicList.topicID);
        ssize_t writeResults = write(archiver.toArchiver, &msgContainer, sizeof(msgContainer));
        assert(writeResults == sizeof(msgContainer));
        topicList.nodeBuffer = nullptr;
    }

}

/* Write a special value to the archiver to tell it
 * that it is done.
 */
void terminateArchiver(Archiver &archiver)
{
    MessageContainer msgContainer;
    msgContainer.topicID = 0;
    msgContainer.numNodes = 0;
    msgContainer.nodeBuffer = nullptr;
    ssize_t writeResults = write(archiver.toArchiver, &msgContainer, sizeof(msgContainer));
    assert(writeResults == sizeof(msgContainer));
    
}
