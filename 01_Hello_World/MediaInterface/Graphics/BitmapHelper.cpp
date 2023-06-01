#include "BitmapHelper.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>

using error = std::runtime_error;
using std::cout;
using std::endl;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::uint32_t;
using std::uint8_t;

namespace {

uint32_t ReadUnsignedInt(istream& in, long bytes)
{
    uint32_t value = 0;
    for(long i=0;i<bytes;i++)
        value |= uint8_t(in.get())<<(8*i);
    return value;
}

void WriteUnsignedInt(ostream& out, long bytes, uint32_t value)
{
    for(long i=0;i<bytes;i++)
        out.put((value>>(8*i))&0xff);
    return;
}

class BITMAPFILEHEADER
{
public:
    unsigned short bfType; 
    unsigned long  bfSize; 
    unsigned short bfReserved1; 
    unsigned short bfReserved2; 
    unsigned long  bfOffBits;

    void Read(istream& bitmapFile);
    void Write(ostream& bitmapFile) const;
};

class BITMAPINFOHEADER
{
public:
    unsigned long  biSize; 
    long           biWidth; 
    long           biHeight; 
    unsigned short biPlanes; 
    unsigned short biBitCount; 
    unsigned long  biCompression; 
    unsigned long  biSizeImage; 
    long           biXPelsPerMeter; 
    long           biYPelsPerMeter; 
    unsigned long  biClrUsed; 
    unsigned long  biClrImportant;

    void Read(istream& bitmapFile);
    void Write(ostream& bitmapFile) const;
};

void BITMAPFILEHEADER::Read(istream& bitmapFile)
{
    bfType      = ReadUnsignedInt(bitmapFile,2);
    bfSize      = ReadUnsignedInt(bitmapFile,4);
    bfReserved1 = ReadUnsignedInt(bitmapFile,2);
    bfReserved2 = ReadUnsignedInt(bitmapFile,2);
    bfOffBits   = ReadUnsignedInt(bitmapFile,4);

    if(bfType!=(((unsigned char)'M')<<8 | (unsigned char)'B'))
        throw error("BITMAPFILEHEADER.Read() : bfType is not 'BM'.");

    if(bfReserved1!=0 || bfReserved2!=0)
        throw error("BITMAPFILEHEADER.Read() : Reserved fields are not 0.");

    return;
}

void BITMAPFILEHEADER::Write(ostream& bitmapFile) const
{
    WriteUnsignedInt(bitmapFile,2,bfType);
    WriteUnsignedInt(bitmapFile,4,bfSize);
    WriteUnsignedInt(bitmapFile,2,bfReserved1);
    WriteUnsignedInt(bitmapFile,2,bfReserved2);
    WriteUnsignedInt(bitmapFile,4,bfOffBits);
    return;
}

void BITMAPINFOHEADER::Read(istream& bitmapFile)
{
    biSize = ReadUnsignedInt(bitmapFile,4);

    if(biSize!=40)//sizeof(BITMAPINFOHEADER))
        throw error("BITMAPINFOHEADER.Load() : Unsupported Info Header (expected size to be 40 bytes).");

    biWidth         = ReadUnsignedInt(bitmapFile,4);
    biHeight        = ReadUnsignedInt(bitmapFile,4);
    biPlanes        = ReadUnsignedInt(bitmapFile,2);
    biBitCount      = ReadUnsignedInt(bitmapFile,2);
    biCompression   = ReadUnsignedInt(bitmapFile,4);
    biSizeImage     = ReadUnsignedInt(bitmapFile,4);
    biXPelsPerMeter = ReadUnsignedInt(bitmapFile,4);
    biYPelsPerMeter = ReadUnsignedInt(bitmapFile,4);
    biClrUsed       = ReadUnsignedInt(bitmapFile,4);
    biClrImportant  = ReadUnsignedInt(bitmapFile,4);
    return;
}

void BITMAPINFOHEADER::Write(ostream& bitmapFile) const
{
    WriteUnsignedInt(bitmapFile,4,biSize);
    WriteUnsignedInt(bitmapFile,4,biWidth);
    WriteUnsignedInt(bitmapFile,4,biHeight);
    WriteUnsignedInt(bitmapFile,2,biPlanes);
    WriteUnsignedInt(bitmapFile,2,biBitCount);
    WriteUnsignedInt(bitmapFile,4,biCompression);
    WriteUnsignedInt(bitmapFile,4,biSizeImage);
    WriteUnsignedInt(bitmapFile,4,biXPelsPerMeter);
    WriteUnsignedInt(bitmapFile,4,biYPelsPerMeter);
    WriteUnsignedInt(bitmapFile,4,biClrUsed);
    WriteUnsignedInt(bitmapFile,4,biClrImportant);
    return;
}

} //End of namespace.

