#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <iostream>
#include <sndfile.h>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

struct ThreadArgs
{
    vector<float> inputSignal;
    vector<float> &outputSignal;
    size_t start;
    size_t end;
    ThreadArgs(const vector<float> &input, vector<float> &output, size_t s, size_t e)
        : inputSignal(input), outputSignal(output), start(s), end(e) {}
};

struct ThreadArgsWrite
{
    vector<float> data;
    SF_INFO &info;
    int index;

    ThreadArgsWrite(const vector<float> output, SF_INFO &sig_info, int num)
        : data(output), info(sig_info), index(num) {}
};

void readWavFile(const std::string &inputFile, std::vector<float> &data, SF_INFO &fileInfo);
void writeWavFile(const std::string &outputFile, const std::vector<float> &data, SF_INFO &fileInfo);

#endif
