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

/*
 * Represents an entry with number, start and end times and sentences in the subtitle file
 */

class StrInfo {
public:
	int number;					// No need to encapsulate
	StrTime startTime;
	StrTime endTime;
	std::string time_interval;
	std::list<std::string> lines;
};

#endif /* STRINFO_H_ */
