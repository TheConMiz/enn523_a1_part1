/* tcpserver.c
   This program implements a TCP client using socket programming. 
   Use it as a sample program only. 

   programmed by Glen Tian
              on 23 February 2018
              in Brisbane
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
/*#include <windows.h> 
*/
#include<winsock2.h>
#include <windows.h>    /* Note: winsock2.h has included windows.h */
#include "tcpserverclient.h"

int main(void)
{
	WSADATA  wsa_data;                /* type defined in winsock2.h */
	SOCKET   TCPListen, TCPClient;    /* type defined in winsock2.h */
	struct sockaddr_in local,client;  /* struct defined in winsock2.h */
	int      ipAddrSize, recvStatus, iterationStep;             
	char     recvBuffer[128];
	double   time_old, time_new, time_interval;

	printf("******** TCP Server ********\n\n");

	/* Step 1: startup winsocket - this is for Windows only */
	/* in pair with WSACleanup()    */
	if(WSAStartup(MAKEWORD(2,2), &wsa_data) != 0)
	{
        puts("WSAStartup failed!");
		exit(1);
	}

	/* Step 2: Create socket and check it is successful */
	/* in pair with closesocket()   */
	TCPListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(TCPListen == SOCKET_ERROR)
	{
		printf("Failed to create scoket(): %d\n", WSAGetLastError());
		exit(1);
	}

	/* Step 3.1: Setup Parameters for local TCP Server*/
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = inet_addr(MY_SERVER_IP_ADDRESS); /* defined in tcpserverclient.h */
	local.sin_port = htons(MY_SERVER_PORT_NUMBER);           /* defined in tcpserverclient.h */

    /* Step 3.2: Bind to the local TCP Server */
	if(bind(TCPListen, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
	{
		printf("Failed to bind(): %d\n", WSAGetLastError());
		exit(1);
	}

	/* Step 4: Listen */
    	listen(TCPListen, 8);

	/* Step 5: Accept in Loops*/
	for (;;)
	{
		/* Step 5.1: Accept */
		ipAddrSize = sizeof(client);
		TCPClient = accept(TCPListen, (struct sockaddr *)&client, &ipAddrSize);
		if(TCPClient == INVALID_SOCKET)
		{
			printf("Failed to accept(): %d\n", WSAGetLastError());
			exit(1);
		}
		printf("Client Accepted: %s:%d\n", inet_ntoa(client.sin_addr),ntohs(client.sin_port));

		/* Step 5.2: Send and Receive Data in loops */
		time_old = getTime();
		iterationStep = 1;
		for (;;)
		{
			recvStatus = recv(TCPClient, recvBuffer,128,0);
	    	if(recvStatus == 0)
		    	break;
	    	else if(recvStatus == SOCKET_ERROR)
			{
			    printf("Failed in recv(): %d\n",WSAGetLastError());
			    break;
			}
			recvBuffer[recvStatus] = 0x00; /* '\0' */
			
			time_new = getTime();
			time_interval = time_new - time_old;
			time_old = time_new;

	    	printf("Step = %5d;     Time Interval = %8.6f;     Received String: %s\n", iterationStep,time_interval,recvBuffer);
			iterationStep++;
		}
		
		/* if "EXIT" received, terminate the program */
			if (!strcmp(recvBuffer,"EXIT"))
			{
				printf("TCP Server ready to terminate.");
				break;
			}
	}
	
	/* Step 6: Close socket, in pair with socket() */ 
	closesocket(TCPListen);

	/* Step 7: Clean up winsocket - this is for Windows only! */
	/* in pair with WSAStartup() */
    WSACleanup();

	return 0;
}