#include "filters.hpp"

float bandPassFilter(float frequency)
{
    return pow(frequency, 2) / (pow(frequency, 2) + pow(DELTA_F, 2));
}

vector<float> applyBandPassFilter(const vector<float> &signal)
{
    vector<float> filtered_signal(signal.size());

    for (size_t i = 0; i < signal.size(); i++)
    {
        float h = bandPassFilter(signal[i]);
        filtered_signal[i] = h;
    }

    return filtered_signal;
}

float notchFilter(float frequency)
{
    return 1 / (pow(frequency / F_0, 2 * N) + 1);
}

vector<float> applyNotchFilter(const vector<float> &signal)
{
    vector<float> filtered_signal(signal.size());

    for (size_t i = 0; i < signal.size(); i++)
    {
        float h = notchFilter(signal[i]);
        filtered_signal[i] = h;
    }

    return filtered_signal;
}

vector<float> applyFIRFilter(const vector<float> &inputSignal)
{
    int signalSize = inputSignal.size();
    vector<float> outputSignal(signalSize, 0.0);

    for (int n = 0; n < signalSize; ++n)
    {
        for (int k = 0; k < FILTER_SIZE; ++k)
        {
            if (n - k >= 0)
            {
                outputSignal[n] += COEFFICIENT * inputSignal[n - k];
            }
        }
    }

    return outputSignal;
}

vector<float> applyIIRFilter(const vector<float> &inputSignal)
{
    int signalSize = inputSignal.size();
    vector<float> outputSignal(signalSize, 0.0);

    for (int n = 0; n < signalSize; ++n)
    {
        for (int k = 0; k < FEEDFORWARD_SIZE; ++k)
        {
            if (n - k >= 0)
            {
                outputSignal[n] += FEEDFORWARD_COEFFICIANT * inputSignal[n - k];
            }
        }
        for (int j = 1; j < FEEDBACK_SIZE; ++j)
        {
            if (n - j >= 0)
            {
                outputSignal[n] -= FEEDBACK_COEFFICIANT * outputSignal[n - j];
            }
        }
    }

    return outputSignal;
}