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
#include "publisherProcess.h"
#include "childUtilities.h"
#include "subscriber.h"
#include "threadUtilities.h"
#include "subServerThread.h"
#include "pubServerThread.h"
#include "myTypes.h"
#include "archiver.h"
#include "topicfifo.h"


int MAXLINE = 100;
static const int kNumTopics = 500;
TopicList topic[kNumTopics];
std::vector<ChildInfo> children;
bool shuttingDown = false;

void writeFromServer (int fromFD)
{
    for (std::vector<ChildInfo>::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        ChildInfo temp = *iter;
        if( fromFD == temp.fromChild )
        {
            writeStr(temp.toChild, "accept");
        }
    }
}

typedef void (*EntryPoint)(void);

int findChildWithFD(std::vector<ChildInfo> children, int fd)
{

    int i = 0;
    for (std::vector<ChildInfo>::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        ChildInfo temp = *iter;
        if(fd == temp.fromChild || fd == temp.toChild )
        {
            return i;
        }
        i++;
    }
    return -1;
}


ChildInfo create_pipes_spawn_child (const char *childPath, const char *childName, TopicList *topicList, ChildType childType, int numTopics, int proxyID)
{
    ChildInfo childInfo;
    childInfo.pid = 0;
    childInfo.connectionType = kNoConnection;
    childInfo.toChild = -1;
    childInfo.fromChild = -1;
    childInfo.childType = childType;
    childInfo.topicLists = topicList;
    childInfo.numTopics = numTopics;
    childInfo.pubSubID = proxyID;
    childInfo.topics = new std::vector<int>;
    pid_t pid;
    int toChildPipe[2];
    int fromChildPipe[2];

    std::ostringstream idStr;
    idStr << proxyID;
    const char *idCStr = idStr.str().c_str();

    std::ostringstream numTopicsStr;
    numTopicsStr << numTopics;
    const char *numTopicCStr = numTopicsStr.str().c_str();

    if(pipe(toChildPipe) < 0 || pipe(fromChildPipe) < 0)
    {
        fprintf(stderr, "Could not create the child pipes.\n");
        //        std::cerr << "Could not create the child pipes." << std::endl;
        exit(1);
    }
    fprintf(stderr, "(%d, %d), (%d, %d)\n", toChildPipe[0], toChildPipe[1], fromChildPipe[0], fromChildPipe[1]);

    if((pid = fork()) < 0)
    {
        fprintf(stderr, "Could not fork.\n");
        //        std::cerr << "Could not fork." << std::endl;
        exit(1);
    }

    else if (pid > 0)
    {
        childInfo.pid = pid;

        /* Close the ends of the pipes that will be used by the child. */
        close(toChildPipe[STDIN_FILENO]);
        close(fromChildPipe[STDOUT_FILENO]);

        /* Remember the ends of the pipes that will be used by the parent. */
        childInfo.toChild = toChildPipe[STDOUT_FILENO];
        childInfo.fromChild = fromChildPipe[STDIN_FILENO];

    } else {

        /* We use stdin and stdout to communicate with the server.
         * We moved the real stout to fd 3.
         */
        dup2(STDOUT_FILENO, 3);

        /* Close the ends of the pipes that will be used by the parent. */
        close(toChildPipe[STDOUT_FILENO]);
        close(fromChildPipe[STDIN_FILENO]);

        /* Duplicate the ends of the pipes that will be used by the child on to the STDIN and STDOUT
         file descriptors. */
        dup2(toChildPipe[STDIN_FILENO], STDIN_FILENO);
        dup2(fromChildPipe[STDOUT_FILENO], STDOUT_FILENO);

        execl(childPath, childName, idCStr, numTopicCStr, NULL);

        /* We don't want the child to return. */
        exit(1);
    }

    //	std::cout << childInfo.pid << " " << childInfo.toChild << " " << childInfo.fromChild << std::endl;


    return childInfo;
}

