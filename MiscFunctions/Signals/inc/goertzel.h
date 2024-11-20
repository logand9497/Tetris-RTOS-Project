// goertzel.h
// Date Created: 2023-07-26
// Date Updated: 2023-07-26
// Signal processing library for threaded goertzel processing

#ifndef GOERTZEL_H_
#define GOERTZEL_H_

/************************************Includes***************************************/

#include <math.h>

/************************************Includes***************************************/

/*************************************Defines***************************************/
/*************************************Defines***************************************/

/******************************Data Type Definitions********************************/
/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/
/****************************Data Structure Definitions*****************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

int32_t goertzel(double detect_hz, double sample_rate_hz, int N, int16_t (*ptrSamplingFunc)(int), int FIFO_index);

/********************************Public Functions***********************************/


#endif /* GOERTZEL_H_ */


