#include "intelvideosource.h"

#include <iostream>

using namespace std;

void main(int argc, char **argv){

    try{
        IntelVideoSource source;
        cout << "Created successfully: " << (bool)(source.isOpened()>0) << endl;

        cout << "picture sizes: " <<  source.width(IntelVideoSource::IMAGE_RGB) << ";" << source.height(IntelVideoSource::IMAGE_RGB) << endl;
        cout << "buffer sizes: " << source.expectedBufferSize(IntelVideoSource::IMAGE_DEPTH) << " "  << source.expectedBufferSize(IntelVideoSource::IMAGE_RGB) << endl;
        cout << "sizes in bytes: " << source.sizeInBytes((IntelVideoSource::ImageType)IntelVideoSource::IMAGE_RGB) << " " << source.sizeInBytes((IntelVideoSource::ImageType)IntelVideoSource::IMAGE_DEPTH) << endl;


        unsigned char *depth = new unsigned char[source.expectedBufferSize(IntelVideoSource::IMAGE_DEPTH)];
        unsigned char *rgb = new unsigned char[source.expectedBufferSize(IntelVideoSource::IMAGE_RGB)];

        cout << "Retrieve depth: " << source.retrieve(depth,IntelVideoSource::IMAGE_DEPTH) << endl;
        cout << "Retrieve RGB: " << source.retrieve(rgb,IntelVideoSource::IMAGE_RGB) << endl;
    }catch(IntelCameraException e){
        cout << e << endl;
    }



}
