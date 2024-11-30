#pragma once
#include <string>
#include <utility>

struct StorageInfo {
    std::string adaptername;
    bool test;
};
StorageInfo checkStorageSupport();