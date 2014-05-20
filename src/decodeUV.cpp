#include <opencv2/opencv.hpp>

#include <iostream>
#include <cstring>
#include <fstream>

using namespace cv;
using namespace std;

void process(const Mat &input, int tw, int th, Mat &Xout, Mat &Yout){

    int rows = input.rows, cols = input.cols/2;
    const float *inputptr;
    unsigned short *xoutptr,*youtptr;

    if(input.isContinuous() & Xout.isContinuous() & Yout.isContinuous()){
        cols *= rows;
        rows = 1;
    }

    for(int i=0; i<rows; i++){
        inputptr = input.ptr<const float>(i);
        xoutptr = Xout.ptr<unsigned short>(i);
        youtptr = Yout.ptr<unsigned short>(i);

        for(int j=0; j<cols; j++){
            xoutptr[j] = (unsigned short)(tw*inputptr[2*j]);
            youtptr[j] = (unsigned short)(th*inputptr[2*j+1]);
        }
    }
}

int main(int argc, char **argv)
{
    //first argument -- a file list with file names; filename.<ext> -> two files: filenameX.<ext>, filenameY.<ext>

    if (argc < 4){
        cout << "usage: <filename> <target width> <target height> [-v]" << endl
             << "the file with filename should contain filenames to process" << endl;
        cerr << "wrong number of input arguments; exiting" << endl;
        exit(-1);
    }

    ifstream in(argv[1]);
    int width = atoi(argv[2]), height = atoi(argv[3]);
    int filecount = 0;
    string fn;
    Mat input;
    Mat Xout,Yout;
    bool verbose = false;

    cout << "start processing a batch..." << endl;

    if (!in.is_open()){
        cerr << "problem reading the input file; exiting" << endl;
        exit(-2);
    }

    if (argc > 4 && strcmp(argv[4],"-v")==0){
        verbose = true;
    }

    while(!in.eof()){
        getline(in,fn);

        if (verbose){
            cout << "processing: " << fn << endl;
        }

        if (!fn.empty()){
            filecount++;
            input = imread(fn,-1); //load image as-is

            if (Xout.size() != input.size()){
                //allocate
                Xout = Mat(Size(input.size().width/2,input.size().height), CV_16UC1);
                Yout = Mat(Size(input.size().width/2,input.size().height), CV_16UC1);
            }

            process(input,width,height,Xout,Yout);

            imwrite(fn.substr(0,fn.length()-4)+"X"+fn.substr(fn.length()-4,4),Xout);
            imwrite(fn.substr(0,fn.length()-4)+"Y"+fn.substr(fn.length()-4,4),Yout);

            if (verbose){
                cout << "files saved: " << fn.substr(0,fn.length()-4)+"X"+fn.substr(fn.length()-4,4)
                     << ";" << fn.substr(0,fn.length()-4)+"Y"+fn.substr(fn.length()-4,4) << endl;
            }
        }
    }

    cout << filecount << " files processed" << endl;
    cout.flush();

    return 0;

}