bool handleCommand(ChildInfo &childInfo)
{
    bool endOfFile = false;

    std::string command;
    read_from_pipe(childInfo.fromChild, command);

    fprintf(stderr, "%s: command:%s\n", __func__, command.c_str());

    if (!command.compare("pub connect")) {

        writeFromServer(childInfo.fromChild);

        /* Setting the connectioType prevents the main server loop from including this child in the select(). */
        childInfo.connectionType = kPublisherConnection;

        /* Start a thread to handle the publisher child.
         */
        //        std::cerr << "Before pub thread creation" << std::endl;
        int threadSuccess = pthread_create(&childInfo.thread, NULL, pubServer, &childInfo);
        assert(threadSuccess == 0);


    } else if (!command.compare("sub connect")) {
        writeFromServer(childInfo.fromChild);

        childInfo.connectionType = kSubscriberConnection;

        /* Start a thread to handle the subscriber child.
         */
        pthread_t thread;
        int threadSuccess = pthread_create(&thread, NULL, subServer, &childInfo);
        assert(threadSuccess == 0);


    } else if (!command.compare("end")) {

        writeFromServer(childInfo.fromChild);

        /* Start a thread to handle the publisher child.
         */
        pthread_t thread;
        int threadSuccess = pthread_create(&thread, NULL, subServer, &childInfo);
        assert(threadSuccess == 0);

    } else if (!command.compare("terminate")) {

        //        std::cerr << "terminate inside compare" << std::endl;
        writeStr(childInfo.toChild, "terminate");

        close(childInfo.toChild);
        childInfo.toChild = -1;

        close(childInfo.fromChild);
        childInfo.fromChild = -1;


    } else if (!command.compare("")) {
        // End of file (or the child mistakenly sent an empty command).
        endOfFile = true;

    } else {
        fprintf(stderr, "Unrecognized command: %s\n", command.c_str());
        //        std::cerr << "Unrecognized command: " << command << std::endl;
    }
    return endOfFile;
}

