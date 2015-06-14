#include "stdafx.h"
#include "CClient.h"


CClient::CClient()
{
	WSADATA wsaData;
	m_connectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		exit(1);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("192.168.0.10", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		exit(1);
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL;ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		m_connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (m_connectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			exit(1);
		}

		// Connect to server.
		iResult = connect(m_connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(m_connectSocket);
			m_connectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (m_connectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		exit(1);
	}

	// Make the socket non blocking
	u_long iMode = 1;
	iResult = ioctlsocket(m_connectSocket, FIONBIO, &iMode);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "ioctlsocket failed with error " << WSAGetLastError() << std::endl;
		closesocket(m_connectSocket);
		WSACleanup();
		exit(1);
	}
}

CClient::~CClient()
{
}

int CClient::sendData(char * buffer, int length)
{
	int iResult;
	
	iResult = send(m_connectSocket, buffer, length, 0);
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(m_connectSocket);
		WSACleanup();
		return -1;
	}

	return iResult;
}

int CClient::recvData(char * buffer, int length)
{
	int iResult;

	iResult = recv(m_connectSocket, buffer, length, 0);
	if (iResult > 0)
	{
		buffer[iResult] = 0x00;
		std::cout << "Recved(" << iResult << " bytes): " << buffer << std::endl;
	}
	else if (iResult == 0)
	{
		std::cout << "Connection closed" << std::endl;
	}
	else
	{
		int error_code = WSAGetLastError();

		if (error_code == WSAEWOULDBLOCK)
		{
			// There are no new messages for a non blocking socket
			return -1;
		}
		else
		{
			std::cout << "Recv failed with error: " << error_code << std::endl;
			closesocket(m_connectSocket);
			WSACleanup();
			exit(1);
		}
	}

	return iResult;
}
