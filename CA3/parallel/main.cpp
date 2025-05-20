#include "utils.hpp"
#include "filters.hpp"
#include <chrono>
#include <pthread.h>

using namespace std;

void *filterThreadBnadPass(void *args)
{
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    applyBandPassFilter(
        threadArgs->inputSignal,
        threadArgs->outputSignal,
        threadArgs->start,
        threadArgs->end);
    pthread_exit(nullptr);
}
void *filterThreadNotch(void *args)
{
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    applyNotchFilter(
        threadArgs->inputSignal,
        threadArgs->outputSignal,
        threadArgs->start,
        threadArgs->end);
    pthread_exit(nullptr);
}
void *filterThreadFIR(void *args)
{
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    applyFIRFilter(
        threadArgs->inputSignal,
        threadArgs->outputSignal,
        threadArgs->start,
        threadArgs->end);
    pthread_exit(nullptr);
}
void *filterThreadIIR(void *args)
{
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    applyIIRFilter(
        threadArgs->inputSignal,
        threadArgs->outputSignal,
        threadArgs->start,
        threadArgs->end);
    pthread_exit(nullptr);
}

void *writeThread(void *args)
{
    ThreadArgsWrite *threadArgs = (ThreadArgsWrite *)args;
    writeWavFile(
        OUTPUT_PATH[threadArgs->index],
        threadArgs->data,
        threadArgs->info);
    pthread_exit(nullptr);
}

void createThreadsBandPass(const vector<float> &input, vector<float> &output)
{
    int signal_size = input.size();
    pthread_t threads[NUM_THREADS];
    size_t chunkSize = signal_size / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        int end = (i == NUM_THREADS - 1) ? signal_size : (i + 1) * chunkSize;
        ThreadArgs *args = new ThreadArgs(input, output, i * chunkSize, end);
        int status = pthread_create(&threads[i], nullptr, filterThreadBnadPass, args);
        if (status)
        {
            cout << "Error in creating thread\n";
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(threads[i], nullptr);
    }
}

void createThreadsNotch(const vector<float> &input, vector<float> &output)
{
    int signal_size = input.size();
    pthread_t threads[NUM_THREADS];
    size_t chunkSize = signal_size / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        int end = (i == NUM_THREADS - 1) ? signal_size : (i + 1) * chunkSize;
        ThreadArgs *args = new ThreadArgs(input, output, i * chunkSize, end);
        int status = pthread_create(&threads[i], nullptr, filterThreadNotch, args);
        if (status)
        {
            cout << "Error in creating thread\n";
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(threads[i], nullptr);
    }
}

void createThreadsFIR(const vector<float> &input, vector<float> &output)
{
    int signal_size = input.size();
    pthread_t threads[NUM_THREADS];
    size_t chunkSize = signal_size / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        int end = (i == NUM_THREADS - 1) ? signal_size : (i + 1) * chunkSize;
        ThreadArgs *args = new ThreadArgs(input, output, i * chunkSize, end);
        int status = pthread_create(&threads[i], nullptr, filterThreadFIR, args);
        if (status)
        {
            cout << "Error in creating thread\n";
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(threads[i], nullptr);
    }
}

void createThreadsIIR(const vector<float> &input, vector<float> &output)
{
    int signal_size = input.size();
    pthread_t threads[NUM_THREADS];
    size_t chunkSize = signal_size / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        int end = (i == NUM_THREADS - 1) ? signal_size : (i + 1) * chunkSize;
        ThreadArgs *args = new ThreadArgs(input, output, i * chunkSize, end);
        int status = pthread_create(&threads[i], nullptr, filterThreadIIR, args);
        if (status)
        {
            cout << "Error in creating thread\n";
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(threads[i], nullptr);
    }
}

void createThreadsWrite(const vector<vector<float>> &outputs, vector<SF_INFO> &infos)
{
    pthread_t threads[4];
    for (int i = 0; i < 4; i++)
    {
        ThreadArgsWrite * args = new ThreadArgsWrite(outputs[i], infos[i], i);
        pthread_create(&threads[i], nullptr, writeThread, args);
    }
    for (int i = 0; i < 4; ++i)
    {
        pthread_join(threads[i], nullptr);
    }
}

int main(int argc, char const *argv[])
{
    auto start = chrono::high_resolution_clock::now();
    cout << "Voice filter program (parallel)\n";
    string input_file_path = argv[1];

    SF_INFO fileInfo;
    vector<float> audio_data;
    memset(&fileInfo, 0, sizeof(fileInfo));

    auto start_read = chrono::high_resolution_clock::now();
    readWavFile(input_file_path, audio_data, fileInfo);
    vector<SF_INFO> info(4, fileInfo);

    int signal_size = audio_data.size();

    vector<vector<float>> outputs(4, vector<float>(signal_size));

    auto start_band_pass_filter = chrono::high_resolution_clock::now();
    createThreadsBandPass(audio_data, outputs[0]);

    auto start_notch = chrono::high_resolution_clock::now();
    createThreadsNotch(audio_data, outputs[1]);

    auto start_FIR = chrono::high_resolution_clock::now();
    createThreadsFIR(audio_data, outputs[2]);

    auto start_IIR = chrono::high_resolution_clock::now();
    createThreadsIIR(audio_data, outputs[3]);

    auto start_write = chrono::high_resolution_clock::now();
    createThreadsWrite(outputs, info);

    auto end = chrono::high_resolution_clock::now();

    cout << "Time results:\n";
    cout << "Read: " << (chrono::duration_cast<chrono::microseconds>(start_band_pass_filter - start_read)).count() << " ms" << endl;
    cout << "Band Pass Filter: " << (chrono::duration_cast<chrono::microseconds>(start_notch - start_band_pass_filter)).count() << " ms" << endl;
    cout << "Notch: " << (chrono::duration_cast<chrono::microseconds>(start_FIR - start_notch)).count() << " ms" << endl;
    cout << "FIR: " << (chrono::duration_cast<chrono::microseconds>(start_IIR - start_FIR)).count() << " ms" << endl;
    cout << "IIR: " << (chrono::duration_cast<chrono::microseconds>(start_write - start_IIR)).count() << " ms" << endl;
    cout << "write: " << (chrono::duration_cast<chrono::microseconds>(end - start_write)).count() << " ms" << endl;
    cout << "Execution: " << (chrono::duration_cast<chrono::microseconds>(end - start)).count() << " ms" << endl;
    return 0;
}
