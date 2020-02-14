#ifndef __CLIENTTEST_H__
#define __CLIENTTEST_H__

#include "ClientSocket.h"
#include <cassert>
#include <sstream>
#include <algorithm>

void printALine(size_t size) {
    std::string line;
    for (size_t i = 0; i < size; i++) {
        line += '-';
    }
    std::cout << line << "\n";
}

// Begin citation
// https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/
std::string ltrim(const std::string & s) {
	size_t start = s.find_first_not_of(' ');
	return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string & s) {
	size_t end = s.find_last_not_of(' ');
	// return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    return (end == std::string::npos) ? "" : s.substr(0, end);  // remove '\n'
}

std::string trim(const std::string & s) {
	return rtrim(ltrim(s));
}
// End of citation

//-2: not found
//-1: error
// 0: found
// 1: found previously
int findHostName(bool & isfound, const std::string & lineMsg, std::string & hostName) {
    if (isfound) {return 1;}
    std::string lineMsgLower = lineMsg;
    for (size_t i = 0; i < lineMsgLower.size(); i++) {
        lineMsgLower[i] = tolower(lineMsgLower[i]);
    }
    if (size_t loc = lineMsgLower.find("host:") != std::string::npos) {
        isfound = true;
        loc += 5;  // size of "host:"
        while (isspace(lineMsg[loc]) && lineMsg[loc] != '\n') {
            loc++;
        }
        if (lineMsg[loc] == '\n' || lineMsg[loc] == '\0') {
            std::cerr << "client test: syntax error on HOST line";
            return -1;
        }
        hostName = lineMsg.substr(loc, lineMsg.size() - loc);
        return 0;
    }
    return -2;
} 

bool readInput(FILE * in, std::string & hostName, std::string & request) {
    assert(in != NULL);
    char * line = NULL;
    size_t sz;
    bool isfound = false;
    while (getline(&line, &sz, in) != -1) {
        const char * temp = line;
        std::string lineMsg = temp;
        lineMsg = trim(lineMsg);

        if (findHostName(isfound, lineMsg, hostName) == -1) {
            return false;
        }
        
        request += lineMsg + "\r\n";
        if (in != stdin) {
            std::cout << lineMsg << "\n";
        }
    }
    if (hostName.size() == 0 || request.size() == 0) {
        std::cerr << "client test: void information was get from testcase file" << "\n";
        return false;
    }
    request += "\r\n";
    free(line);
    return true;
}

bool closeFile(FILE * f, const char * fileNm) {
  assert(f != stdin);
  if (fclose(f) != 0) {
    std::cerr << "An exception happened when closing '" << fileNm << "'\n";
    perror("Close failure");
    return false;
  }
  return true;
}

#endif