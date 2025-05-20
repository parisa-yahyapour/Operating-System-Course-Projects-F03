#include "utils.hpp"
#include "filters.hpp"
#include <chrono>

using namespace std;

int main(int argc, char const *argv[])
{
    auto start = chrono::high_resolution_clock::now();
    cout << "Voice filter program (serial)\n";
    string input_file_path = argv[1];

    SF_INFO fileInfo, fileInfo2, fileInfo3, fileInfo4;
    vector<float> audio_data;
    memset(&fileInfo, 0, sizeof(fileInfo));

    auto start_read = chrono::high_resolution_clock::now();
    readWavFile(input_file_path, audio_data, fileInfo);
    fileInfo2 = fileInfo;
    fileInfo3 = fileInfo;
    fileInfo4 = fileInfo;

    auto start_band_pass_filter = chrono::high_resolution_clock::now();
    vector<float> band_pass = applyBandPassFilter(audio_data);

    string output_path = "outputBandPassSerial.wav";
    auto start_write_band_pass = chrono::high_resolution_clock::now();
    writeWavFile(output_path, band_pass, fileInfo);

    auto start_notch = chrono::high_resolution_clock::now();
    vector<float> notch = applyNotchFilter(audio_data);

    output_path = "outputNotchSerial.wav";
    auto start_write_notch = chrono::high_resolution_clock::now();
    writeWavFile(output_path, notch, fileInfo2);

    auto start_FIR = chrono::high_resolution_clock::now();
    vector<float> FIR = applyFIRFilter(audio_data);

    output_path = "outputFIRSerial.wav";
    auto start_write_FIR = chrono::high_resolution_clock::now();
    writeWavFile(output_path, FIR, fileInfo3);

    auto start_IIR = chrono::high_resolution_clock::now();
    vector<float> IIR = applyIIRFilter(audio_data);

    output_path = "outputIIRSerial.wav";
    auto start_write_IIR = chrono::high_resolution_clock::now();
    writeWavFile(output_path, FIR, fileInfo4);

    auto end = chrono::high_resolution_clock::now();

    cout << "Time results:\n";
    cout << "Read: " << (chrono::duration_cast<chrono::microseconds>(start_band_pass_filter - start_read)).count() << " ms" << endl;
    cout << "Band Pass Filter: " << (chrono::duration_cast<chrono::microseconds>(start_write_band_pass - start_band_pass_filter)).count() << " ms" << endl;
    cout << "Notch: " << (chrono::duration_cast<chrono::microseconds>(start_write_notch - start_notch)).count() << " ms" << endl;
    cout << "FIR: " << (chrono::duration_cast<chrono::microseconds>(start_write_FIR - start_FIR)).count() << " ms" << endl;
    cout << "IIR: " << (chrono::duration_cast<chrono::microseconds>(start_write_IIR - start_IIR)).count() << " ms" << endl;

    auto write_duration = chrono::duration_cast<chrono::microseconds>(start_notch - start_write_band_pass) +
                          chrono::duration_cast<chrono::microseconds>(start_FIR - start_write_notch) +
                          chrono::duration_cast<chrono::microseconds>(start_IIR - start_write_FIR) +
                          chrono::duration_cast<chrono::microseconds>(end - start_write_IIR);

    cout << "write: " << write_duration.count() << " ms" << endl;
    cout << "Execution: " << (chrono::duration_cast<chrono::microseconds>(end - start)).count() << " ms" << endl;
    return 0;
}
