#include "filters.hpp"

float bandPassFilter(float frequency)
{
    return pow(frequency, 2) / (pow(frequency, 2) + pow(DELTA_F, 2));
}

void applyBandPassFilter(const vector<float> &signal, vector<float> &output, int start, int end)
{
    for (size_t i = start; i < end; i++)
    {
        float h = bandPassFilter(signal[i]);
        output[i] = h;
    }
}

float notchFilter(float frequency)
{
    return 1 / (pow(frequency / F_0, 2 * N) + 1);
}

void applyNotchFilter(const vector<float> &signal, vector<float> &output, int start, int end)
{
    for (size_t i = start; i < end; i++)
    {
        float h = notchFilter(signal[i]);
        output[i] = h;
    }
}

void applyFIRFilter(const vector<float> &signal, vector<float> &output, int start, int end)
{
    for (int n = start; n < end; ++n)
    {
        for (int k = 0; k < FILTER_SIZE; ++k)
        {
            if (n - k >= 0)
            {
                output[n] += COEFFICIENT * signal[n - k];
            }
        }
    }
}

void applyIIRFilter(const vector<float> &signal, vector<float> &output, int start, int end)
{
    for (int n = start; n < end; ++n)
    {
        for (int k = 0; k < FEEDFORWARD_SIZE; ++k)
        {
            if (n - k >= 0)
            {
                output[n] += FEEDFORWARD_COEFFICIANT * signal[n - k];
            }
        }
        for (int j = 1; j < FEEDBACK_SIZE; ++j)
        {
            if (n - j >= 0)
            {
                output[n] -= FEEDBACK_COEFFICIANT * output[n - j];
            }
        }
    }
}