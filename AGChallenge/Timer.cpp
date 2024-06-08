//#include  "stdafx.h"
#include  "timer.h"
using namespace TimeCounters;

TimeCounter::TimeCounter()
{
	startInited = false;
	finishInited = false;
}

void TimeCounter::setStartNow()
{
	startInited = true;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startPosition);
}

//if returned value is false it means the timer was not set on start
bool TimeCounter::getTimePassed(double *timePassedSec)
{
	if (startInited == false)
	{
		return false;
	}

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	double result = now.QuadPart - startPosition.QuadPart;
	result = result / frequency.QuadPart;

	*timePassedSec = result;

	return true;
}

bool TimeCounter::setFinishOn(double timeToFinishSec)
{
	if (startInited == false || timeToFinishSec <= 0)
	{
		return false;
	}

	finishInited = true;

	finishPosition.QuadPart = startPosition.QuadPart + frequency.QuadPart * timeToFinishSec;

	return true;
}

bool TimeCounter::isFinished()
{
	if (startInited != true || finishInited != true)
	{
		return true;
	}

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	if (now.QuadPart > finishPosition.QuadPart)
	{
		return true;
	}
	return false;
}
