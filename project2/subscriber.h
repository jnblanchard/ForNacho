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
#include "childUtilities.h"
#include "myTypes.h"
#include <sstream>
#include "threadUtilities.h"


void writeResult(int subscriberID, std::string text);

void writeSubTopicCommandToServer(int n);

void subscriber(int proxyID, int numTopics);