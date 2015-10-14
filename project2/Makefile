all: DIPS

DIPS: main.o topicfifo.o childUtilities.o publisherProcess.o subscriber.o threadUtilities.o pubServerThread.o subServerThread.o archiver.o
	g++ main.o topicfifo.o childUtilities.o publisherProcess.o subscriber.o threadUtilities.o pubServerThread.o subServerThread.o archiver.o -o DIPS

main.o: main.cpp
	g++ -c main.cpp
	
topicfifo.o: topicfifo.cpp
	g++ -c topicfifo.cpp

childUtilities.o: childUtilities.cpp
	g++ -c childUtilities.cpp

publisherProcess.o: publisherProcess.cpp
	g++ -c publisherProcess.cpp
	
subscriber.o: subscriber.cpp
	g++ -c subscriber.cpp
	
threadUtilities.o: threadUtilities.cpp
	g++ -c threadUtilities.cpp
	
pubServerThread.o: pubServerThread.cpp
	g++ -c pubServerThread.cpp

subServerThread.o: subServerThread.cpp
	g++ -c subServerThread.cpp
	
archiver.o: archiver.cpp
	g++ -c archiver.cpp
	
clean:
	rm -rf *o DIPS