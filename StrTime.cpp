/*
 * StrTime.cpp
 *
 *  Created on: Mar 1, 2016
 *      Author: kadir
 */

#include "StrTime.h"

std::string StrTime::getPrintableTime()
{
	std::string out;
	out.resize(20);
	int msec = this->timeInMsec % 1000;
	int hours = (this->timeInMsec / (60 * 60 * 1000));
	int minutes = (this->timeInMsec / (60 * 1000)) % 60;
	int seconds = (int) (this->timeInMsec / 1000) % 60;
	sprintf(&out[0], "%02d:%02d:%02d:%03d", hours, minutes, seconds, msec);
	return out;
}

int StrTime::cmpTime(StrTime ctime)
{
	if (this->timeInMsec < ctime.timeInMsec)	return -1;
	else if (this->timeInMsec > ctime.timeInMsec)	return 1;
	else	return 0;
}

/*
 * This function checks the time is between given two times
 */
bool StrTime::betweenTimes(StrTime start, StrTime end)
{
	if (cmpTime(start) >= 0 && cmpTime(end) <= 0)
		return true;
	else
		return false;
}
