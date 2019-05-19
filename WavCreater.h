#include <cstdint>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>

typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int16_t SHORT;
typedef int32_t LONG;

#define WAVE_HEAD_LENGTH 44//wav头文件长度
#define MATH_PI 3.1415

using namespace std;
//.wav文件的文件头结构

typedef struct
{
    WORD wFormatTag;//编码格式，包括WAVE_FORMAT_PCM，WAVEFORMAT_ADPCM等
    WORD nChannels;//声道数，单声道为1，双声道为2
    DWORD nSamplesPerSec;//采样频率
    DWORD nAvgBytesPerSec;//每秒的数据量
    WORD nBlockAlign;//块对齐（每个采样的字节长度）
} WAVEFORMAT;

typedef struct
{
    //WAVEFORMAT wf; // 波形格式；
    WORD wFormatTag;//编码格式，包括WAVE_FORMAT_PCM，WAVEFORMAT_ADPCM等
    WORD nChannels;//声道数，单声道为1，双声道为2
    DWORD nSamplesPerSec;//采样频率
    DWORD nAvgBytesPerSec;//每秒的数据量
    WORD nBlockAlign;//块对齐（每个采样的字节长度）
    WORD wBitsPerSample;//WAVE文件的采样大小；
} PCMWAVEFORMAT;

typedef struct
{
    char chRIFF[4];
    DWORD dwRIFFLen;
    char chWAVE[4];
    char chFMT[4];
    DWORD dwFMTLen;
    PCMWAVEFORMAT pwf;
    char chDATA[4];
    DWORD dwDATALen;
    //UINT8* pBufer;
}WaveHeader;

void MakeWaveData(int rate, int freq, int amp, char* p, int len,int m_channelbits,int m_channels)
//采样率、频率、音量、采样点数、通道比特数、通道数
{
    int flag = 0;
    if (m_channelbits == 16)        //16位
    {
        if (m_channels == 1)
        {
            for (int i = 0; i < len; i++)
            {
                SHORT v = amp/100*32768 * sin(2 * MATH_PI * freq * i / rate);
                *(p + flag) = v & 0xFF;//低8位
                *(p + flag + 1) = (v >> 8) & 0xFF;//16bit量化 高8位
                flag += 2;
            }
        }
        else
        {
            for (int i = 0; i < len; i++)
            {
                SHORT vl = amp / 100 * 32768 * sin(2 * MATH_PI * freq * i / rate) ;
                SHORT vr = amp / 100 * 32768 * sin((2 * MATH_PI * freq * (i+5) )/ rate) ;
                *(p + flag) = (vl & 0xFF);
                *(p + flag + 1) = ((vl >> 8) & 0xFF);
                *(p + flag + 2) = (vr & 0xFF);
                *(p + flag + 3) = ((vr >> 8) & 0xFF);
                flag += 4;
            }
        }
    }
    else
    {
        if (m_channels == 1)
        {
            for (int i = 0; i < len; i++)
            {
                *(p + i) = sin(i * (MATH_PI * 2) / rate * freq) * amp * 128 / 100 + 128;
            }
        }
        else
        {
            for (int i = 0; i < len; i++)
            {
                *(p + flag)= sin(i * (MATH_PI * 2) / rate * freq) * amp * 128 / 100+128;
                *(p + flag + 1)= sin((i+5) * (MATH_PI * 2) / rate * freq) * amp * 128 / 100+128;
                flag += 2;
            }
        }
    }
}

