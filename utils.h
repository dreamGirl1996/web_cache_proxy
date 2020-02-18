#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>
#include <sstream>
#include <cctype>
#include <vector>
#include <cstring>

void printALine(size_t size) {
    std::string line;
    for (size_t i = 0; i < size; i++) {
        line += '-';
    }
    std::cout << line << "\n";
}

void skipSpace(const char ** strp) {
    //code cited from homework 081
    while (isspace(**strp)) {
      *strp = *strp + 1;
    }
}

void cleanVectorChar(std::vector<char> & vecChar) {
    while (vecChar.size() > 0) {
        vecChar.pop_back();
    }
}

void cstrToVectorChar(std::vector<char> & vecChar, const char * cstr) {
    cleanVectorChar(vecChar);
    for (size_t i = 0; i < strlen(cstr); i++) {
          vecChar.push_back(cstr[i]);
    }
    if (vecChar.size() > 0) {
        vecChar.push_back('\0');
    }
}

void appendCstrToVectorChar(std::vector<char> & vecChar, const char * cstr) {
    for (size_t i = 0; i < strlen(cstr); i++) {
          vecChar.push_back(cstr[i]);
    }
    if (vecChar.size() > 0) {
        vecChar.push_back('\0');
    }
}

#endif