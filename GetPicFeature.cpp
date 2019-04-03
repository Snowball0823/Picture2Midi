#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "include/MidiFile.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <cmath>
#define MIDICHANNEL 14

using namespace cv;
using namespace std;

typedef unsigned char uchar;
typedef struct SpeedDiff
{
    double speedDiff;
    int num;
} SD;

void rgb_to_hsv(Vec3b *p);
uchar channelSelect(uchar a,uchar b,uchar c,string args);
void get_newdata(Vec3b *oldData, int cols, int *newData);
void get_newdata(Vec3b *oldData, int cols, int *newData, int padding_cols);
void get_tonguevalue(vector<vector<int>> *tongueData, int rows, vector<vector<int>> *hueData, int maxHue, int minHue);
void get_speedvalue(vector<vector<int>> *speedData, int rows, vector<vector<int>> *diffData, int maxDiff, int minDiff);
void get_svalue(vector<vector<int>> *speedAll, int *s);
bool find_element(vector<SD> *SDsequence, double diffSpeed, double *flagSpeed);
//void write_midifile();

int main(int argc, char const* const* argv) {
    int midi_cols,padding_cols,minHue=360,maxHue=0,minDiff=360,maxDiff=0;
    int S;
    //int tongueNull=-1;
    string file_path;
    cout << "Plese enter the file path\n";
    cin >> file_path;
    file_path.erase(file_path.find_last_not_of(" ") + 1);
    Mat origin_image,image_grey;
    origin_image = imread(file_path, IMREAD_COLOR);
    vector<vector<int>>new_data(origin_image.rows);
    vector<vector<int>>hue_cols(MIDICHANNEL);
    vector<vector<int>>piexl_diff(MIDICHANNEL);
    vector<vector<int>>tongueAll(origin_image.rows);
    vector<vector<int>>speedAll(origin_image.rows-1);
    if(origin_image.data!=NULL){
    	cout << "rows:" << origin_image.rows << ",cols:" << origin_image.cols << endl;
	midi_cols = origin_image.cols/MIDICHANNEL + 1;
	padding_cols = midi_cols*MIDICHANNEL-origin_image.cols;
	cout << "pad cols:" << padding_cols << ",midi cols:" << midi_cols << endl;
	//cout << "Hue values:" << endl;
	for(int i=0; i<origin_image.rows;i++){
	    new_data[i].resize(MIDICHANNEL);
	    tongueAll[i].resize(MIDICHANNEL);
	    if(i != origin_image.rows-1) speedAll[i].resize(MIDICHANNEL);
	    Vec3b *tmp_data = origin_image.ptr<Vec3b>(i);
	    for(int j=0; j<origin_image.cols; j++){
		//BGR->HSV and get average number
		rgb_to_hsv(&(tmp_data[j]));
		if(j%midi_cols == 0 && j != 0){
		    if(hue_cols[j/midi_cols-1].size() == 0)
		      hue_cols[j/midi_cols-1].resize(origin_image.rows);
		    if(piexl_diff[j/midi_cols-1].size() == 0)
		      piexl_diff[j/midi_cols-1].resize(origin_image.rows-1);
		    //cout<<"hue_cols:"<<hue_cols[j].size()<<endl;
		    get_newdata(tmp_data, j, &(new_data[i][j/midi_cols-1]));
		    hue_cols[j/midi_cols-1][i] = new_data[i][j/midi_cols-1];
		    if(i>0){ 
			piexl_diff[j/midi_cols-1][i-1] = abs(hue_cols[j/midi_cols-1][i] - hue_cols[j/midi_cols-1][i-1]);
			if(piexl_diff[j/midi_cols-1][i-1] < minDiff) minDiff = piexl_diff[j/midi_cols-1][i-1];
			if(piexl_diff[j/midi_cols-1][i-1] > maxDiff) maxDiff = piexl_diff[j/midi_cols-1][i-1];
		    }
		    if(new_data[i][j/midi_cols-1] < minHue) minHue = new_data[i][j/midi_cols-1];
		    if(new_data[i][j/midi_cols-1] > maxHue) maxHue = new_data[i][j/midi_cols-1];
		}
	    }
	    if(piexl_diff[MIDICHANNEL-1].size() == 0)
	      piexl_diff[MIDICHANNEL-1].resize(origin_image.rows-1);
	    if(hue_cols[MIDICHANNEL-1].size() == 0)
	      hue_cols[MIDICHANNEL-1].resize(origin_image.rows);
	    get_newdata(tmp_data, origin_image.cols, &(new_data[i][MIDICHANNEL-1]), padding_cols);
	    hue_cols[MIDICHANNEL-1][i] = new_data[i][MIDICHANNEL-1];
	    if(i>0){
		piexl_diff[MIDICHANNEL-1][i-1] = abs(hue_cols[MIDICHANNEL-1][i] - hue_cols[MIDICHANNEL-1][i-1]);
		if(piexl_diff[MIDICHANNEL-1][i-1] < minDiff) minDiff = piexl_diff[MIDICHANNEL-1][i-1];
		if(piexl_diff[MIDICHANNEL-1][i-1] > maxDiff) maxDiff = piexl_diff[MIDICHANNEL-1][i-1];
	    }
	    if(new_data[i][MIDICHANNEL-1] < minHue) minHue = new_data[i][MIDICHANNEL-1];
	    if(new_data[i][MIDICHANNEL-1] > maxHue) maxHue = new_data[i][MIDICHANNEL-1];
	    /*cout << "Line " << i << ":" << endl;
	    for(int j=0; j<MIDICHANNEL;j++){
		cout << new_data[i][j] <<" ";
	    }
	    cout << endl;*/
	}
	//cout<<"maxHue:"<<maxHue<<" minHue:"<<minHue<<endl;
    }else{
	cout << "path wrong!" << endl;
    }
    //get the tongue
    get_tonguevalue(&tongueAll, origin_image.rows, &new_data, maxHue, minHue);
    get_speedvalue(&speedAll, origin_image.rows, &piexl_diff, maxDiff, minDiff);
    get_svalue(&speedAll, &S);
    cout << "Tongue values:" << endl;
    for(int i=0; i<tongueAll.size(); i++)
    {
	cout << "line " << i+1 << ":" << endl;
	for(int j=0; j<tongueAll[i].size(); j++)
	  cout << tongueAll[i][j] << " ";
	cout << endl;
    }
    cout << "maxDiff: " << maxDiff << " minDiff: " << minDiff << endl;
    cout << "Speed values:" << endl;
    for(int i=0; i<speedAll.size(); i++)
    {
	cout << "line " << i+1 << ":" << endl;
	for(int j=0; j<speedAll[i].size(); j++)
	  cout << speedAll[i][j] << " ";
	cout << endl;
    }
    cout << "S: " << S << endl;
    return 0;
}

