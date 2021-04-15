#include <iostream>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <string>

using namespace std;

void main(int argc, char* argv[]) {
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

	// Create a socket var for the server and allocate memory.
	sockaddr_in server;
	server.sin_family = AF_INET;
	// Port has been hard-coded
	server.sin_port = htons(54000);
	// IP Address has been hard-coded
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	SOCKET output = socket(AF_INET, SOCK_DGRAM, 0);

	// Send a message to the socket
	string s(argv[1]);

	int sendOk = sendto(output, s.c_str(), s.size() + 1, 0, (sockaddr*)&server, sizeof(server));

	// If no message received, error.
	if (sendOk == SOCKET_ERROR) {
		cout << "Unable to send the message. " << WSAGetLastError() << endl;
	}

	// Close the socket
	closesocket(output);

	// End WinSock
	WSACleanup();
}