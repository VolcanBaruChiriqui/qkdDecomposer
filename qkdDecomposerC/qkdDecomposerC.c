/***************************************************************************
 * 3. Decomposition of timestamp and basis information: in quantum key distribution, 
 * we locally measure a series of photon arrival times with one out of four possible outcomes. 
 * Each photon arrival event receives a timestamp (integer of the current time in nanoseconds) 
 * and a detector click (1-4). The two are combined and stored in a large binary data file 
 * (much like the one attached as “test3_Alice”) as 64 bit integers (or rather 2x 32 bit integers). 
 * Could you write a short C/C++ program to read the file, split the timestamp and detector click information, 
 * and store them in separate files? Measure how fast the read/processing/write processes are. 
 * What is the average separation of time stamps in the attached file?
 *
 * The data is generated from two arrays (stamp = time stamps, basis= detector clicks) 
 * in python with the function in saveBinary.py. By understanding this function, 
 * you should be able to decipher the data format.
 *
 * VolcanBaruChiriqui's solution using Microsoft Visual Studio Community 2017:
 *	Debug x86 preprocessor options set:	HUMAN_READABLE_OUTPUT;MEASURE_READ_TIME;MEASURE_PROCESS_TIME;MEASURE_WRITE_TIME;_CRT_SECURE_NO_WARNINGS
 *	Release x86 preprocessor options set: MEASURE_READ_TIME;MEASURE_PROCESS_TIME;MEASURE_WRITE_TIME;_CRT_SECURE_NO_WARNINGS
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

//#define _POSIX_C_SOURCE 199309L
//#include <time.h>
#include <windows.h>
#include <minwinbase.h>
#define CLOCK_PROCESS_CPUTIME_ID 2 //dummy #define for clock_gettime signature compatibility

#define PACKAGE "qkdDecomposerC"
#define VERSION "0.2.0"

static FILE *inputFile = 0;
static FILE *timeStampFile = 0;
static FILE *detectorClickFile = 0;

struct timespec { long tv_sec; long tv_nsec; };    //header part

int64_t difftimespec_ns(const struct timespec stop, const struct timespec start)
{
	return ((int64_t)stop.tv_sec - (int64_t)start.tv_sec) * (int64_t)1000000000
		+ ((int64_t)stop.tv_nsec - (int64_t)start.tv_nsec);
}
/* From Asain Kujovic's answer at https://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows */
//struct timespec { long tv_sec; long tv_nsec; };    //header part
//int clock_gettime(int X, struct timespec *spec)      //C-file part
//{
//	__int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
//	wintime -= 116444736000000000i64;  //1jan1601 to 1jan1970
//	spec->tv_sec = wintime / 10000000i64;           //seconds
//	spec->tv_nsec = wintime % 10000000i64 * 100;      //nano-seconds
//	return 0;
//}
#define exp7           10000000i64     //1E+7     //C-file part
#define exp9         1000000000i64     //1E+9
#define w2ux 116444736000000000i64     //1.jan1601 to 1.jan1970
void unix_time(struct timespec *spec)
{
	__int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
	wintime -= w2ux;  spec->tv_sec = wintime / exp7;
	spec->tv_nsec = wintime % exp7 * 100;
}
int clock_gettime(int X, struct timespec *spec)
{
	static  struct timespec startspec; static double ticks2nano;
	static __int64 startticks, tps = 0;    __int64 tmp, curticks;
	QueryPerformanceFrequency((LARGE_INTEGER*)&tmp); //some strange system can
	if (tps != tmp) {
		tps = tmp; //init ~~ONCE         //possibly change freq ?
		QueryPerformanceCounter((LARGE_INTEGER*)&startticks);
		unix_time(&startspec); ticks2nano = (double)exp9 / tps;
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&curticks); curticks -= startticks;
	spec->tv_sec = startspec.tv_sec + (curticks / tps);
	spec->tv_nsec = startspec.tv_nsec + (double)(curticks % tps) * ticks2nano;
	if (!(spec->tv_nsec < exp9)) { spec->tv_sec++; spec->tv_nsec -= exp9; }
	return 0;
}
/* From Asain Kujovic's answer */


