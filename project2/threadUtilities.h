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
#include "myTypes.h"

void writeStr(int fd, const char *str);

bool hasPrefix(std::string full, std::string prefix);

void read_from_pipe (int file, std::string &str);

void splitTopicEntry(std::string command, std::vector<std::string>& splits);
