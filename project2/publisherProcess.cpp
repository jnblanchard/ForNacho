#include "publisherProcess.h"

void writePubTopicCommandToServer(int n)
{
    std::string pubTopic = "pub topic ";
    std::string numString = std::to_string(n);
    pubTopic.append(numString);
    writeCommandToServer(pubTopic, "accept");
}

bool pushEntryToPubProxy(int n ,std::string str)
{
    std::cout << "topic " << n << " " << str << std::endl;
    std::cout.flush();

    std::string response;
    std::cin >> response;

    fprintf(stderr, "pub entry pushed to proxy: %s || response: %s \n", str.c_str(), response.c_str());


    assert(response == "retry" || response == "successful");
    if (response == "successful") {
        return true;
    } else {
        return false;
    }
}

void publisher(int proxyID, int numTopics)
{

    int totalTopicsForPubID = numTopics - ((proxyID -1) % numTopics);
    writeCommandToServer("pub connect", "accept");
    for (int i = 0; i < numTopics; i++) {
        writePubTopicCommandToServer(i);
    }
    for (int j = 0; j < totalTopicsForPubID*5; j++) {
        std::string message = "Publisher: " + std::to_string(proxyID) + ", Article: " + std::to_string(j+1) + ", Topic: " + std::to_string(1+j/5);
        pushEntryToPubProxy(proxyID, message);
    }
    writeCommandToServer("end", "accept");
    writeCommandToServer("terminate", "terminate");
}
