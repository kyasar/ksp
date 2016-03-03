/*
 * StrInfo.h
 *
 *  Created on: Mar 1, 2016
 *      Author: kadir
 */

#ifndef STRINFO_H_
#define STRINFO_H_

#include <string>
#include <list>
#include "StrTime.h"

class StrInfo {
public:
	int number;
	StrTime startTime;
	StrTime endTime;
	std::string time_interval;
	std::list<std::string> lines;
	StrInfo();
	virtual ~StrInfo();
};

#endif /* STRINFO_H_ */
