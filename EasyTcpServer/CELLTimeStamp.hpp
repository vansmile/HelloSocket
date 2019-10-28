#pragma once
#ifndef _CELLTimeStamp_hpp_
#define _CELLTimeStamp_hpp_

#endif // !_

#include <Windows.h>
#include<chrono>
using  namespace std::chrono;
class CELLTimeStamp {
public:
	CELLTimeStamp() {
		//QueryPerformanceFrequency(&_frequency);
		//QueryPerformanceCounter(&_startCount);
		update();

	}
	~CELLTimeStamp(){}
	void update() {
		//QueryPerformanceCounter(&_startCount);
		_begin = high_resolution_clock::now();
	}

	/*
	*获取当前秒
	*/
	double getElaspsedSecond() {
		return getElapsedTimeInMicroSec() * 0.000001;
	}
	/*
		获取毫秒
	*/
	double getElapsedTimeInMilliSec() {
		return this->getElapsedTimeInMicroSec() * 0.001;
	}

	/*
		获取微秒
	*/
	long long getElapsedTimeInMicroSec() {
		/*LARGE_INTEGER endCount;
		QueryPerformanceCounter(&endCount);
		double startTimeInMicroSec = _startCount.QuadPart * (1000000.0 / _frequency.QuadPart);
		double endTimeInMicroSec = endCount.QuadPart * (1000000.0 / _frequency.QuadPart);

		return endTimeInMicroSec - startTimeInMicroSec;*/

		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}

protected:
	//LARGE_INTEGER _frequency;
	//LARGE_INTEGER _startCount;
	time_point<high_resolution_clock> _begin;
};