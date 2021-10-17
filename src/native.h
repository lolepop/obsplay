#pragma once

#include <unordered_set>
#include <vector>
#include <string>

#include "source.h"

namespace Native
{
    std::unordered_set<std::string> getAllWindows();
    bool isWindowOpen(std::vector<std::string> filter);
}