/* tcpclient.c
   This program implements a TCP client using socket programming. 
   Use it as a sample program only. 

   programmed by Glen Tian
              on 23 February 2018
              in Brisbane
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>     /* Note: winsock2.h has included windows.h */
#include <time.h>
#include "tcpserverclient.h"


#define PERIOD   2        /* in second */
#define NUMBER_RANGE 1000 /* range 0 - 1000 for random number generator*/

int main(void)
{
    WSADATA       wsa_data;           /* defined in winsock2.h */
    SOCKET        TCPClient;          /* defined in winsock2.h */
    struct sockaddr_in TCPServer;     /* defined in winsock2.h */
    struct hostent     *host = NULL;  /* defined in winsock2.h */

	char sendMessageStr[8]; /* message to be sent to server */
	int sendStatus;
	double time_old, time_new, time_interval = 0;
	int i;               /* loop counter */

	printf("======== TCP Client ========\n\n");

	/* Step 1: startup winsocket - this is for Windows only */
	/* in pair with WSACleanup()                    */ 
    if (WSAStartup(MAKEWORD(2,2), &wsa_data) != 0)
    {
        puts("WSAStartup failed!");
        exit(1);
    }

	/* Step 2: Create socket and check it is successful */
	/* in pair with closesocket()               */
    TCPClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (TCPClient == INVALID_SOCKET)
    {
        printf("Failed to create socket(): %d\n", WSAGetLastError());
        exit(1);
    }

	/* Step 3.1: Setup TCP Server Parameters */
    TCPServer.sin_family = AF_INET;
    TCPServer.sin_addr.s_addr = inet_addr(MY_SERVER_IP_ADDRESS); /* defined in tcpserverclient.h */
    TCPServer.sin_port = htons(MY_SERVER_PORT_NUMBER);           /* defined in tcpserverclient.h */

	/* Step 3.2: Connect to the TCP Server */
    if (connect(TCPClient, (struct sockaddr *)&TCPServer, sizeof(TCPServer)) == SOCKET_ERROR)
    {
        printf("connect() failed: %d\n", WSAGetLastError());
        exit(1);
    }
    
	time_old = getTime();
	srand(time(NULL));  /* seed random number generator */
	/* Step 4: Send and Receive Data - Here is an example only */
	for(i = 0; i < MAXCOUNT; i++)   /* defined in tcpserveclient.h */
    {
		time_interval = 0;
		while (time_interval < PERIOD)
		{
			time_new = getTime();
			time_interval = time_new - time_old;
		}
		time_old = time_new;

	    /* a simple delay using loops - for demonstration only */
//		for (j = 0; j < 20000; j++)
//			for (k=0; k < 20000; k++);

		/* prepare message to be sent */
		/* example: simply convert a random number into a string and then send it out */
		itoa(rand()%(NUMBER_RANGE+1),sendMessageStr,10);

		/* send message */
        sendStatus = send(TCPClient, sendMessageStr, strlen(sendMessageStr), 0);

		/* check the status of the send() call */
		/* send() returns the number of bytes sent OR -1 if failure */
        if (sendStatus == 0)
            break;  /* nothing has been sent */
        else if (sendStatus == SOCKET_ERROR)
        {
            printf("Failed to send(): %d\n", WSAGetLastError());
            break;
        }
        printf("Step=%4d; Time Interval=%8.6fs; Sent Msg = %4s; Sent %3d byte/s\n", i+1,time_interval,sendMessageStr,sendStatus);
    }
    
	send(TCPClient, "EXIT", strlen("EXIT"), 0); /* ready to exit, notify the TCP Server */

	/* Step 5: Close socket, in pair with socket() */ 
	closesocket(TCPClient);

	/* Step 6: Clean up winsocket - this is for Windows only! */
	/* in pair with WSAStartup() */
    WSACleanup();
	
    return 0;
}

