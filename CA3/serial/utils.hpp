#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <iostream>
#include <sndfile.h>
#include <vector>
#include <string>
#include <cstring>

void readWavFile(const std::string &inputFile, std::vector<float> &data, SF_INFO &fileInfo);
void writeWavFile(const std::string &outputFile, const std::vector<float> &data, SF_INFO &fileInfo);

#endif
