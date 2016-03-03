//============================================================================
// Name        : main.cpp
// Author      : Kadir Yasar
// Version     :
// Copyright   : MIT License
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <ctype.h>
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <list>
#include <cmath>

#include "StrInfo.h"
#include "StrTime.h"

using namespace std;

#define SRT_FILE_NAME_LEN 	256
#define ESC_BTN		27

const std::string WHITESPACE = " \n\r\t";

static StrTime movie_time;
int counter = 0, lines_printed = 0;
std::list<StrInfo*> strInfoList;
std::list<StrInfo*>::iterator iter;
struct itimerval timer;
struct sigaction sa;
int totalPrinted = 0;
bool currPrinted = false;
int executed = 0;
int movie_end_time = 0;

void movie_player(int);
bool timerStopped = false;

/*
 * Movie timer pause/play methods
 */
void pauseMovie(void)
{
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
	timerStopped = true;
}

void continueMovie()
{
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &movie_player;
	sigaction (SIGALRM, &sa, NULL);	// POSIX signal

	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1000;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000;	// every milliseconds

	setitimer (ITIMER_REAL, &timer, NULL);
	timerStopped = false;
}

void togglePlaying(void)
{
	if (timerStopped)	continueMovie();
	else	pauseMovie();
}

void clearLatestLinesOnScreen()
{
	for (int i = 0; i < lines_printed; i++) {
		move(14 + i, 0);
		clrtoeol();
	}
	lines_printed = 0;
}

/*
 * This method is invoked when fast forward or backward operation is needed
 * It finds the next subtitle entry on the list according to updated movie time
 */
void findNextSubtitle(int direction)
{
	currPrinted = false;
	clearLatestLinesOnScreen();
	/*
	 * FORWARD
	 */
	if (direction) {
		while (movie_time.cmpTime((*iter)->endTime) > 0) {
			if (iter != --strInfoList.end()) {
				iter++;
				mvprintw(10, 10, "N: %2d, Total shown: %2d", (*iter)->number, totalPrinted);
			} else
				break;
		}
	}
	/*
	 * BACKWARD
	 */
	else {
		while (movie_time.cmpTime((*iter)->startTime) < 0) {
			if (iter != strInfoList.begin()) {
				iter--;
				if (movie_time.cmpTime((*iter)->endTime) >= 0) {
					iter++;
					break;
				}
				mvprintw(10, 10, "N: %2d, Total shown: %2d", (*iter)->number, totalPrinted);
			}
			else
				break;
		}
	}
}

/*
 * This function is actual player function
 * It is invoked by POSIX ALARM interrupt in every milliseconds
 * Responsible for showing or removing subtitle sentences according to movie time
 * 		on the Ncurses screen
 */
void movie_player(int sig)
{
	/*
	 * We reach the end of the subtitle list?
	 * If yes, stop the movie time
	 */
	if (iter == strInfoList.end()) {
		pauseMovie();
		return;
	}

	movie_time.incMillisecs();
	mvprintw(9, 10, "Movie Time: %s", movie_time.getPrintableTime().c_str());
	mvprintw(10, 10, "N: %2d, Total shown: %2d", (*iter)->number, totalPrinted);
	refresh();

	/*
	 * Start time for the current subtitle sentences ?
	 */
	if (!currPrinted && movie_time.betweenTimes((*iter)->startTime, (*iter)->endTime))
	{
		mvprintw(12, 10, "%d %s --> %s", (*iter)->number, (*iter)->startTime.getPrintableTime().c_str(), (*iter)->endTime.getPrintableTime().c_str());
		lines_printed = 0;
		for (std::list<std::string>::iterator li = (*iter)->lines.begin(); li != (*iter)->lines.end(); li++, lines_printed++) {
			mvprintw(14 + lines_printed, 10, "%s", (*li).c_str());
		}
		refresh();

		/*
		 * This subtitle entry is shown, do not process it again
		 */
		currPrinted = true;
		totalPrinted++;
	}
	/*
	 * Deadline of the current subtitle sentences ?
	 */
	else if (currPrinted && movie_time.cmpTime((*iter)->endTime) >= 0)
	{
		/*
		 * Remove the sentences on the screen, they are expired
		 */
		clearLatestLinesOnScreen();

		/*
		 * Skip to next subtitle entry
		 */
		iter++;
		currPrinted = false;
	}
}

/*
 * Some string functions to trim empty lines
 */
std::string TrimLeft(const std::string& s)
{
    size_t startpos = s.find_first_not_of(WHITESPACE);
    return (startpos == std::string::npos) ? "" : s.substr(startpos);
}

std::string TrimRight(const std::string& s)
{
    size_t endpos = s.find_last_not_of(WHITESPACE);
    return (endpos == std::string::npos) ? "" : s.substr(0, endpos+1);
}

std::string Trim(const std::string& s)
{
    return TrimRight(TrimLeft(s));
}

bool isEmptyLine(string strInput)
{
	if (Trim(strInput).compare("") != 0)
		return false;
	else
		return true;
}

std::string readLineFromFile(ifstream* fd_srt)
{
	std::string line;
	getline(*fd_srt, line);
	return line;
}

/*
 * This function process given Subtitle file
 * Creates a list of subtitle entries in the file
 */
