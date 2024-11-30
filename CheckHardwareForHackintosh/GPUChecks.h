#pragma once
#include <string>

std::string detectGPU();

std::string getSupportedMacOSVersion(const std::string& gpuModel);

std::string getSupportedMacOSVersionC(const std::string& gpuModel);

bool checkGPU(const std::string& gpuModel);

bool checkNVIDIA(const std::string& gpuModel);

bool checkAMD(const std::string& gpuModel);

bool checkIntel(const std::string& gpuModel);

std::string trimSpaces(const std::string& str);