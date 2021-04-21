// UDP SERVER

#include <iostream>
#include <string>

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>


#include <chrono>
#include <time.h>
#include <thread>
#include <iomanip>


#define PORT 54000
#define LOOPBACK "127.0.0.1"

// ASCII Codes for keyboard input
#define KEY_E 69
#define KEY_e 101


// To deal with deprecation warnings
#pragma warning(disable : 4996)

using namespace std;

// Global variables for memory allocation.
const int BUFFERLENGTH = 1024;
const int IP_LENGTH = 256;

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

		cout << "Round Trip Time: " << ms << "ms" << endl << endl;
	}
};

// Combines the sending of message to the client and error-checking
void sendMessage(SOCKET socketFile, const char* message, int sequenceNum, const sockaddr *to, int tolen) {

	// Do something here to send both message and sequence number

	int sendStatus = sendto(socketFile, message, BUFFERLENGTH, 0, to, tolen);

	// If unable to send, print error message.
	if (sendStatus == SOCKET_ERROR) {
		cout << "Unable to send message. Error: " << WSAGetLastError() << endl;
	}

}

// Combines the receipt of message from the client and error-checking
void receiveMessage(SOCKET socketFile, char* message, sockaddr *from, int *fromlen) {

	// Do something to collect the sequenceNum

	int messageIn = recvfrom(socketFile, message, BUFFERLENGTH, 0, from, fromlen);

	// If no message received, error.
	if (messageIn == SOCKET_ERROR) {
		cout << "Unable to receive from server. " << WSAGetLastError() << endl;
	}
}

// Returns a timestamp of format hh:mm:ss:ms
// Problem -- Milliseconds not working as expected. Could be because of low latency associated with loopback
string getTimestamp() {

	const auto now = std::chrono::system_clock::now();

	const auto rawTime = std::chrono::system_clock::to_time_t(now);
	
	// Need this to generate milliseconds value separately
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
	
	// Required for formatting time value as string
	std::stringstream nowSs;
	
	nowSs << std::put_time(std::localtime(&rawTime), "%a %b %d %Y %T") << ':' << std::setfill('0') << std::setw(3) << ms.count();
	
	return nowSs.str();
}

bool udpWorkFinished = false;

void UDPLoop(int test) {
	while (!udpWorkFinished) {
		cout << test << endl;
	}
}

int main(int argc, char* argv[]) {

	// Socket address variables for local and client.
	sockaddr_in local, client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);

	// Buffer variable for incoming messages.
	char buffer[BUFFERLENGTH];

	time_t oldTime, newTime;


	// Commands
	const char* ack = "ACK";
	const char* ackE = "ACK E";
	const char* ackR = "ACK R";
	const char* e = "E";
	const char* r = "R";

	// Variable for sequence number
	int currentSeqNum = 0;
	int oldSeqNum = currentSeqNum;

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

	// ROUND 1 -  RECEIVE A MESSAGE FROM THE CLIENT TO ASCERTAIN ITS CREDENTIALS. REMOVE THIS LATER.
	// Allocate space for buffer.
	ZeroMemory(buffer, BUFFERLENGTH);

	// Wait for a message. Sending a sequence number of -1 here. ignore it.
	receiveMessage(socketFile, buffer, (sockaddr*)&client, &clientLength);

	// Allocate space for client's IP information.
	char clientIp[IP_LENGTH];
	ZeroMemory(clientIp, IP_LENGTH);
	// Store IP information in clientIp variable
	inet_ntop(AF_INET, &client.sin_addr, clientIp, IP_LENGTH);

	// REMOVE LATER.
	cout << "NOT PART OF PROJECT - Message from client " << clientIp << ": " << buffer << endl;

	// Threading is what we need
	int test = 5;

	thread worker(UDPLoop, test);

	char command[24];

	cin.get(command, 24);

	// THREAD TEST
	// If the command is e, handle exit stuff. TODO: make it send the required messages.
	if (!strcmp(command, "e")) {

		cout << "Exit" << command << endl;
		udpWorkFinished = true;
	}



	// Join the main thread once the keyboard receives input
	worker.join();


	// THIS IS WHERE THE CODE SHOULD START.
	while (true) {

		// Allocate space for buffer.
		ZeroMemory(buffer, BUFFERLENGTH);
		
		// Note the old time.
		oldTime = time(NULL);

		while (true) {

			newTime = time(NULL);

			if (newTime - oldTime == 3) {

				// Provides round trip delay.
				RoundTripTimer roundTripTimer;

				++currentSeqNum;

				sendMessage(socketFile, r, currentSeqNum, (sockaddr*)&client, clientLength);

				// Wait for a message.
				receiveMessage(socketFile, buffer, (sockaddr*)&client, &clientLength);

				// If received message is ACK R:
				if (!strcmp(buffer, ackR)) {
					
					cout << buffer << " seqno " << getTimestamp() << endl;

					// Send ACK.
					sendMessage(socketFile, ack, currentSeqNum, (sockaddr*)&client, clientLength);

					// Wait for a response. 
					receiveMessage(socketFile, buffer, (sockaddr*)&client, &clientLength);
				}

				// If received message is ACK: 
				if (!strcmp(buffer, ack)) {
					
					// Get Current Time
					auto now = std::chrono::system_clock::now();

					cout << buffer << " seqno " << getTimestamp() << endl;

					cout << endl;
				}

				oldTime = newTime;
			}
		}
	}

	// Close WinSock
	closesocket(socketFile);

	// End WinSock
	WSACleanup();

	return 0;
}