int  main (int argc, const char * argv[])
{
    fprintf(stderr, "Entering process %s\n", argv[0]);

    /* When exec'ing the publisher and subscriber children
     * argv[0] is set to either "publisher" or "subscriber".
     * argv[1] is the serial number of the publisher or subscriber.
     * argv[2] is the number of topics for the test.
     */
    if (std::strcmp(argv[0], "publisher") == 0)
    {
        publisher(atoi(argv[1]), atoi(argv[2]));
        exit(0);
    }
    if (std::strcmp(argv[0], "subscriber") == 0)
    {
        subscriber(atoi(argv[1]), atoi(argv[2]));
        exit(0);
    }
    /* 
     * DIPS will default to 1 sub, 1 pub, and 5 topics if not specified on the command line.
     * Example ( ./DIPS -p 8 -s 9 -t 25 ) 8 publishers, 9 subscribers, and 25 topics.
     */
    int option;
    int subscribers = 1;
    int publishers = 1;
    int numOfTopics = 5;

    do {
        option = getopt(argc, (char * const *)argv, "p:s:t:h");
        switch (option) {
            case 'p':
                publishers = atoi(optarg);
                break;

            case 's':
                subscribers = atoi(optarg);
                break;

            case 't':
                numOfTopics = atoi(optarg);
                if (numOfTopics > kNumTopics) {
                    numOfTopics = kNumTopics;
                }
                break;

            case 'h':
                printf("Usage: main [-p <number of publishers>] [-s <number of subscribers>] [-t <number of topics>\n");
        }
    } while (option != -1);

    fprintf(stderr, "Publishers %d\n", publishers);
    fprintf(stderr, "Subscribers %d\n", subscribers);
    fprintf(stderr, "Topics %d\n", numOfTopics);

    /*
     * Start the archive thread, pass in the read and write pipe to the thread process.
     * Create the empty topics.
     */
    pthread_t archiveThread;
    Archiver archive;
    archive.numTopics = numOfTopics;
    int archiverPipe[2];
    int arPipe =  pipe(archiverPipe);
    assert(arPipe == 0);
    archive.toArchiver = archiverPipe[1];
    archive.readArchiver = archiverPipe[0];
    int tSuccess = pthread_create(&archiveThread, NULL, archiver, &archive);
    assert(tSuccess == 0);

    for (int i = 0; i < numOfTopics; i++)
    {
        topic[i] = createTopicList(i+1, archive);
    }

    /*
     Start IPC create read and write pipe for each publisher and subscriber.
     The publisher's write pipe will attempt to push data to the proxy.
     The subscriber's read pipe will listen for topics from the proxy.
    */
    for (int i = 0; i < publishers; i++) {
        ChildInfo childInfo = create_pipes_spawn_child(argv[0], "publisher", topic, kPublisherChild, numOfTopics, i + 1);
        children.push_back(childInfo);
    }
    for (int i = 0; i < subscribers; i++) {
        ChildInfo childInfo = create_pipes_spawn_child(argv[0], "subscriber", topic, kSubscriberChild, numOfTopics, i + 1);
        children.push_back(childInfo);
    }


    struct timeval wait;
    /* 
    Wait 2 seconds for one of the children to send us a message.
    */
    wait.tv_sec = 2;
    wait.tv_usec = 0;

    /* 
    fd_set will hold FDs waiting for updates, ZERO to init.
    Loop through all FDs in the program, find the read pipes that have changed, set them in fd_set.
    Find FDs in our set, handle commands from set read pipes.
    */
    fd_set readfds;
    FD_ZERO(&readfds);
    bool finished = false;
    while(!finished)
    {
        FD_ZERO(&readfds);
        int max = -1;
        int numAliveFDs = 0;
        for (std::vector<ChildInfo>::iterator iter = children.begin(); iter != children.end(); ++iter)
        {
            ChildInfo temp = *iter;
            if ( temp.fromChild > max )
            {
                max = temp.fromChild;
            }
            if(temp.fromChild > 0) {
                if (temp.connectionType == kNoConnection) {
                    FD_SET(temp.fromChild, &readfds);
                }
                if (temp.childType == kPublisherChild) {
                    numAliveFDs++;
                }
            }

        }
        if (numAliveFDs > 0)
        {
            int numPipesReadyForReading = select(max+1, &readfds, NULL, NULL, &wait);
            if (numPipesReadyForReading > 0)
            {
                for ( int i = 0; i < max+1; i++ ) {
                    if ( FD_ISSET(i, &readfds))
                    {
                        int childIndex = findChildWithFD(children, i);
                        //        			std::cerr << "here: " << i << " || with childIndex: " << childIndex << " ||  With from child in FD_ISSET: "  << temp.fromChild << std::endl;
                        bool end = handleCommand(children[childIndex]);
                        if(end)
                        {
                            //                            std::cerr << "closing the pipe" << std::endl;
                            close(children[childIndex].fromChild);
                            children[childIndex].fromChild = -1;
                        }
                    }
                }
            }
        } else {
            /* 
             Handled every updated FD in fd_set
             */
            finished = true;
        }
    }

    /* 
    Give the subscribers time to get their messages.
     */
    sleep(5);

    /* 
    Let the subscriber threads know we are shutting down.
     */
    shuttingDown = true;

    /* 
     Join the shutdown threads
     */
    fprintf(stderr, "Waiting for threads to finish\n");
    std::vector<ChildInfo>::iterator it;
    for (it = children.begin(); it != children.end(); it++) {
        if (it->thread) {
            int joinResult = pthread_join(it->thread, NULL);
            assert(joinResult == 0);
        }
    }

    /* Get all of the non-archived messages into the archive.
     */
    for (int i = 0; i < archive.numTopics; i++) {
        writeAllToArchive(topic[i]);
    }
    
    terminateArchiver(archive);
    
    fprintf(stderr, "Waiting for archiving thread to finish...\n");
    int archiveJoinResult = pthread_join(archiveThread, NULL);
    assert(archiveJoinResult == 0);
    fprintf(stderr, "Archive thread done.\n");
    fprintf(stderr, "Exiting the program\n");
}

