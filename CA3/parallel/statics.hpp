#ifndef _STATICS_HPP_
#define _STATICS_HPP_

using namespace std;

const float DELTA_F = 2;
const float F_0 = 1;
const int N = 2;
const float COEFFICIENT = 0.2;
const int FILTER_SIZE = 100;
const float FEEDFORWARD_COEFFICIANT = 0.5;
const float FEEDBACK_COEFFICIANT = 1.0;
const int FEEDFORWARD_SIZE = 100;
const int FEEDBACK_SIZE = 100;
const int NUM_THREADS = 10;

const vector<string> OUTPUT_PATH =
    {
        "outputBandPassParallel.wav",
        "outputNotchParallel.wav",
        "outputFIRParallel.wav",
        "outputIIRParallel.wav"
};

#endif