
/*Copyright (c) <2009> <Benedikt Zoefel>

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.*/

/* mostly taken from
 BMP Loader - Codehead 08/11/04
 http://gpwiki.org/index.php/LoadBMPCpp   */

#ifndef BITMAP_LOADER_H
#define BITMAP_LOADER_H

#include <iostream>
#include <fstream>
#include <string>
#include <memory.h>
 
#define IMG_OK              0x1
#define IMG_ERR_NO_FILE     0x2
#define IMG_ERR_MEM_FAIL    0x4
#define IMG_ERR_BAD_FORMAT  0x8
#define IMG_ERR_UNSUPPORTED 0x40
 
using namespace std;

class BMPImg
 {
  public:
   BMPImg();
   ~BMPImg();
   int Load(char* szFilename);
   int GetBPP();
   int GetWidth();
   int GetHeight();
   unsigned char* GetImg();       // Return a pointer to image data
   unsigned char* GetPalette();   // Return a pointer to VGA palette
 
  private:
   unsigned int iWidth,iHeight,iEnc;
   short int iBPP,iPlanes;
   int iImgOffset,iDataSize;
   unsigned char *pImage, *pPalette, *pData;
   
   // Internal workers
   int GetFile(char* szFilename);
   int ReadBmpHeader();
   int LoadBmpRaw();
   int LoadBmpRLE8();
   int LoadBmpPalette();
   void FlipImg(); // Inverts image data, BMP is stored in reverse scanline order
 };
 
string GetPath();  /* get the path of the bitmap files from pybci setup.cfg file */

#endif
 