void CreateHead(int durations,int m_samplefreq,int m_channels,int m_channelbits,char *pWaveBuffer)
//频率、音量、持续时间、采样频率、通道数、通道比特数
{
    WaveHeader *pHeader = new WaveHeader;
    DWORD totalLen = (m_samplefreq * m_channels * m_channelbits / 8) * durations + 44;
    //文件总长度=(采样率 * 通道数 * 比特数 / 8) * 持续时间(s)+头文件长度
    pHeader->chRIFF[0] = 'R';
    pHeader->chRIFF[1] = 'I';
    pHeader->chRIFF[2] = 'F';
    pHeader->chRIFF[3] = 'F';
    pHeader->dwRIFFLen = totalLen - 8;//文件的总长度-8bits

    pHeader->chWAVE[0] = 'W';
    pHeader->chWAVE[1] = 'A';
    pHeader->chWAVE[2] = 'V';
    pHeader->chWAVE[3] = 'E';

    pHeader->chFMT[0] = 'f';
    pHeader->chFMT[1] = 'm';
    pHeader->chFMT[2] = 't';
    pHeader->chFMT[3] = ' ';

    //cout<<"Size of format:"<<sizeof(PCMWAVEFORMAT)<<endl;

    pHeader->dwFMTLen = sizeof(PCMWAVEFORMAT);
    //一般情况下Size为16，如果为18则最后多了2个字节的附加信息
    //pHeader->pwf.wf.wFormatTag = 0x01;//编码方式
    //pHeader->pwf.wf.nChannels = m_channels; //1为单通道，2为双通道
    //pHeader->pwf.wf.nSamplesPerSec = m_samplefreq;  //=44.1KHz
    //pHeader->pwf.wf.nAvgBytesPerSec = m_samplefreq * m_channels * m_channelbits / 8;//每秒所需字节数
    //pHeader->pwf.wf.nBlockAlign = m_channels * m_channelbits / 8;//一个采样的字节数
    pHeader->pwf.wFormatTag = 0x01;//编码方式
    pHeader->pwf.nChannels = m_channels; //1为单通道，2为双通道
    pHeader->pwf.nSamplesPerSec = m_samplefreq;  //=44.1KHz
    pHeader->pwf.nAvgBytesPerSec = m_samplefreq * m_channels * m_channelbits / 8;//每秒所需字节数
    pHeader->pwf.nBlockAlign = m_channels * m_channelbits / 8;//一个采样的字节数
    pHeader->pwf.wBitsPerSample = m_channelbits;//16位，即设置PCM的方式为16位立体声(双通道)
    //cout<<"Size of format:"<<sizeof(pHeader->pwf)<<endl;

    pHeader->chDATA[0] = 'd';
    pHeader->chDATA[1] = 'a';
    pHeader->chDATA[2] = 't';
    pHeader->chDATA[3] = 'a';
    pHeader->dwDATALen = totalLen - WAVE_HEAD_LENGTH;//数据的长度，=文件总长度-头长度(44bit)

    //pWaveBuffer = new char[totalLen]; //音频数据
    memcpy(pWaveBuffer, pHeader, WAVE_HEAD_LENGTH);
    //DWORD sample_tmp;
    //sample_tmp=pHeader->pwf.nSamplesPerSec;
    delete(pHeader);


    //MakeWaveData(pHeader->pwf.wf.nSamplesPerSec, freq, volume, pWaveBuffer+ WAVE_HEAD_LENGTH, m_samplefreq*durations);//采样点数
    //MakeWaveData(sample_tmp, freq, volume, pWaveBuffer+ WAVE_HEAD_LENGTH, m_samplefreq*durations,m_channelbits,m_channels);//采样点数
}
int WriteWavFile(string file_path,char* pWaveBuffer,DWORD totalLen)
{

    ofstream ocout;
    ocout.open(file_path, ios::out | ios::binary);//以二进制形式打开文件
    if (ocout)
    {
        ocout.write(pWaveBuffer, totalLen);
    	cout << "创建成功！" << endl;
    }
    else
    {
	cout << "创建失败！" << endl;
        return 0;
    }
    ocout.close();
    return 1;
}
/*
int main()
{
    if (CreateHead(10000, 100, 100,44100,2,8))
        cout << "创建成功！" << endl;
    else
        cout << "创建失败！" << endl;
    return 0;
}*/
