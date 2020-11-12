/*
 * displaySegment.cpp
 *
 *  Created on: Nov 9, 2020
 *      Author: blazer
 */

#include <tiffio.h>
#include <tiffio.hxx>
#include <sstream>
#include <iostream>
#include "displaySegment.h"
#include "pgsUtil.h"
#include "../bmp/bitmapUtil.h"

displaySegment::displaySegment()
{

}

displaySegment::displaySegment(presentationCompositionSegment pcs)
{
	this->pcs = pcs;
}

displaySegment::displaySegment(presentationCompositionSegment pcs, std::vector<windowDefinitionSegment> wds, std::vector<paletteDefinitionSegment> pds, std::vector<objectDefinitionSegment> ods, pgsSegment end) {
	this->pcs = pcs;
	this->wds = wds;
	this->pds = pds;
	this->ods = ods;
	this->end = end;
}

displaySegment::~displaySegment() {

}

void displaySegment::getTiff()
{
	std::ostringstream tiffStream;
	TIFF* temp = TIFFStreamOpen("temp", &tiffStream);
	TIFFClose(temp);
	//TIFF* temp = TIFFOpen("test.tiff", "w");
}

bitmap displaySegment::getBitmap()
{
	unsigned long * pixels = new unsigned long[this->ods[0].width * this->ods[0].height];
	pgsUtil::decodeRLE(pixels, this->pds[0], this->ods[0]);
	char * data = new char[this->ods[0].width * this->ods[0].height*4];
	for(unsigned long i = 0; i < this->ods[0].width * this->ods[0].height; i++){ bitmapUtil::numToChars(data+i*4, pixels[i], 4); }

	bitmapDIBHeaderV4 dib = bitmapDIBHeaderV4(this->ods[0].width, this->ods[0].height, this->ods[0].width * this->ods[0].height*4, BI_BITFIELDS);
	unsigned long fileSize = (BITMAP_FILEHEADER_SIZE + this->ods[0].width * this->ods[0].height*4 + dib.size);
	unsigned long offset = ((BITMAP_FILEHEADER_SIZE + dib.size));
	bitmapFileHeader fh = bitmapFileHeader(BM, fileSize, 0, 0, offset);
	return bitmap(fh, dib, data);
}

bitmap displaySegment::getRLEBitmap()
{
	bitmapColorTable ct = this->pds[0].getColorTable();
	char * data = this->ods[0].data;
	bitmapDIBHeaderV4 dib = bitmapDIBHeaderV4(this->ods[0].width, this->ods[0].height, this->ods[0].objectDataLength, BI_RLE8, ct.length);
	unsigned long fileSize = (BITMAP_FILEHEADER_SIZE + ct.length*4 + this->ods[0].objectDataLength + dib.size);
	unsigned long offset = ((BITMAP_FILEHEADER_SIZE + ct.length*4 + dib.size));
	bitmapFileHeader fh = bitmapFileHeader(BM, fileSize, 0, 0, offset);
	return bitmap(fh, dib, data, ct);
}
