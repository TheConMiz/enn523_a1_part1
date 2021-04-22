// UDP SERVER
#include "UDPTools.h"

// Global variables
// Send Error
bool sendError = false;

// Thread Completion
bool exitRequest = false;


// Struct that is destroyed upon each iteration of R being sent. Provides Round Trip Delay upon destruction.
struct RoundTripTimer {

	std::chrono::time_point<std::chrono::steady_clock> start, end;

	std::chrono::duration<float> duration;

	RoundTripTimer() {

		start = std::chrono::high_resolution_clock::now();
	}

	~RoundTripTimer() {

		end = std::chrono::high_resolution_clock::now();
		// Duration in seconds
		duration = end - start;

		// Duration in ms.
		float ms = duration.count() * 1000.0f;

		if (!sendError) {
			cout << "Round Trip Time: " << ms << "ms" << endl << endl;
		}
	}
};

// Combines the sending of message to the client and error-checking
void sendMessage(SOCKET socketFile, const char* message, int sequenceNum, const sockaddr* to, int tolen) {

	// Do something here to send both message and sequence number
	int sendStatus = sendto(socketFile, message, BUFFERLENGTH, 0, to, tolen);

	// If unable to send, print error message.
	if (sendStatus == SOCKET_ERROR) {

		sendError = true;

		cout << "Unable to send message with sequence number " << sequenceNum << ". Error: " << WSAGetLastError() << endl << endl;
	}

	else {
		sendError = false;
	}
}

void UDPLoop(time_t oldTime, time_t newTime, int currentSeqNum, SOCKET socketFile, sockaddr_in client, int clientLength, char* buffer) {
	while (true) {

		newTime = time(NULL);

		// Find a way to acknowledge E
		if (exitRequest) {

			++currentSeqNum;

			// Send an E to the client
			sendMessage(socketFile, e, currentSeqNum, (sockaddr*)&client, clientLength);

			// Allocate space for buffer.
			ZeroMemory(buffer, BUFFERLENGTH);

			// Wait for a message.
			receiveMessage(socketFile, buffer, (sockaddr*)&client, &clientLength);

			if (!strcmp(buffer, ackE)) {

				cout << buffer << " " << currentSeqNum << " " << getTimestamp() << endl;

				// Allocate space for buffer.
				ZeroMemory(buffer, BUFFERLENGTH);

				sendMessage(socketFile, ack, currentSeqNum, (sockaddr*)&client, clientLength);

				exit(1);
			}
		}


		if (newTime - oldTime == 3) {

			++currentSeqNum;

			// Provides round trip delay.
			RoundTripTimer roundTripTimer;

			// Send an R to the client
			sendMessage(socketFile, r, currentSeqNum, (sockaddr*)&client, clientLength);

			// Wait for a message. PROBLEM - IF THE CLIENT DOES NOT COME ONLINE ON time, it stalls.
			receiveMessage(socketFile, buffer, (sockaddr*)&client, &clientLength);

			// If received message is ACK R:
			if (!strcmp(buffer, ackR)) {

				cout << buffer << " " << currentSeqNum << " " << getTimestamp() << endl;

				// Send ACK.
				sendMessage(socketFile, ack, currentSeqNum, (sockaddr*)&client, clientLength);

				// Allocate space for buffer.
				ZeroMemory(buffer, BUFFERLENGTH);

				// Wait for a response. 
				receiveMessage(socketFile, buffer, (sockaddr*)&client, &clientLength);
			}

			// If received message is ACK: 
			if (!strcmp(buffer, ack)) {

				cout << buffer << " " << currentSeqNum << " " << getTimestamp() << endl;

				// Allocate space for buffer.
				ZeroMemory(buffer, BUFFERLENGTH);

				cout << endl;
			}

			oldTime = newTime;
		}
	}
}

int main(int argc, char* argv[]) {

	// Variables for collecting configuration information from the command line
	const char* localIP;
	const char* clientIP;
	int localPort;
	int clientPort;

	// IF COMMAND LINE ARGUMENTS ARE PROVIDED, USE THEM
	if (argc >= 5) {

		localIP = argv[1];
		clientIP = argv[2];
		localPort = atoi(argv[3]);
		clientPort = atoi(argv[4]);
	}

	// IF NOT, RESORT TO DEFAULT LOOPBACK ADDRESS AND PORT
	else {
		localIP = LOOPBACK;
		clientIP = LOOPBACK;
		localPort = PORT;
		clientPort = PORT;
	}

	// Socket address variables for local and client.
	sockaddr_in local, client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);

	// Buffer variable for incoming messages.
	char buffer[BUFFERLENGTH];
	// Allocate space for buffer.
	ZeroMemory(buffer, BUFFERLENGTH);

	// Time variables for enforcing the sending rate of 1 R every 3 seconds.
	time_t oldTime, newTime;

	// Variable for sequence number
	int currentSeqNum = 0;

	cout << "********** UPD SERVER - ACTIVE **********" << endl;

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
	if (socketFile == SOCKET_ERROR) {
		cout << "Failed to create socket: " << WSAGetLastError() << endl;
		exit(1);
	}

	// Establish local UDP server parameters
	local.sin_addr.S_un.S_addr = inet_addr(localIP);
	local.sin_family = AF_INET;
	// Port is converted to big endian. 
	local.sin_port = htons(localPort);


	// Bind to the local UDP server. On fail, error and exit.
	if (bind(socketFile, (sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
		cout << "Cannot bind socket. " << WSAGetLastError() << endl;
		exit(1);
	}


	// SERVER WILL REMAIN IDLE UNTIL IT RECEIVES SOMETHING FROM THE CLIENT.
	// Wait for a message from the client.
	receiveMessage(socketFile, buffer, (sockaddr*)&client, &clientLength);


	// ONCE THE CLIENT HAS MADE CONTACT, BEGIN THE COMMUNICATION PROCESS.
	while (true) {

		// Allocate space for buffer.
		ZeroMemory(buffer, BUFFERLENGTH);

		// Note the old time.
		oldTime = time(NULL);

		// Initialise the new time. This will be modified later.
		newTime = time(NULL);

		// Call the function that handles the UDP loop as a thread.
		thread worker(UDPLoop, oldTime, newTime, currentSeqNum, socketFile, client, clientLength, buffer);

		// Variable for storing exit command
		char command[24];

		cin.get(command, 24);

		// If the command is e, modify the boolean value that initiates the exit process.
		if (!strcmp(command, "e")) {

			exitRequest = true;
		}
		// Same as above, but handles upper case E
		if (!strcmp(command, "E")) {

			exitRequest = true;
		}

		// Join the main thread once the keyboard receives input.
		worker.join();
	}

	// Close WinSock
	closesocket(socketFile);

	// End WinSock
	WSACleanup();

	return 0;
}