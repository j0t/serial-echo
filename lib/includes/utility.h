#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sys/ioctl.h>

void makeVector(std::vector<char> &inputVector, const char *data, std::size_t size = -1);
std::string modemStatusToString(unsigned int status);
