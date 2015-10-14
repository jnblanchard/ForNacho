#include "subscriber.h"
#include <sstream>
#include "threadUtilities.h"

void writeSubTopicCommandToServer(int n)
{
    std::string subTopic = "sub topic ";
    std::string numString = std::to_string(n);
    subTopic.append(numString);
    writeCommandToServer(subTopic, "accept");
}

void subscriber(int proxyID, int numTopics)
{

//    fprintf(stderr, "%s: Entering\n", __func__);

    writeCommandToServer("sub connect", "accept");

//    fprintf(stderr, "%s: did sub connect\n", __func__);
    for (int i = 1; i <= numTopics; i++) {
        writeSubTopicCommandToServer(i);
    }
    while (!std::cin.eof()) {
        std::string response;

        std:getline(std::cin, response);

        fprintf(stderr, "%s\n", response.c_str());

        if (response != "") {
            std::vector<std::string> splits;
            splitTopicEntry(response, splits);

            writeResult(proxyID, splits[2]);

            std::cout << "successful" << std::endl;
        }

    };
//    fprintf(stderr, "%s: done wih topic loop.\n", __func__);

    writeCommandToServer("end", "accept");
    writeCommandToServer("terminate", "terminate");
}

/* Write the given string to stdout. The console stdout is
 * on fd 3 since the program uses stdin (0) and stdout (10
 * to communicate with the server.
 */
void writeResult(int subscriberID, std::string text)
{
    std::ostringstream finalS;
    finalS << "Subscriber: " << subscriberID << ", " << text;

    const char *str = finalS.str().c_str();
    size_t len = strlen(str);
    write(3, str, len);
    write(3, "\n", 1);
}