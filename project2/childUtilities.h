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

void writeCommandToServer(std::string str, std::string expectedResponse);