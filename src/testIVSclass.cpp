#include "intelvideosource.h"

#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

#include "syncqueue.h"

using namespace std;
using namespace cv;

template <typename T>
string num2str ( T num )
{
    ostringstream ss;
    ss << num;
    return ss.str();
}

struct DataFrame{
    DataFrame(){

    }


    unsigned char *depth_;
    unsigned char *uvmap_;
    unsigned char *rgb_;
};

void main(int argc, char **argv){

    try{
        IntelVideoSource source;
        cout << "Created successfully: " << (bool)(source.isOpened()>0) << endl;

        cout << "picture sizes: " <<  source.width(IntelVideoSource::IMAGE_RGB) << ";" << source.height(IntelVideoSource::IMAGE_RGB) << endl;
        cout << "buffer sizes: " << source.expectedBufferSize(IntelVideoSource::IMAGE_DEPTH) << " "  << source.expectedBufferSize(IntelVideoSource::IMAGE_RGB) << endl;
        cout << "sizes in bytes: " << source.sizeInBytes((IntelVideoSource::ImageType)IntelVideoSource::IMAGE_RGB) << " " << source.sizeInBytes((IntelVideoSource::ImageType)IntelVideoSource::IMAGE_DEPTH) << endl;


        unsigned char *depth[3];
        depth[0] = new unsigned char[source.expectedBufferSize(IntelVideoSource::IMAGE_DEPTH)];
        depth[1] = new unsigned char[source.expectedBufferSize(IntelVideoSource::IMAGE_CONFIDENCE)];
        depth[2] = new unsigned char[source.expectedBufferSize(IntelVideoSource::IMAGE_UV)];
        unsigned char *rgb[1];
        rgb[0] = new uchar[source.expectedBufferSize(IntelVideoSource::IMAGE_RGB)];

        cout << "Retrieve depth: " << source.retrieve(depth,IntelVideoSource::IMAGE_DEPTH) << endl;
        cout << "Retrieve RGB: " << source.retrieve(rgb,IntelVideoSource::IMAGE_RGB) << endl;

        /*allocate video buffer*/
        int N=1000;
        //unsigned short **depthBuf = new unsigned short*[N];
        //unsigned char **rgbBuf = new uchar*[N];

        int height = source.height(IntelVideoSource::IMAGE_DEPTH);
        int width = source.width(IntelVideoSource::IMAGE_DEPTH);
        string filename;
        Mat container;

        /*
        for(int i=0; i<N; i++){
            depthBuf[i] = new unsigned short[source.width(IntelVideoSource::IMAGE_DEPTH)*source.height(IntelVideoSource::IMAGE_DEPTH)];
            rgbBuf[i] = new uchar[source.expectedBufferSize(IntelVideoSource::IMAGE_RGB)];
        }
        */

        for(int i=0; i<N; i++){
            source.retrieve(depth,IntelVideoSource::IMAGE_DEPTH);
            source.retrieve(rgb,IntelVideoSource::IMAGE_RGB);
            //memcpy((char *)depthBuf[i],depth[0],source.expectedBufferSize(IntelVideoSource::IMAGE_DEPTH));
            //memcpy((char *)rgbBuf[i],rgb[0],source.expectedBufferSize(IntelVideoSource::IMAGE_RGB));

            container = Mat(height,width,CV_16UC1,depth[0]);
            filename = "D" + num2str(i) + ".png";
            imwrite(filename,container);

            container = Mat(source.height(IntelVideoSource::IMAGE_RGB),source.width(IntelVideoSource::IMAGE_RGB),CV_8UC3,rgb[0]);
            filename = "RGB" + num2str(i) + ".png";
            imwrite(filename,container);

            container = Mat(height,2*width,CV_8UC4,depth[2]);
            filename = "UV" + num2str(i) + ".png";
            imwrite(filename,container);
        }

        cout << "Finished recording images" << endl;

        /*

        for(int i=0; i<N; i++){
            Mat container = Mat(height,width,CV_16UC1,depthBuf[i]);

            filename = "D" + num2str(i) + ".png";

            imwrite(filename,container);

            container = Mat(source.height(IntelVideoSource::IMAGE_RGB),source.width(IntelVideoSource::IMAGE_RGB),CV_8UC3,rgbBuf[i]);

            filename = "RGB" + num2str(i) + ".png";

            imwrite(filename,container);
        }

        */

    }catch(IntelCameraException e){
        cout << e << endl;
    }



}
