#ifndef _FILTERS_HPP_
#define _FILTERS_HPP_

#include <string>
#include <vector>
#include <cmath>
#include "statics.hpp"
#include <chrono>
#include <iostream>

using namespace std;

void applyBandPassFilter(const vector<float> &signal, vector<float> &output, int start, int end);
void applyNotchFilter(const vector<float> &signal, vector<float> &output, int start, int end);
void applyFIRFilter(const vector<float> &signal, vector<float> &output, int start, int end);
void applyIIRFilter(const vector<float> &signal, vector<float> &output, int start, int end);

#endif