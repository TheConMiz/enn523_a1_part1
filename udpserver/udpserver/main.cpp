#include <iostream>
#include <WS2tcpip.h>
#include <WinSock2.h>

using namespace std;

void main() {
	const int BUFFERLENGTH = 1024;
	const int IP_LENGTH = 256;

	// Start a WinSock
	WSADATA data;

	// Specify version
	WORD version = MAKEWORD(2, 2);

	// Variable for checking WinSock status
	int wsOk = WSAStartup(version, &data);

	// If unable to start, return the following text
	if (wsOk != 0) {
		cout << "Cannot start WinSock. " << wsOk << endl;
		return;
	}

	// Create a socket
	SOCKET input = socket(AF_INET, SOCK_DGRAM, 0);

	// Bind socket to IP address
	sockaddr_in server;

	// BOUND TO WHATEVER IP ADDRESS IS AVAILABLE. WILL HAVE TO CHANGE THE DECLARATION LATER.
	server.sin_addr.S_un.S_addr = ADDR_ANY;
	server.sin_family = AF_INET;
	
	// Port 54000 is converted to big endian.
	server.sin_port = htons(54000);

	// If unable to bind the socket variable to the actual port, return an error.
	if (bind(input, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR){
		cout << "Cannot bind socket. " << WSAGetLastError() << endl;
	}

	// Create a socket var for the client and allocate memory.
	sockaddr_in client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);

	// Create buffer space for incoming messages.

	char buffer[BUFFERLENGTH];

	while (true) {

		// Allocate space for buffer.
		ZeroMemory(buffer, BUFFERLENGTH);

		// Wait for a message
		int bytesIn = recvfrom(input, buffer, 1024, 0, (sockaddr*)&client, &clientLength);

		// If no message received, error.
		if (bytesIn == SOCKET_ERROR) {
			cout << "Unable to receive from client. " << WSAGetLastError() << endl;
			continue;
		}

		// Allocate space for client's IP information.
		char clientIp[IP_LENGTH];
		ZeroMemory(clientIp, IP_LENGTH);

		inet_ntop(AF_INET, &client.sin_addr, clientIp, IP_LENGTH);

		cout << "Message from " << clientIp << ": " << buffer << endl;


		// Respond to what has been received
	}

	// Close WinSock
	closesocket(input);


	// End WinSock
	WSACleanup();
}