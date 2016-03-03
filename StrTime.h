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
	std::string strtime;
	unsigned int timeInMsec;
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

	void printTime()
	{
		std::cout << timeInMsec << std::endl;;
	}

	std::string getPrintableTime()
	{
		std::string out;
		out.resize(20);
		int msec = this->timeInMsec % 1000;
		int hours = (this->timeInMsec / (60 * 60 * 1000));
		int minutes = (this->timeInMsec / (60 * 1000)) % 60;
		int seconds = (int) (this->timeInMsec / 1000) % 60;
		sprintf(&out[0], "%02d:%02d:%02d:%03d", hours, minutes, seconds, msec);
		//sprintf(&out[0], "%06d", timeInMsec);
		return out;
	}

	int cmpTime(StrTime ctime)
	{
		if (this->timeInMsec < ctime.timeInMsec)	return -1;
		else if (this->timeInMsec > ctime.timeInMsec)	return 1;
		else	return 0;
		/*
		if (ctime.hour > hour) return -1;
		else if (ctime.hour < hour)	return 1;
		else {
			if (ctime.minute > minute) return -1;
			else if (ctime.minute < minute)	return 1;
			else {
				if (ctime.sec > sec) return -1;
				else if (ctime.sec < sec)	return 1;
				else {
					if (ctime.sec > sec) return -1;
					else if (ctime.sec < sec)	return 1;
					else
						return 0;
				}
			}
		}
		*/
	}

	bool betweenTimes(StrTime start, StrTime end)
	{
		if (cmpTime(start) >= 0 && cmpTime(end) <= 0)
			return true;
		else
			return false;
	}

	virtual ~StrTime();
};

#endif /* STRTIME_H_ */