void rgb_to_hsv(Vec3b *p)
{
    uchar B = (*p)[0];
    uchar G = (*p)[1];
    uchar R = (*p)[2];
    uchar H,S,V;
    //cout << "B:" << (int)B << " G:" << (int)G << " R:" << (int)R << endl;
    uchar maxRGB = channelSelect(R,G,B,"max");
    uchar minRGB = channelSelect(R,G,B,"min");
    uchar delta = maxRGB - minRGB;
    if((int)delta == 0)
    {
	H = 0;
	S = 0;
	V = maxRGB;
    }else{
	if(maxRGB == R){
	    H = (double) ((double)(G-B)/(double)delta*60);
	}
	else if(maxRGB == G){
	    H = (double) ((2+(double)(G-B)/(double)delta)*60);
	}
	else{
	    H = (double) ((4+(double)(G-B)/(double)delta)*60);
	}
	if(H < 0) H += 360;
	S = (double) ((double)delta/(double)maxRGB);
	V = maxRGB;
    }
    (*p)[0] = H;
    (*p)[1] = S;
    (*p)[2] = V;
    //cout << "maxRGB:" << (int)maxRGB << ",minRGB:" << (int)minRGB << endl;
    //cout << "H:" << (double)H << " S:" << (double)S << " V:" << (double)V << endl;
}

uchar channelSelect(uchar a,uchar b,uchar c,string args)
{
    if(strcmp(args.c_str(),"max")==0){
    	if(a>b)b=a;
    	if(b>c)c=b;
    	return c;
    }else if(strcmp(args.c_str(),"min")==0){
	if(a<b)b=a;
    	if(b<c)c=b;
    	return c;
    }
    return false;
}