namespace mi
{

void BitmapHelper::SetDimensions(long width, long height)
{
    this->width  = width;
    this->height = height;
    data.resize(width*height*4);
    return;
}

void BitmapHelper::Load(const std::string& filename)
{
    unsigned long    byteCount;
    unsigned long    bytesPerRow;
    unsigned long    usefulBytesPerRow;
    long             width;
    long             height;
    ifstream         file;
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;

    file.open(filename,std::ios::binary);

    if(file.good()==false)
        throw error("BitmapHelper.Load() : Could not open file.");

    //Read header and info.
    bmfh.Read(file);
    bmih.Read(file);
    
    //Check for errors.
    if(bmih.biWidth<=0)
        throw error("BitmapHelper.Load() : Width is non-positive.");

    if(bmih.biHeight==0)
        throw error("BitmapHelper.Load() : Height is 0.");

    if(bmih.biPlanes!=1)
        throw error("BitmapHelper.Load() : Planes is not 1.");

    if(bmih.biBitCount!=24 && bmih.biBitCount!=32)
        throw error("BitmapHelper.Load() : Unsupported bits per pixel (only 24 and 32 are currently supported).");

    if(bmih.biCompression!=0) //0 = BI_RGB
        throw error("BitmapHelper.Load() : Compression is not supported (only BI_RGB (0) is supported).");

    if(bmfh.bfOffBits<54)
        throw error("BitmapHelper.Load() : Offset is less than 54 bytes.");

    //Move to the start of the Image Data
    for(unsigned long i=54;i<bmfh.bfOffBits;i++)
        file.get();

    //Set the width and height.
    width  = bmih.biWidth;
    height = bmih.biHeight;
    if(height<0)
        height = -height;

    SetDimensions(width, height);

    //Read in the pixel data.
    byteCount         = bmih.biBitCount/8;
    usefulBytesPerRow = width*byteCount;
    bytesPerRow       = (usefulBytesPerRow+3)/4*4; //UsefulBytesPerRow rounded up to the nearest multiple of 4.

    if (byteCount == 4)
    {
        for(long i=0;i<height;i++)
        {
            long m = (bmih.biHeight<0) ? i : height-1-i;

            for(long j=0;j<width;j++)
            {
                for(long k=0;k<3;k++)
                    data[j*4+width*m*4+2-k] = uint8_t(file.get());
                data[j*4+width*m*4+3] = uint8_t(file.get());
            }
        }
    }
    else
    {
        for(long i=0;i<height;i++)
        {
            long m = (bmih.biHeight<0) ? i : height-1-i;

            for(long j=0;j<width;j++)
            {
                for(long k=0;k<3;k++)
                    data[j*4+width*m*4+2-k] = uint8_t(file.get());
                data[j*4+width*m*4+3] = 255;
            }

            for(unsigned long j=0;j<bytesPerRow-usefulBytesPerRow;j++)
                file.get();
        }
    }

    file.close();
    return;
}

void BitmapHelper::Save(const std::string& filename) const
{    
    unsigned long    i, j;
    ofstream         file;
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;

    if(width<=0 || height<=0)
        throw error("BitmapHelper.Save() : Width and Height must be positive.");

    //Open the file.
    file.open(filename,std::ios::binary);

    if(file.good()==false)
        throw error("BitmapHelper.Save() : Could not open file.");

    //Set the bitmap file and info headers.

    bmfh.bfType          = 19778; //(unsigned short)('M'<<8 | 'B');
    bmfh.bfSize          = 54+height*width*4; //bmfh.bfOffBits+bmih.biSizeImage
    bmfh.bfReserved1     = 0;
    bmfh.bfReserved2     = 0;
    bmfh.bfOffBits       = 54;    //sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)

    bmih.biSize          = 40;    //sizeof(BITMAPINFOHEADER)
    bmih.biWidth         = width;
    bmih.biHeight        = height;
    bmih.biPlanes        = 1;
    bmih.biBitCount      = 32;
    bmih.biCompression   = 0;     //BI_RGB
    bmih.biSizeImage     = height*width*4; //Optionally 0
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed       = 0;
    bmih.biClrImportant  = 0;

    bmfh.Write(file);
    bmih.Write(file);

    //Write the pixel data.
    for(i=0;i<height;i++)
        for(j=0;j<width;j++)
        {
            file.put(data[j*4+(height-1-i)*width*4+2]);
            file.put(data[j*4+(height-1-i)*width*4+1]);
            file.put(data[j*4+(height-1-i)*width*4+0]);
            file.put(data[j*4+(height-1-i)*width*4+3]);
        }

    file.close();
    return;
}

} //End of namespace mi.