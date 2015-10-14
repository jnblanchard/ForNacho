#include "pubServerThread.h"

/*
 * pub server thread
 * pass in child publisher process arg[0]
 * Listen for commands: "end", "pub topic", "topic", ""
 * If we are not at end of the publisher read pipe then continue listening for commands.
 * If the pubServer receives a 'pub topic 5 <message>' command then the publisher will push the message to the TopicList and add the topic# to the list of topics that process has published to.
 * "" command specifies that the pub server is done.
 */
void *pubServer(void *childInfoP)
{
    ChildInfo &childInfo = *(ChildInfo *)childInfoP;

    bool done = false;
    while (!done) {
        std::string command;
        read_from_pipe(childInfo.fromChild, command);

        if (!command.compare("end")) {
            writeStr(childInfo.toChild, "accept");
            done = true;
        } else if (hasPrefix(command, "pub topic")) {

            int topic = std::atoi(command.c_str() + strlen("pub topic"));
            //            std::cerr << "Server got pub topic command: " << topic << std::endl;

            childInfo.topics->push_back(topic);

            writeStr(childInfo.toChild, "accept");
        } else if ( hasPrefix(command, "topic") ) {
            std::vector<std::string> splits;
            splitTopicEntry(command, splits);
            std::string topNumCPP = splits[1];
            char* topNumC = (char*)topNumCPP.c_str();
            int topNum = std::atoi(topNumC);
            int topicIndex = topNum - 1;
            fprintf(stderr, "%s: Got article for topic %d\n", __func__, topNum);
            if (0 <= topicIndex && topicIndex < childInfo.numTopics) {
                bool success = enqueue(childInfo.topicLists[topicIndex], childInfo.pubSubID, splits[2].c_str());
                writeStr(childInfo.toChild, success ? "successful" : "retry");
            } else {
                writeStr(childInfo.toChild, "reject");
            }
        } else if (command == "") {
            done = true;

        }  else {
            fprintf(stderr, "pub server rejecting unexpected command\n");
            //            std::cerr << "pub server rejecting unexpected command: " << command << std::endl;
            writeStr(childInfo.toChild, "reject");
        }
    }
    //    std::cerr << "exiting PubServer" << std::endl;
    /* 
     * Setting the connection type to noConnection puts the select loop back in charge of the child's file descriptor.
     */
    ((ChildInfo *)childInfoP)->connectionType = kNoConnection;
    return NULL;
}

