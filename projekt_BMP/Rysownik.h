#include "bmp.h"
#include <cassert>
#include <fstream>
#include <cstring>
#include <cmath>
#include "Interpreter.h"

using namespace std;
#define PI 3.14

BMP::BMP(uint16_t width, uint16_t height) :
		bitmapCoreHeader(width, height) {

	assert(IS_LITTLE_ENDIAN);
	assert(width > 0);
	assert(height > 0);

	const unsigned int rowSize = ((bitmapCoreHeader.bitsPerPixel * width + 31)
			/ 32) * 4;
	const unsigned int imgSize = rowSize * height;

	bmpFileHeader.size = 14 + bitmapCoreHeader.size + imgSize;
	bmpFileHeader.dataOffset = 14 + bitmapCoreHeader.size;

	pixelData = new unsigned char[imgSize];
	std::memset(pixelData, 255, imgSize);
}

BMP::~BMP() {
	delete[] pixelData;
}

void BMP::setPixel(uint16_t x, uint16_t y, unsigned char r, unsigned char g,
		unsigned char b) {
	assert(bitmapCoreHeader.bitsPerPixel == 24);

	const size_t rowSize = ((bitmapCoreHeader.bitsPerPixel
			* bitmapCoreHeader.bmpWidth + 31) / 32) * 4;
	const size_t offset = rowSize * (bitmapCoreHeader.bmpHeight - y)
			+ x * (bitmapCoreHeader.bitsPerPixel / 8);

	pixelData[offset + 0] = b;
	pixelData[offset + 1] = g;
	pixelData[offset + 2] = r;
}

std::ostream& operator<<(std::ostream& os, const BMP& bmp) {
	os.write(bmp.bmpFileHeader.id, sizeof(bmp.bmpFileHeader.id));
	os.write((const char*) &bmp.bmpFileHeader.size,
			sizeof(bmp.bmpFileHeader.size));
	os.write(bmp.bmpFileHeader.reserved, sizeof(bmp.bmpFileHeader.reserved));
	os.write((const char*) &bmp.bmpFileHeader.dataOffset,
			sizeof(bmp.bmpFileHeader.dataOffset));

	os.write((const char*) &bmp.bitmapCoreHeader.size,
			sizeof(bmp.bitmapCoreHeader.size));
	os.write((const char*) &bmp.bitmapCoreHeader.bmpWidth,
			sizeof(bmp.bitmapCoreHeader.bmpWidth));
	os.write((const char*) &bmp.bitmapCoreHeader.bmpHeight,
			sizeof(bmp.bitmapCoreHeader.bmpHeight));
	os.write((const char*) &bmp.bitmapCoreHeader.colorPlanes,
			sizeof(bmp.bitmapCoreHeader.colorPlanes));
	os.write((const char*) &bmp.bitmapCoreHeader.bitsPerPixel,
			sizeof(bmp.bitmapCoreHeader.bitsPerPixel));

	const unsigned int rowSize = ((bmp.bitmapCoreHeader.bitsPerPixel
			* bmp.bitmapCoreHeader.bmpWidth + 31) / 32) * 4;
	const unsigned int imgSize = rowSize * bmp.bitmapCoreHeader.bmpHeight;

	os.write((const char*) bmp.pixelData, imgSize);

	return os;
}

struct point{
    int x;
    int y;
    point(int xx, int yy) : x(xx), y(yy) {}
};

const uint16_t imgWidth = 800;
const uint16_t imgHeight = 600;

BMP bmp(imgWidth, imgHeight);

class Rysownik {
public:
    virtual void odcinek(point from, point to, unsigned char r, unsigned char g,unsigned char b)=0;
    virtual void okrag_kolo(point mid, int rad, float alfa1, float alfa2,bool fill,
                            unsigned char r, unsigned char g,unsigned char b)=0;
};

