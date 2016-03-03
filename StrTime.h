/*
 * StrTime.h
 *
 *  Created on: Mar 1, 2016
 *      Author: kadir
 */

#ifndef STRTIME_H_
#define STRTIME_H_

#include <string>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdlib.h>

class StrTime {
public:
	std::string strtime;		// No need to encapsulate
	unsigned int timeInMsec;	// Just keep in milliseconds

	StrTime() {
		resetTime();
	};

	void incMillisecs() {
		timeInMsec++;
	}

	void resetTime()
	{
		timeInMsec = 0;
	}

	void setTime(int h, int m, int s, int ms)
	{
		this->timeInMsec = (((((h * 60 ) + m) * 60) + s) * 1000 + ms);
	}

	std::string getPrintableTime();
	int cmpTime(StrTime ctime);
	bool betweenTimes(StrTime start, StrTime end);
};

#endif /* STRTIME_H_ */
