/*
 * objectDefinitionSegment.cpp
 *
 *  Created on: Nov 4, 2020
 *      Author: blazer
 */

#include "objectDefinitionSegment.h"
#include "pgsUtil.h"
#include <cstring>

objectDefinitionSegment::objectDefinitionSegment(char * objectID, char * objectVersionNumber, char * lastInSequenceFlag, char * objectDataLength, char * width, char * height, char * data)
{
	this->objectID = pgsUtil::char2ToInt(objectID);
	this->objectVersionNumber = pgsUtil::cleanChar(objectVersionNumber[0]);
	this->lastInSequenceFlag = pgsUtil::cleanChar(lastInSequenceFlag[0]);
	this->objectDataLength = pgsUtil::char3ToLong(objectDataLength);
	this->width = pgsUtil::char2ToInt(width);
	this->height = pgsUtil::char2ToInt(height);
	this->data = new char [this->objectDataLength];
	memcpy(this->data, data, this->objectDataLength);
}

objectDefinitionSegment::~objectDefinitionSegment() {
	// TODO Auto-generated destructor stub
}
