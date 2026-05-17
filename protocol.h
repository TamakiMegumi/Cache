#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include "utils.h"
bool parseSetCommand(const std::string &cmd, std::string &key,
                     std::string &value, int &expireSec);