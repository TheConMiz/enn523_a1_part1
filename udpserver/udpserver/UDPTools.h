#pragma once
// To deal with deprecation warnings
#pragma warning(disable : 4996)

#define PORT 54000
#define LOOPBACK "127.0.0.1"

#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <chrono>
#include <time.h>
#include <thread>
#include <iomanip>


using namespace std;

// Memory Allocation.
const int BUFFERLENGTH = 1024;
const int IP_LENGTH = 256;

// UDP Messages
const char* ack = "ACK";
const char* ackE = "ACK E";
const char* ackR = "ACK R";
const char* e = "E";
const char* r = "R";

// Combines the receipt of message from the client and error-checking
void receiveMessage(SOCKET socketFile, char* message, sockaddr* from, int* fromlen) {

	// Do something to collect the sequenceNum

	int messageIn = recvfrom(socketFile, message, BUFFERLENGTH, 0, from, fromlen);

	// If no message received, error.
	if (messageIn == SOCKET_ERROR) {
		cout << "Unable to receive from server. " << WSAGetLastError() << endl;
	}
}

// Returns a timestamp of format hh:mm:ss:ms
// Problem -- Milliseconds not working as expected. Could be because of low latency associated with loopback. Or, the clock does not have as high a resolution.
string getTimestamp() {

	const auto now = std::chrono::system_clock::now();

	const auto rawTime = std::chrono::system_clock::to_time_t(now);

	// Need this to generate milliseconds value separately
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;


	// Required for formatting time value as string
	std::stringstream nowSs;

	// Format the raw time value into a timestamp string of format hh:mm:ss:ms
	nowSs << std::put_time(std::localtime(&rawTime), "%a %b %d %Y %T") << ':' << std::setfill('0') << std::setw(3) << ms.count();

	return nowSs.str();
}

