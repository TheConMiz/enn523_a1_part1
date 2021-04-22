// UDP CLIENT
#include <iostream>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <chrono>
#include <iomanip>

#define PORT 54000
#define LOOPBACK "127.0.0.1"


// To deal with deprecation warnings
#pragma warning(disable : 4996)

using namespace std;

// Global variables for memory allocation.
const int BUFFERLENGTH = 1024;
const int IP_LENGTH = 256;

// Combines the sending of message to the server and error-checking
void sendMessage(SOCKET socketFile, const char* message, const sockaddr* to, int tolen) {

	int sendStatus = sendto(socketFile, message, BUFFERLENGTH, 0, to, tolen);

	// If unable to send, print error message.
	if (sendStatus == SOCKET_ERROR) {
		cout << "Unable to send message. Error: " << WSAGetLastError() << endl;
	}
}

// Combines the receipt of message from the server and error-checking
void receiveMessage(SOCKET socketFile, char* message, sockaddr* from, int* fromlen) {

	int messageIn = recvfrom(socketFile, message, BUFFERLENGTH, 0, from, fromlen);

	// If no message received, error.
	if (messageIn == SOCKET_ERROR) {
		cout << "Unable to receive from server. " << WSAGetLastError() << endl;
	}

}

// Returns a timestamp of format hh:mm:ss:ms
// PROBLEM WITH THE MS
string getTimestamp() {

	const auto now = std::chrono::system_clock::now();

	const auto rawTime = std::chrono::system_clock::to_time_t(now);

	// Need this to generate milliseconds value separately
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	// Required for formatting time value as string
	std::stringstream nowSs;

	nowSs << std::put_time(std::localtime(&rawTime), "%a %b %d %Y %T") << ':' << std::setfill('0') << std::setw(3) << ms.count();

	return nowSs.str();
}

int main(int argc, char* argv[]) {

	// Variables for memory allocation.
	const int BUFFERLENGTH = 1024;
	const int IP_LENGTH = 256;

	// Socket address variables for local and client.
	sockaddr_in server;

	// Allocate memory for the server socket address.
	int serverLength = sizeof(server);
	ZeroMemory(&server, serverLength);

	// Buffer variable for incoming messages.
	char buffer[BUFFERLENGTH];

	// Variables for calculating round trip delay, and maintaining time
	double time_old, time_new, time_interval;

	// Commands
	const char* ack = "ACK";
	const char* ackE = "ACK E";
	const char* ackR = "ACK R";
	const char* e = "E";
	const char* r = "R";

	// Start a WinSock
	WSADATA data;
	// Specify version
	WORD version = MAKEWORD(2, 2);
	// Variable for checking WinSock status
	int wsOk = WSAStartup(version, &data);

	// If unable to start, print error text and exit the program.
	if (wsOk != 0) {
		cout << "WinSock Start-up -- Failed: " << wsOk << endl;
		exit(1);
	}

	// Socket family
	server.sin_family = AF_INET;
	// Socket port
	server.sin_port = htons(PORT);
	// Socket IP address
	inet_pton(AF_INET, LOOPBACK, &server.sin_addr);

	// Socket object named output
	SOCKET socketFile = socket(AF_INET, SOCK_DGRAM, 0);

	// This line is here to establish the connection between client and server by sending client info to the server. NEEDS TO BE REMOVED. 
	sendMessage(socketFile, "establish contact", (sockaddr*)&server, serverLength);

	while (true) {

		// THIS IS WHERE THE CODE SHOULD START. THE SERVER IP INFORMATION SHOULD BE OBTAINED ONCE THE SERVER MAKES CONTACT
		receiveMessage(socketFile, buffer, (sockaddr*)&server, &serverLength);

		// If receive "R", respond with "ACK R"
		if (!strcmp(buffer, r)) {

			cout << buffer << " seqno " << getTimestamp() << endl;

			sendMessage(socketFile, ackR, (sockaddr*)&server, serverLength);
		}

		// If receive "ACK", respond with "ACK"
		if (!strcmp(buffer, ack)) {

			cout << buffer << " seqno " << getTimestamp() << endl;

			// Send an ACK
			sendMessage(socketFile, ack, (sockaddr*)&server, serverLength);

		}

		// If receive E, respond with ACK E
		if (!strcmp(buffer, e)) {

			cout << buffer << " seqno " << getTimestamp() << endl;
			
			sendMessage(socketFile, ackE, (sockaddr*)&server, serverLength);

			// Wait for an Ack
			receiveMessage(socketFile, buffer, (sockaddr*)&server, &serverLength);

			// Upon receiving an ack, exit.
			if (!strcmp(buffer, ack)) {

				cout << buffer << " seqno " << " time" << endl;
				
				exit(1);
			}
		}

	}



	// Close the socket
	closesocket(socketFile);

	// End WinSock
	WSACleanup();

	return 0;
}