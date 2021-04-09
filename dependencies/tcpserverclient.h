/* tcpserverclient.h
   header file for tcpserver.c and tcpclient.c

      programmed by Glen Tian
              on 23 February 2018
              in Brisbane
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>  /* for timing control */

/* IP address of TCP server to be connected to  */
#define MY_SERVER_IP_ADDRESS     "127.0.0.1"   
// #define MY_SERVER_IP_ADDRESS     "192.168.1.12"   
// #define IP_ADDRESS     "10.171.128.251"   
/* #define IP_ADDRESS     "10.0.1.167" */

/* port no. on TCP server to be connected to */
#define MY_SERVER_PORT_NUMBER    3247        

/* maximum number of iterations of sending data */
#define MAXCOUNT 15

/* get time. Demonstration only */
/* Note: better error checking and handling required in this function. */
double getTime(void)
{
	LARGE_INTEGER ticksPerSecond;
    LARGE_INTEGER tick;           /* time instant */
    LARGE_INTEGER startTicks;

    /* get the accuracy */
    if (!QueryPerformanceFrequency(&ticksPerSecond))
		printf("\tQueryPerformance not present.");

    /* check tick counter */
    if (!QueryPerformanceCounter(&tick)) 
		printf("\tTimer tick counter not installed."); 

    QueryPerformanceCounter(&startTicks); 
     
	return  (double)startTicks.QuadPart/(double)ticksPerSecond.QuadPart;
}