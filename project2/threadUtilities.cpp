#include "threadUtilities.h"

void writeStr(int fd, const char *str)
{
    size_t len = strlen(str);
    ssize_t result = write(fd, str, len);
    assert(result == len);

    result = write(fd, "\n", 1);
    assert(result == 1);
}

bool hasPrefix(std::string full, std::string prefix)
{
    bool isPrefix = false;
    if (full.size() >= prefix.size()) {
        isPrefix = std::equal(prefix.begin(), prefix.end(), full.begin());
    }
    return isPrefix;
}

void read_from_pipe (int file, std::string &str)
{
    //    std::cerr << "file being read in: " << file << std::endl;
    char buffer[100];
    char *inBuffer = buffer;
    char c;
    while ( (read(file, &c, 1)) > 0) {
        //    	std::cerr << "Read byte: " << (int)c << std::endl;
        if (c != '\n') {
            *inBuffer++ = c;
            *inBuffer = '\0';		//debugging

        } else {
            *inBuffer = '\0';
            break;
        }
    }

    str = buffer;

}

/* Split up a topic command into its three component parts:
 * <command> <topic> <data>
 * The three strings are appended to the provided array.
 */
void splitTopicEntry(std::string command, std::vector<std::string>& splits)
{
    static const char *delimiters = " ";
    char *save = NULL;
    char *str = strdup(command.c_str());
    assert(str != nullptr);

    /* Push the command (should be topic)
     */
    char *pch = strtok_r(str, delimiters, &save);
    if (pch != NULL) {
        if (strcmp(pch, "topic")) {
            fprintf(stderr, "%s: bad command = %s\n", __func__, command.c_str());
        }
        assert(strcmp(pch, "topic") == 0);
        splits.push_back(pch);
        pch = strtok_r(nullptr, delimiters, &save);
    }

    /* And the topic number.
     */
    if (pch != NULL) {
        splits.push_back(pch);
        pch = strtok_r(NULL, "", &save);
    }

    /* And the data entry.
     */
    if (pch != NULL) {
        splits.push_back(pch);
    }

    free(str);
    
    assert(splits.size() == 3);
}