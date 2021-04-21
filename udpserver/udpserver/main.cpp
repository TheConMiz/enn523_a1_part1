// UDP SERVER
#include <iostream>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <time.h>
#include <string>
#include <chrono>

#define PORT 54000
#define LOOPBACK "127.0.0.1"

using namespace std;

// Combines the sending of message to the client and error-checking
void sendMessage(SOCKET socketFile, const char* messageType, int length, int flags, const sockaddr *to, int tolen) {

	int sendStatus = sendto(socketFile, messageType, length, flags, to, tolen);

	// If unable to send, print error message.
	if (sendStatus == SOCKET_ERROR) {
		cout << "Unable to send message. Error: " << WSAGetLastError() << endl;
	}
}

// Combines the receipt of message from the client and error-checking
void receiveMessage(SOCKET socketFile, char* messageType, int length, int flags, sockaddr *from, int *fromlen) {
	int messageIn = recvfrom(socketFile, messageType, length, flags, from, fromlen);

	// If no message received, error.
	if (messageIn == SOCKET_ERROR) {
		cout << "Unable to receive from server. " << WSAGetLastError() << endl;
	}
	
}

int main(int argc, char* argv[]) {

	// Variables for memory allocation.
	const int BUFFERLENGTH = 1024;
	const int IP_LENGTH = 256;

	// Socket address variables for local and client.
	sockaddr_in local, client;

	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);

	// Buffer variable for incoming messages.
	char buffer[BUFFERLENGTH];

	time_t oldTime, newTime, timeDifference;

	// Variable for counting iteration in loop
	int iterationStep;

	// Commands
	const char* ack = "ACK";
	const char* ackE = "ACK E";
	const char* ackR = "ACK R";
	const char* e = "E";
	const char* r = "R";


	cout << "UPD Server -- ACTIVE" << endl;

	// Start a WinSock
	WSADATA wsaData;
	// Specify version
	WORD version = MAKEWORD(2, 2);
	// Variable for checking WinSock status
	int wsOk = WSAStartup(version, &wsaData);

	// If unable to start, print error text and exit the program.
	if (wsOk != 0) {
		cout << "WinSock Start-up -- Failed: " << wsOk << endl;
		exit(1);
	}

	// Create a socket
	SOCKET socketFile = socket(AF_INET, SOCK_DGRAM, 0);

	// If failed to create a socket, print error and exit the program. 
	if (socketFile == SOCKET_ERROR){
		cout << "Failed to create socket: " << WSAGetLastError() << endl;
		exit(1);
	}

	// Establish local UDP server parameters
	local.sin_addr.S_un.S_addr = ADDR_ANY;
	local.sin_family = AF_INET;
	// Port 54000 is converted to big endian. 
	local.sin_port = htons(PORT);

	// Bind to the local UDP server. On fail, error and exit.
	if (bind(socketFile, (sockaddr*)&local, sizeof(local)) == SOCKET_ERROR){
		cout << "Cannot bind socket. " << WSAGetLastError() << endl;
		exit(1);
	}

	oldTime = time(NULL);

	// Allocate space for buffer.
	ZeroMemory(buffer, BUFFERLENGTH);

	receiveMessage(socketFile, buffer, 1024, 0, (sockaddr*)&client, &clientLength);

	// Allocate space for client's IP information.
	char clientIp[IP_LENGTH];
	ZeroMemory(clientIp, IP_LENGTH);

	// Store IP information in clientIp variable
	inet_ntop(AF_INET, &client.sin_addr, clientIp, IP_LENGTH);

	cout << "NOT PART OF PROJECT - Message from client " << clientIp << ": " << buffer << endl;
	
	while (true) {

		// Allocate space for buffer.
		ZeroMemory(buffer, BUFFERLENGTH);


		// THIS IS WHERE THE CODE SHOULD START. THE CLIENT IP INFORMATION SHOULD BE GATHERED FROM A HEADER FILE OR SOMETHING SIMILAR.
					// Send "R" to client
		
		oldTime = time(NULL);

		// PROBLEM - TIMING IS OFF EVERY NOW AND THEN. THE ACK R IS NOT RETURNED EVERY SINGLE TIME. 
		while (true) {
			
			newTime = time(NULL);

			if (newTime - oldTime == 3) {
				sendMessage(socketFile, r, BUFFERLENGTH, 0, (sockaddr*)&client, clientLength);

				// Wait for a message. 
				receiveMessage(socketFile, buffer, BUFFERLENGTH, 0, (sockaddr*)&client, &clientLength);


				// If received message is ACK R: 
				if (!strcmp(buffer, ackR)) {
				
					cout << buffer << " seqno " << newTime << endl;

					// Send ACK
					sendMessage(socketFile, ack, BUFFERLENGTH, 0, (sockaddr*)&client, clientLength);

					receiveMessage(socketFile, buffer, BUFFERLENGTH, 0, (sockaddr*)&client, &clientLength);
				}

				oldTime = newTime;
			}


			
			// If received message is ACK R: 
			// if (!strcmp(buffer, ackR)) {
				
			// 	cout << buffer << " seqno " << newTime << endl;

			// 	// Send ACK
			// 	sendMessage(socketFile, ack, BUFFERLENGTH, 0, (sockaddr*)&client, clientLength);

			// 	receiveMessage(socketFile, buffer, BUFFERLENGTH, 0, (sockaddr*)&client, &clientLength);
			// }

			// If received message is ACK: 
			// if (!strcmp(buffer, ack)) {

			// 	cout << buffer << " seqno " << newTime << endl;

			// 	cout << "Calculating Round Trip Delay: " << endl;
			// }

		}
	}

	// Close WinSock
	closesocket(socketFile);

	// End WinSock
	WSACleanup();

	return 0;
}