int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970 
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}
int64_t difftimespec_us(const struct timeval stop, const struct timeval start)
{
	return ((int64_t)stop.tv_sec - (int64_t)start.tv_sec) * (int64_t)1000000
		+ ((int64_t)stop.tv_usec - (int64_t)start.tv_usec);
}


int readEventFromFile(uint32_t *m, uint32_t *l, int *eventCount)
{
#ifdef MEASURE_READ_TIME
	struct timespec startTime, stopTime;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);
	//struct timeval startTime, stopTime;
	//gettimeofday(&startTime, NULL);
#endif

	size_t ret = fread(m, sizeof(uint32_t), 1, inputFile);
	if (ret != 1) {
		printf("fread() failed: %zu\n", ret);
		//break;
		return -1;
	}

	ret = fread(l, sizeof(uint32_t), 1, inputFile);
	if (ret != 1) {
		printf("fread() failed: %zu\n", ret);
		//break;
		return -2;
	}

#ifdef MEASURE_READ_TIME
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stopTime);
	int64_t elapsed = difftimespec_ns(stopTime, startTime);
	//gettimeofday(&stopTime, NULL);
	//int64_t elapsed = difftimespec_us(stopTime, startTime);
	printf("event:% 5d\treadtime:% 9" PRId64, ++(*eventCount), elapsed);
#endif

	return 0;
}

uint64_t processEvent(uint32_t *m, uint32_t *l, uint64_t *stamp, uint32_t *basis)
{
	uint64_t prevTimeStamp = *stamp;

#ifdef MEASURE_PROCESS_TIME
	struct timespec startTime, stopTime;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);
#endif

	uint64_t msw = *m;
	msw = msw << 17;

	uint32_t mask = 0b00000000000000000111111111111111;
	uint32_t ll = *l & mask;
	*basis = log2(ll);

	*l -= *basis;
	uint64_t lsw = *l;
	lsw = lsw >> 15;
	*stamp = msw | lsw;

#ifdef MEASURE_PROCESS_TIME
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stopTime);
	int64_t elapsed = difftimespec_ns(stopTime, startTime);
	printf("\tstamp:%" PRIu64 "\tbasis:%" PRIu32 "\tprocesstime:% 9" PRId64, *stamp, *basis, elapsed);
#endif

	return *stamp - prevTimeStamp;
}

int writeEventToSeparateFiles(uint64_t *stamp, uint32_t *basis)//, int *eventCount)
{
#ifdef MEASURE_WRITE_TIME
	struct timespec startTime, stopTime;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);
#endif

#ifdef HUMAN_READABLE_OUTPUT
	fprintf(timeStampFile, "%" PRIu64 "\n", *stamp);
	fprintf(detectorClickFile, "%" PRIu32 "\n", *basis);
#else
	fwrite(stamp, sizeof(uint64_t), 1, timeStampFile);
	fwrite(basis, sizeof(uint32_t), 1, detectorClickFile);
#endif

#ifdef MEASURE_WRITE_TIME
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stopTime);
	int64_t elapsed = difftimespec_ns(stopTime, startTime);
	printf("\twritetime:% 9" PRId64 "\n", elapsed);
#endif

	return 0;
}

int
main(int argc, char **argv)
{
	if ((inputFile = fopen("test3_Alice.dat", "rb")) != NULL)
	{
		timeStampFile = fopen("timeStamp.dat", "w");// b");
		detectorClickFile = fopen("detectorClick.dat", "w");// b");

		if (timeStampFile != NULL && detectorClickFile != NULL)
		{
			uint32_t m, l;
			uint32_t basis;
			uint64_t stamp = 0;
			int eventCount = 0;
			uint64_t accumulatedTimeStampDiff = 0;

			while (1)
			{
				if (readEventFromFile(&m, &l, &eventCount) != 0)
					break;

				accumulatedTimeStampDiff += processEvent(&m, &l, &stamp, &basis);

				writeEventToSeparateFiles(&stamp, &basis);// , &eventCount);
			}

			fclose(timeStampFile);
			fclose(detectorClickFile);

			printf("average separation of %d time stamps:% 9" PRIu64, eventCount, accumulatedTimeStampDiff / eventCount);
		}

		fclose(inputFile);
		return 0;
	}
	else
	{
		printf("fopen() inputFile failed\n");
		return -1;
	}
} /* End of main() */


