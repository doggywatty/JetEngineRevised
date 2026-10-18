// libs/social_framework/src/imageconverter.h
#ifndef LIBS_SOCIAL_FRAMEWORK_SRC_IMAGECONVERTER_H
#define LIBS_SOCIAL_FRAMEWORK_SRC_IMAGECONVERTER_H

class ImageConverter {  // sizeof 0x1
public:
    static unsigned char* GetImageData(unsigned char*, int, int*, int*, int*, int*);
private:
    static int padImageToPowerOf2(unsigned char**, unsigned char**, int, int, int);
};

#endif
