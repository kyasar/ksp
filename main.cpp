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
//#include <event.h>

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
//struct event ev;
//struct timeval tv;
int totalPrinted = 0;
bool currPrinted = false;
int executed = 0;

//void timer_handler(int, short, void *);
void movie_player(int);
bool timerStopped = false;

/*
 * POSIX Timer set/unset methods
 */
void stop_timer(void)
{
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
	timerStopped = true;
}

void start_timer()
{
	/*
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	event_init();
	event_set(&ev, -1, EV_PERSIST, timer_handler, NULL);
	event_add(&ev, &tv);
	*/
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &movie_player;
	sigaction (SIGALRM, &sa, NULL);	// POSIX signal

	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1000;

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000;

	setitimer (ITIMER_REAL, &timer, NULL);
	timerStopped = false;
}

void toggle_timer(void)
{
	if (timerStopped)	start_timer();
	else	stop_timer();
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
		stop_timer();
		return;
	}

	movie_time.incMillisecs();
	mvprintw(9, 10, "Movie Time: %s", movie_time.getPrintableTime().c_str());
	mvprintw(10, 10, "iter: %d, total: %d", (*iter)->number, totalPrinted);
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
		for (int i = 0; i < lines_printed; i++) {
			move(14 + i, 0);
			clrtoeol();
		}
		/*
		 * Skip to next subtitle entry
		 */
		iter++;
		currPrinted = false;
	}
}

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
                cout << "FF FB speed: " << speed << endl;
                break;
            case 'h':
            	usage();
            	exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Unrecognized option!\n");
                break;
        }
    }

    if (!strcmp(srt_file, ""))
    {
    	cout << "No Subtitle file given !!" << srt_file << endl;
		exit(EXIT_FAILURE);
    }

    ifstream fd_srt;
    fd_srt.open(srt_file);
    if (!fd_srt)
	{
		cout << "Cannot open Subtitle file: " << srt_file << endl;
		exit(EXIT_FAILURE);
	}

	// While there's still stuff left to read
    std::string strLine;
	while (fd_srt)
	{
		// read stuff from the file into a string and print it
		strLine = readLineFromFile(&fd_srt);
		if (!isEmptyLine(strLine))
		{
			StrInfo* strInfo = new StrInfo();
			strInfo->number = atoi(strLine.c_str());

			strLine = readLineFromFile(&fd_srt);

			int vals[8];
			sscanf(strLine.c_str(), "%u:%u:%u,%u --> %u:%u:%u,%u",
				&vals[0], &vals[1], &vals[2], &vals[3],
				&vals[4], &vals[5], &vals[6], &vals[7]);

			strInfo->startTime.setTime(vals[0], vals[1], vals[2], vals[3]);
			strInfo->endTime.setTime(vals[4], vals[5], vals[6], vals[7]);

			strLine = readLineFromFile(&fd_srt);
			while (!isEmptyLine(strLine)) {
				strInfo->lines.insert(strInfo->lines.end(), strLine);
				strLine = readLineFromFile(&fd_srt);
			}
			strInfoList.insert(strInfoList.end(), strInfo);
		}
	}

	for (std::list<StrInfo*>::iterator it = strInfoList.begin(); it != strInfoList.end(); it++)
	{
		cout << (*it)->number << "  " << (*it)->startTime.getPrintableTime() << " --> " << (*it)->endTime.getPrintableTime() << endl;
		cout << (*it)->number << "  " << (*it)->startTime.timeInMsec << " --> " << (*it)->endTime.timeInMsec << endl;
		cout << (*it)->number << "  " << (*it)->startTime.getPrintableTime() << " --> " << (*it)->endTime.getPrintableTime() << endl;

		//cout << (*it)->endTime.cmpTime((*it)->startTime) << endl;

		for (std::list<std::string>::iterator li = (*it)->lines.begin(); li != (*it)->lines.end(); li++) {
			cout << (*li) << endl;
		}
		cout << endl;
	}

    //exit(EXIT_SUCCESS);

    /*  Initialize ncurses  */
    if ( (mainwin = initscr()) == NULL ) {
		fprintf(stderr, "Error initializing ncurses.\n");
		exit(EXIT_FAILURE);
    }

    noecho();                  /*  Turn off key echoing                 */
    keypad(mainwin, TRUE);     /*  Enable the keypad for non-char keys  */

    /*  Print a prompt and refresh() the screen  */
    mvaddstr(2, 10, "       Usage");
    mvaddstr(3, 10, "====================");
    mvaddstr(4, 10, "SpaceBar   to Pause");
    mvaddstr(5, 10, "ESC        to quit");
    mvaddstr(6, 10, "Right      to forward");
    mvaddstr(7, 10, "Left       to forward");
    refresh();

    /*  Loop until user presses 'q'  */
    int counter = 0, lines_printed = 0;
    iter = strInfoList.begin();

    //event_loop(0);
    start_timer();

    while ((ch = getch()) != ESC_BTN)
    {
    	switch (ch) {
    	case KEY_LEFT:
    //		movie_time.backwardSec(5);
    		/*if (iter == strInfoList.begin()) {
				continue;
			}
    		iter--;*/
    		break;
    	case KEY_RIGHT:
		//	movie_time.forwardSec(5);
			/*if (iter == --strInfoList.end()) {
				continue;
			}
			iter++;*/
			break;
    	case ' ':
    		toggle_timer();
    		break;
    	}
    }

    /*  Clean up after ourselves  */
    delwin(mainwin);
    endwin();
    refresh();

    return EXIT_SUCCESS;
}