void get_newdata(Vec3b *oldData, int cols, int *newData)
{
    double H = 0, S = 0, V = 0;
    for(int i=0; i<cols; i++)
    {
	H += (int)oldData[i][0];
	S += (int)oldData[i][1];
	V += (int)oldData[i][2];
	//cout << "old_H:" << (double)oldData[i][0] << " old_S:" << (double)oldData[i][1] << " old_V:" << (double)oldData[i][2] << endl;
	//cout<<"H period"<<(double)H<<" S period"<<(double)S<<" V period"<<(double)V<<endl;
    }
    //cout<<"H amount"<<(double)H<<" S amount"<<(double)S<<" V amount"<<(double)V<<endl;
    H = (double)H / (double)cols;
    S = (double)S / (double)cols;
    V = (double)V / (double)cols;
    //cout<<"H average"<<(double)H<<" S average"<<(double)S<<" V average"<<(double)V<<endl;
    (*newData) = (int)H; 
}

void get_newdata(Vec3b *oldData, int cols, int *newData, int padding_cols)
{
    double H = 0, S = 0, V = 0;
    for(int i=0; i<cols+padding_cols; i++)
    {
	H += (int)oldData[i][0];
	S += (int)oldData[i][1];
	V += (int)oldData[i][2];
    }
    H = (double)H / (double)(cols+padding_cols);
    S = (double)S / (double)(cols+padding_cols);
    V = (double)V / (double)(cols+padding_cols);
    (*newData) = (int)H;
}

void get_tonguevalue(vector<vector<int>> *tongueData, int rows, vector<vector<int>> *hueData, int maxHue, int minHue)
{
    //cout << "Tongue values:" << endl;
    int tongueLabel=-1;
    if(maxHue == minHue){
	tongueLabel = 1;
    }
    for(int i=0; i<rows;i++){
	//cout << "Line " << i << ":" << endl;
	for(int j=0; j<MIDICHANNEL; j++){
	    if(tongueLabel == -1){
		(*tongueData)[i][j] = (int)((double)((*hueData)[i][j]-minHue)*87/(double)(maxHue-minHue)+0.5);
	    }else{
		(*tongueData)[i][j] = 0;
	    }
	    //cout << (*tongueData)[i][j] << " ";
	    }
	//cout << endl;
	}
}

void get_speedvalue(vector<vector<int>> *speedData, int rows, vector<vector<int>> *diffData, int maxDiff, int minDiff)
{
    //cout << "Speed values:" << endl;
    int diffLabel=-1;
    if(maxDiff == minDiff){
	diffLabel = 1;
    }
    for(int i=0; i<rows-1; i++){
	//cout << "Line " << i << ":" << endl;
	for(int j=0; j<MIDICHANNEL; j++){
	    if(diffLabel == -1){
		(*speedData)[i][j] = (int)((double)((*diffData)[j][i]-minDiff)*10/(double)(maxDiff-minDiff)+0.5);
	    }else{
		(*speedData)[i][j] = 0;
	    }
	    //cout << (*speedData)[i][j] << " ";
	    }
	//cout << endl;
	}
}

void get_svalue(vector<vector<int>> *speedAll, int *s)
{
    double maxSpeedDiff=0, minSpeedDiff=10, flagSpeedDiff, sTmp;
    SD diffTmp; 
    vector<SD> sortSequen;
    for(int i=0; i<(*speedAll).size(); i++)
    {
	for(int j=0; j<(*speedAll)[i].size(); j++)
	{
	    if(j>0){
		diffTmp.speedDiff = (*speedAll)[i][j] - (*speedAll)[i][j-1];
		if(diffTmp.speedDiff<minSpeedDiff) minSpeedDiff = diffTmp.speedDiff;
		if(diffTmp.speedDiff>maxSpeedDiff) maxSpeedDiff = diffTmp.speedDiff;
		if(!find_element(&sortSequen, diffTmp.speedDiff, &flagSpeedDiff))
		{
		    diffTmp.num = 1;
		    sortSequen.push_back(diffTmp);
		}
	    }
	}
    }
    (*s) = (int)((flagSpeedDiff-minSpeedDiff)*80/(maxSpeedDiff-minSpeedDiff)+120);
}

bool find_element(vector<SD> *SDsequence, double diffSpeed, double *flagSpeed)
{
    bool finded=false;
    int maxNum=0;
    for(int i=0; i<(*SDsequence).size(); i++)
    {
	if((*SDsequence)[i].speedDiff == diffSpeed)
	{
	    (*SDsequence)[i].num++;
	    finded=true;
	}
	if((*SDsequence)[i].num>maxNum){
	    maxNum = (*SDsequence)[i].num;
	    (*flagSpeed) =  (*SDsequence)[i].speedDiff;
	}
    }
    return finded;
}
