// UDP CLIENT
#include "UDPTools.h"

// Combines the sending of message to the server and error-checking
void sendMessage(SOCKET socketFile, const char* message, const sockaddr* to, int tolen) {

	int sendStatus = sendto(socketFile, message, BUFFERLENGTH, 0, to, tolen);

	// If unable to send, print error message.
	if (sendStatus == SOCKET_ERROR) {
		cout << "Unable to send message. Error: " << WSAGetLastError() << endl;
	}
}


int main(int argc, char* argv[]) {

	// Variables for collecting configuration information from the command line
	const char* localIP;
	const char* serverIP;
	int localPort;
	int serverPort;

	// IF COMMAND LINE ARGUMENTS ARE PROVIDED, USE THEM
	if (argc >= 5) {

		localIP = argv[1];
		serverIP = argv[2];
		localPort = atoi(argv[3]);
		serverPort = atoi(argv[4]);
	}

	// IF NOT, RESORT TO DEFAULT LOOPBACK ADDRESS AND PORT
	else {
		localIP = LOOPBACK;
		serverIP = LOOPBACK;
		localPort = PORT;
		serverPort = PORT;
	}

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
	// Allocate space for buffer.
	ZeroMemory(buffer, BUFFERLENGTH);

	// Variables for calculating round trip delay, and maintaining time
	double time_old, time_new, time_interval;

	// Commands
	const char* ack = "ACK";
	const char* ackE = "ACK E";
	const char* ackR = "ACK R";
	const char* e = "E";
	const char* r = "R";

	// Variable for sequence number.
	int currentSeqNum = 0;

	cout << "********** UPD CLIENT - ACTIVE **********" << endl << endl;

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
	server.sin_port = htons(serverPort);
	// Socket IP address -- GOTTA BE SOMETHING DIFFERENT
	inet_pton(AF_INET, serverIP, &server.sin_addr);

	// Socket object named output
	SOCKET socketFile = socket(AF_INET, SOCK_DGRAM, 0);

	// PING THE SERVER TO START THE UDP COMMUNICATION PROCESS
	sendMessage(socketFile, "PING", (sockaddr*)&server, serverLength);

	while (true) {

		receiveMessage(socketFile, buffer, (sockaddr*)&server, &serverLength);

		// If receive "R", respond with "ACK R"
		if (!strcmp(buffer, r)) {

			++currentSeqNum;

			cout << buffer << " " << currentSeqNum << " " << getTimestamp() << endl;

			sendMessage(socketFile, ackR, (sockaddr*)&server, serverLength);
		}

		// If receive "ACK", respond with "ACK"
		if (!strcmp(buffer, ack)) {

			cout << buffer << " " << currentSeqNum << " " << getTimestamp() << endl << endl;

			// Send an ACK
			sendMessage(socketFile, ack, (sockaddr*)&server, serverLength);

		}

		// If receive E, respond with ACK E
		if (!strcmp(buffer, e)) {

			++currentSeqNum;

			cout << buffer << " " << currentSeqNum << " " << getTimestamp() << endl;

			sendMessage(socketFile, ackE, (sockaddr*)&server, serverLength);

			// Wait for an Ack
			receiveMessage(socketFile, buffer, (sockaddr*)&server, &serverLength);

			// Upon receiving an ack, exit.
			if (!strcmp(buffer, ack)) {

				cout << buffer << " " << currentSeqNum << " " << getTimestamp() << endl;

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