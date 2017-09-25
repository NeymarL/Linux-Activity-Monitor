#ifndef TOOLS_H
#define TOOLS_H

#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <string.h>

bool is_digit(char* str);
std::map<int, std::string>* get_username_dict();
std::string get_proper_unit(int num);

#endif