void processSrtFile(char* srt_file)
{
    ifstream fd_srt;
    std::string strLine;
    int vals[8];

    fd_srt.open(srt_file);
    if (!fd_srt)
	{
		cout << "Cannot open Subtitle file: " << srt_file << endl;
		exit(EXIT_FAILURE);
	}

	// While there's still stuff left to read
	while (fd_srt)
	{
		// read stuff from the file into a string and print it
		strLine = readLineFromFile(&fd_srt);
		if (!isEmptyLine(strLine))
		{
			// Create an Entry object to keep the number times and sentences
			StrInfo* strInfo = new StrInfo();
			strInfo->number = atoi(strLine.c_str());	// Gets the entry number

			strLine = readLineFromFile(&fd_srt);

			// Gets the start and end times
			sscanf(strLine.c_str(), "%u:%u:%u,%u --> %u:%u:%u,%u",
				&vals[0], &vals[1], &vals[2], &vals[3],
				&vals[4], &vals[5], &vals[6], &vals[7]);

			strInfo->startTime.setTime(vals[0], vals[1], vals[2], vals[3]);
			strInfo->endTime.setTime(vals[4], vals[5], vals[6], vals[7]);

			// Gets sentences for current entry
			strLine = readLineFromFile(&fd_srt);
			while (!isEmptyLine(strLine)) {
				strInfo->lines.insert(strInfo->lines.end(), strLine);
				strLine = readLineFromFile(&fd_srt);
			}

			// Store current entry in the list of all entries
			strInfoList.insert(strInfoList.end(), strInfo);
		}
	}

    // Print all entries read. Just for debug
	/*for (std::list<StrInfo*>::iterator it = strInfoList.begin(); it != strInfoList.end(); it++)
	{
		cout << (*it)->number << "  " << (*it)->startTime.getPrintableTime() << " --> " << (*it)->endTime.getPrintableTime() << endl;
		cout << (*it)->number << "  " << (*it)->startTime.timeInMsec << " --> " << (*it)->endTime.timeInMsec << endl;
		cout << (*it)->number << "  " << (*it)->startTime.getPrintableTime() << " --> " << (*it)->endTime.getPrintableTime() << endl;

		for (std::list<std::string>::iterator li = (*it)->lines.begin(); li != (*it)->lines.end(); li++) {
			cout << (*li) << endl;
		}
		cout << endl;
	} */

    /*
     * Keep the end time of the subtitle for boundary control
     */
    iter = strInfoList.end();
    movie_end_time = (*--iter)->endTime.timeInMsec;

	/*
	 * Current player iterator on the first entry of the list
	 * Srt File is completely processed and all entries are stored in an arraylist
	 * Now, we are ready to play
	 */
	iter = strInfoList.begin();
}

void usage()
{
	cout << "Welcome to KSP (Kadir's just Subtitle Player)" << endl << endl;
	cout << "\t Usage:" << endl << "\t ===================" << endl;
	cout << "\t -h \t\t\t: Help" << endl;
	cout << "\t -f <srt-file> \t\t: SRT formatted subtitle file" << endl;
	cout << "\t -s <milliseconds> \t: Fast forward/backward speed on left/right buttons" << endl << endl;
}

int main(int argc, char *argv[]) {
    WINDOW * mainwin;
    int ch, ch_prev = 0, speed = 0;
    char srt_file[SRT_FILE_NAME_LEN] = {0};

    while ( (ch = getopt(argc, argv, "f:s:h") ) != -1)
    {
        switch (ch)
        {
            case 'f':
                snprintf(srt_file, SRT_FILE_NAME_LEN, "%s", optarg);
                cout << "SRT file: " << srt_file << endl;
                break;
            case 's':
                speed = atoi(optarg);
                if (speed < 0 || speed > 10000)
                {
                	cout << "FF/FB speed must be less than 10 seconds (10.000 msec) !" << endl;
                }
                break;
            case 'h':
            	usage();
            	exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Unrecognized option!\n");
                break;
        }
    }

    // Check file
    if (!strcmp(srt_file, ""))
    {
    	cout << "No Subtitle file given !!" << srt_file << endl;
    	usage();
		exit(EXIT_FAILURE);
    }

    /*
     * No need for return value
     */
    processSrtFile(srt_file);

    /*  Initialize ncurses  */
    if ((mainwin = initscr()) == NULL ) {
		fprintf(stderr, "Error initializing ncurses.\n");
		exit(EXIT_FAILURE);
    }

    noecho();                  /*  Turn off key echoing                 */
    keypad(mainwin, TRUE);     /*  Enable the keypad for non-char keys  */

    /*  Print usage and refresh() the screen  */
    mvaddstr(1, 10, "       Usage");
    mvaddstr(2, 10, "====================");
    mvaddstr(3, 10, "SpaceBar   to Pause");
    mvaddstr(4, 10, "ESC        to quit");
    mvaddstr(5, 10, "Right      to forward  ");
    mvaddstr(6, 10, "Left       to backward ");
    mvprintw(8, 10, "ff/fb speed is %d msec", speed);
    refresh();

    /*
     * Start movie timer to play
     */
    continueMovie();

    while ((ch = getch()) != ESC_BTN)
    {
    	switch (ch) {
    	case KEY_LEFT:
    		pauseMovie();
    		movie_time.backward(speed);
    		findNextSubtitle(0);
    		continueMovie();
    		break;
    	case KEY_RIGHT:
    		/*
    		 * If we reach the end, no need to forward
    		 */
    		//if (iter != strInfoList.end()) {
    		if (movie_time.timeInMsec + speed <= movie_end_time) {
    			pauseMovie();
				movie_time.forward(speed);
				findNextSubtitle(1);
				continueMovie();
    		}
			break;
    	case ' ':
    		togglePlaying();
    		break;
    	}
    }

    /*  Clean up after ourselves  */
    delwin(mainwin);
    endwin();
    refresh();

    return EXIT_SUCCESS;
}
