#include <cstdint>
#include <iostream>
#include <fstream>
#include <math.h>
#include "WavCreater.h"

using namespace std;

 int main()
{
    int samplefreq=41100;
    int channels=2;
    int channelbits=8;
    int duration=100;//单位s
    int volume=100;
    int freq=10000;
    string file_path="newWave.wav";
    DWORD totalLen = (samplefreq * channels * channelbits / 8) * duration + 44;
    char *allBuffer=new char[totalLen];
    CreateHead(duration,samplefreq,channels,channelbits,allBuffer);
    MakeWaveData(samplefreq, freq, volume, allBuffer+ WAVE_HEAD_LENGTH, samplefreq*duration,channelbits,channels);
    WriteWavFile(file_path,allBuffer,totalLen);
    return 0;
}
