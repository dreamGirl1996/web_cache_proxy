#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>
#include <sstream>
#include <cctype>
#include <vector>
#include <cstring>
#include <locale>
#include <iomanip>
// #include <exception>

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
    if (vecChar.size() > 0 && vecChar[vecChar.size()-1] == '\0') {
        vecChar.pop_back(); // pop '\0'
    }
    for (size_t i = 0; i < strlen(cstr); i++) {
          vecChar.push_back(cstr[i]);
    }
    if (vecChar.size() > 0) {
        vecChar.push_back('\0');
    }
}

std::tm getDatetime(std::vector<char> & datetimeVectorChar) {
    std::tm t = {};
    std::istringstream ss(datetimeVectorChar.data());
    // ss.imbue(std::locale("de_DE.utf-8"));
    ss >> std::get_time(&t, "%a, %d %b %Y %H:%M:%S");
    if (ss.fail()) {
        std::stringstream ess;
        ess << "Parse failed on datetime in " << __func__;
        throw std::invalid_argument(ess.str());
    } 
    return t;
}

#endif