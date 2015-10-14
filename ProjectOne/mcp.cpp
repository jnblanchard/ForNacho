#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <algorithm>

using namespace std;


vector<int> pids;
vector<int> children;
int sleepTime;
int curChildPID;
int curIndex = 0;
char* command;
char* path;

int numOfProcesses = 0;


class CommandContainer
{
public:
    std::string path;
    std::string command;
    int pid;
};

CommandContainer commandContainer;

class InFunct {
public:
    std::string name;
    int length;
    std::string path;
    std::string command;
};

std::vector<InFunct> populateArray(int numOfElements) {
    vector<InFunct> functions;
    for (int i = 0; i <= numOfElements; i++) {
        InFunct funct;
        funct.name = "Process ";
        funct.length = i;
        funct.path = "/usr/bin/ls";
        funct.command = "ls";
        functions.push_back(funct);
    }
    //	char output[100];
    //    std::ifstream readFile;
    //    readFile.open("input.txt");
    ////	std::cout << "Here First" << std::endl;
    //	if (readFile.is_open()) {
    //		int count = 1;
    //		InFunct* funct;
    //		while (!readFile.eof()) {
    //			readFile >> output;
    //			if( (count % 2) == 0) {
    //				funct->length = output;
    ////				std::cout << funct->length << std::endl;
    //				functions.push_back(funct);
    //			} else {
    //				funct = new InFunct;
    //				funct->name = output;
    ////				std::cout << funct->name << std::endl;
    //			}
    //			count++;
    ////			std::cout << output << std::endl;
    //		}
    //	}
    return functions;
}

void child()
{
    //	std::cout << "sleeping for: " << sleepTime << std::endl;
    int receivedSignal;
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigset, NULL);
    sigwait(&sigset, &receivedSignal);
    if ( receivedSignal == SIGUSR1 ) {
        fprintf(stderr, "Signal Received - child process: %i $ execl(%s, %s, NULL)\n", commandContainer.pid,commandContainer.path.c_str(), commandContainer.command.c_str());
        execl(commandContainer.path.c_str(), commandContainer.command.c_str(), NULL);
    }
}

void alarmHandler(int sigNum)
{
    curChildPID = pids.at(curIndex);
    curIndex++;
    if ( curIndex >= pids.size() ) {
        curIndex = 0;
    }
    kill(curChildPID, SIGSTOP);
    curChildPID = pids.at(curIndex);
    kill(curChildPID, SIGCONT);
    //	signal(SIGALRM, SIG_IGN);
    std::cout << "alarm raised" << std::endl;
    //	signal(SIGALRM, alarmHandler);
    signal(SIGALRM, alarmHandler);
    alarm(1);
}





int main() {
    std::vector<InFunct> functions = populateArray(20);
    for (std::vector<InFunct>::iterator iter = functions.begin(); iter != functions.end(); ++iter)
    {
        InFunct temp = *iter;
        string sIdentifer;
        cout << temp.name << ' ' << temp.length << endl;
        char * tempPath = new char[temp.path.size() + 1];
        std::copy(temp.path.begin(), temp.path.end(), tempPath);
        tempPath[temp.path.size()] = '\0';
        path = tempPath;
        commandContainer.path = path;
        delete[] tempPath;
        cout << "path: " << path << endl;
        char * tempCommand = new char[temp.command.size() + 1];
        std::copy(temp.command.begin(), temp.command.end(), tempCommand);
        tempCommand[temp.command.size()] = '\0';
        command = tempCommand;
        commandContainer.command = command;
        delete[] tempCommand;
        cout << "command: " << command << endl;
        int pid = fork();
        commandContainer.pid = getpid();
        pids.push_back(pid);
        std::cout << "PID: " << pid << std::endl;
        numOfProcesses++;
        if ( pid == 0 ) {
            child();
            exit(1);
        } else if ( pid < 0 ) {
            cout << strerror(errno) << endl;
        } else {
            cout << "Parent process: " << getpid() << endl;
        }
        cout << "_____________________________________" << endl;
    }

    for (vector<int>::iterator itera = pids.begin(); itera != pids.end(); ++itera)
    {
        int childPID = *itera;
        kill(childPID, SIGSTOP);
        cout << "Child: " << childPID << " sent signal to stop" << endl;
    }


    curChildPID = pids.at(curIndex);
    kill(curChildPID, SIGCONT);
    cout << "Children sent signal to begin again." << endl;
    signal(SIGALRM, alarmHandler);
    alarm(1);

    for (std::vector<int>::iterator iter = pids.begin(); iter != pids.end(); ++iter)
    {
        int stat_loc;
        int finishedChild = wait(&stat_loc);
        children.push_back(finishedChild);
        //		wait(&finishedChild);
        cout << finishedChild << " terminated" << endl;
    }
    
    cout << "_______________________________________" << endl;
    cout << "Number of Processes: " << numOfProcesses << endl;
    cout << "Parent Process: " << getpid() << endl;
    cout << "    Child Processes:" << endl;
    int childCount = 0;
    for (vector<int>::iterator iter = pids.begin(); iter != pids.end(); ++iter)
    {
        childCount++;
        cout << "   Child Process " << childCount << ": " << *iter << endl;
    }
    return 0;
}
