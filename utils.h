#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>
#include <sstream>

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
int parseHostNameFromALine(bool & isFound, const std::string & lineMsg, std::string & hostName) {
    if (isFound) {return 1;}
    std::string lineMsgLower = lineMsg;
    for (size_t i = 0; i < lineMsgLower.size(); i++) {
        lineMsgLower[i] = tolower(lineMsgLower[i]);
    }
    if (size_t loc = lineMsgLower.find("host:") != std::string::npos) {
        isFound = true;
        loc += 5;  // size of "host:"
        // while (isspace(lineMsg[loc]) && lineMsg[loc] != '\n' && lineMsg[loc] != ':') {
        while (isspace(lineMsg[loc]) && lineMsg[loc] != '\n') {
            loc++;
        }
        // if (lineMsg[loc] == '\n' || lineMsg[loc] == '\0' || lineMsg[loc] == ':') {
        if (lineMsg[loc] == '\n' || lineMsg[loc] == '\0') {
            std::cerr << "client test: syntax error on HOST line when inplicenting " << \
            __func__ << "\n";
            return -1;
        }
        hostName = lineMsg.substr(loc, lineMsg.size() - loc);
        if (hostName.find(":443") != std::string::npos) {
            hostName = hostName.substr(0, hostName.size() - 4);
        }
        return 0;
    }
    return -2;
} 

int parseMethodFromALine(bool & isFound, const std::string & lineMsg, std::string & method) {
    if (isFound) {return 1;}
    std::string lineMsgLower = lineMsg;
    // for (size_t i = 0; i < lineMsgLower.size(); i++) {
    //     lineMsgLower[i] = tolower(lineMsgLower[i]);
    // }
    if (lineMsg.find("GET") != std::string::npos) {
        method = "GET";
    }
    else if (lineMsg.find("CONNECT") != std::string::npos) {
        method = "CONNECT";
    }
    else if (lineMsg.find("POST") != std::string::npos) {
        method = "POST";
    }
    else {
        return -2;
    }
    return 0;
}

// isfound, lineMsg, Object
typedef int (*parsingMethod) (bool &, const std::string &, std::string &);
bool parseObjectFromString(const std::string & str, 
std::string & obj, parsingMethod pParsingMethod) {
    if (obj.size() > 0) {
        std::cerr << "Error occurs in method " << __func__ << ": obj should be zero length\n";
    }
    bool isFound = false;
    std::stringstream ss;
    std::string curLine;
    ss << str;
    
    while (std::getline(ss, curLine)) {
        curLine = trim(curLine);
        if (pParsingMethod(isFound, curLine, obj) == -1) {
            std::cerr << "Error occurs in method: " << __func__ << "\n";
            return false;
        }
        if (obj.size() > 0) {
            return true;
        }
    }

    std::cerr << "Empty request! In " << __func__ << "\n";
    return false;
}

#endif