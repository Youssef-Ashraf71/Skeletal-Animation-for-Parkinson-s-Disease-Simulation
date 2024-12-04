#ifndef GETBMP_H
#define GETBMP_H

#include <string>  // Include the necessary header for std::string

// Define a structure to represent an image file with width, height, and pixel data.
struct imageFile
{
    int width;
    int height;
    unsigned char *data;
};

// Declare a function prototype for getBMP, which takes a file name and returns an imageFile pointer.
imageFile *getBMP(std::string fileName);

#endif
