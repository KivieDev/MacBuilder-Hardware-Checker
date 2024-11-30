#pragma once
#include <string>
#include <utility>

struct NetworkInfo {
    std::string adaptername;
    bool test;
};

NetworkInfo checkNetworkAdapterSupport();