class RysownikImplementacja : public Rysownik {
    Interpreter O;
public:
    RysownikImplementacja (const char* file) : O(file) {}
    void odcinek(point from, point to, unsigned char r, unsigned char g,unsigned char b) {
        if (from.x == to.x) {
            for (int y = from.y; y <= to.y; y++) {
                bmp.setPixel(from.x, y, r, b, g);
            }
        } else if (from.x > to.x) {
            float a = (float) (to.y - from.y) / (float) (to.x - from.x);
            float c = (from.y - (a * from.x)); // parametry do funkcji liniowej
            int z = (int) (from.x * a + c);
            for (int x = from.x; x >= to.x; x--) {
                int y=(int)((x * a) + c);
                if(x>0 && x<imgWidth && y>0 && y<imgHeight) bmp.setPixel(x, y, r, g, b);
                if(from.y>to.y){
                    for (z; z >= y; z--) {
                        if(x>0 && x<imgWidth && z>0 && z<imgHeight) bmp.setPixel(x, z, r, g, b);
                    }
                    z =  y - 1;
                }else {
                    for (z; z <= (int) (x * a + c); z++) {
                        if(x>0 && x<imgWidth && z>0 && z<imgHeight) bmp.setPixel(x, z, r, g, b);
                    }
                    z = y + 1;
                }// z obsluguje dziury miedzy pikselami
            }
        } else {
            float a = (float) (to.y - from.y) / (float) (to.x - from.x);
            float c = (from.y - (a * from.x));
            int z = (int) (from.x * a + c);
            for (int x = from.x; x <= to.x; ++x) {
                int y=(int)((x * a) + c);
                if(x>0 && x<imgWidth && y>0 && y<imgHeight) bmp.setPixel(x, y, r, g, b);
                if(from.y>to.y){
                    for (z; z >= y; --z) {
                        if(x>0 && x<imgWidth && z>0 && z<imgHeight) bmp.setPixel(x, z, r, g, b);
                    }
                    z = y-1;
                }else {
                    for (z; z <= y; ++z) {
                        if(x>0 && x<imgWidth && z>0 && z<imgHeight) bmp.setPixel(x, z, r, g, b);
                    }
                    z = y+1;
                }
            }
        }
    }

    void okrag_kolo(point mid, int rad, float alfa1, float alfa2,bool fill,
                    unsigned char r, unsigned char g,unsigned char b){
        float one_deg=(float)PI/180;
        double prec=one_deg/rad;
        float i=alfa1*one_deg;
        int z=mid.x+(int)(rad*cos(i))-1;
        int q=mid.y+(int)(rad*sin(i))+1;
        while(i<=alfa2*one_deg+1){
            int x=mid.x+(int)(rad*cos(i));
            int y=mid.y+(int)(rad*sin(i));
            if(fill) odcinek(point(mid.x, mid.y), point(x,y), r, g, b);
            if(x>0 && x<imgWidth && y>0 && y<imgHeight) bmp.setPixel(x, y, r, g, b);
            if(i>=0 && i<PI/2) {
                for (q; q <= y; q++) {
                    if(x>0 && x<imgWidth && q>0 && q<imgHeight) bmp.setPixel(x, q, r, g, b);
                }
                for (z; z >= x; z--) {
                    if(z>0 && z<imgWidth && y>0 && y<imgHeight) bmp.setPixel(z, y, r, g, b);
                }
                q = y;
                z=x;
            }
            else if(i>=PI/2 && i<PI) {
                for (q; q >= y; q--) {
                    if(x>0 && x<imgWidth && q>0 && q<imgHeight) bmp.setPixel(x, q, r, g, b);
                }
                for (z; z >= x; z--) {
                    if(z>0 && z<imgWidth && y>0 && y<imgHeight) bmp.setPixel(z, y, r, g, b);
                }
                q=y;
                z = x;
            }
            else if(i>=PI && i<PI*3/2) {
                for (q; q >= y; q--) {
                    if(x>0 && x<imgWidth && q>0 && q<imgHeight) bmp.setPixel(x, q, r, g, b);
                }
                for (z; z <= x; z++) {
                    if(z>0 && z<imgWidth && y>0 && y<imgHeight) bmp.setPixel(z, y, r, g, b);
                }
                q=y;
                z = x;
            }
            else if(i>=PI*3/2 && i<=2*PI+1) {
                for (q; q <= y; q++) {
                    if(x>0 && x<imgWidth && q>0 && q<imgHeight) bmp.setPixel(x, q, r, g, b);
                }
                for (z; z <= x; z++) {
                    if(z>0 && z<imgWidth && y>0 && y<imgHeight) bmp.setPixel(z, y, r, g, b);
                }
                q=y;
                z = x;
            }
            i+=prec;
        }
        if(fill) odcinek(point(mid.x-1, mid.y), point(mid.x, mid.y+(int)(rad*cos(3/2*PI))), r, g, b);
    }
    void init(){
        for(int i=0; i<O.lines.size(); i++){
            odcinek(point(O.lines[i]->x1, O.lines[i]->y1), point(O.lines[i]->x2, O.lines[i]->y2),
                    O.lines[i]->r, O.lines[i]->g, O.lines[i]->b);
        }
        for(int i=0; i<O.circles.size(); i++){
            okrag_kolo(point(O.circles[i]->x, O.circles[i]->y), O.circles[i]->R, O.circles[i]->a1,
            O.circles[i]->a2, O.circles[i]->F, O.circles[i]->r, O.circles[i]->g, O.circles[i]->b);
        }
    }
};
