#ifndef _FILTERS_HPP_
#define _FILTERS_HPP_

#include <string>
#include <vector>
#include <cmath>
#include "statics.hpp"
#include <chrono>
#include <iostream>

using namespace std;

vector<float> applyBandPassFilter(const vector<float> &signal);
vector<float> applyNotchFilter(const vector<float> &signal);
vector<float> applyFIRFilter(const vector<float> &inputSignal);
vector<float> applyIIRFilter(const vector<float> &inputSignal);

#endif