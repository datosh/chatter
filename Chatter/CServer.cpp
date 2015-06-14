#include "stdafx.h"
#include "CServer.h"

CServer::CServer()
{
	UINT32 m_clientId = 0;

	WSADATA wsaData;
	int iResult;

	m_listenSocket = INVALID_SOCKET;
	m_clientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	// Init winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed with error: " << iResult << std::endl;
		exit(1);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server adress and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		std::cout << "getaddrinfo failed with errror: " << iResult << std::endl;
		WSACleanup();
		exit(1);
	}

	// Create a SOCKET connecting to a server
	m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_listenSocket == INVALID_SOCKET)
	{
		std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	// Make the socket non blocking
	u_long iMode = 1;
	iResult = ioctlsocket(m_listenSocket, FIONBIO, &iMode);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "ioctlsocket failed with error " << WSAGetLastError() << std::endl;
		closesocket(m_listenSocket);
		WSACleanup();
		exit(1);
	}

	// Setup the TCP listening socket
	iResult = bind(m_listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(m_listenSocket);
		WSACleanup();
		exit(1);
	}

	freeaddrinfo(result);

	iResult = listen(m_listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "listen failed with error: " << WSAGetLastError() << std::endl;
		closesocket(m_listenSocket);
		WSACleanup();
		exit(1);
	}
}

CServer::~CServer()
{
}

bool CServer::AcceptNewClient(unsigned int & id)
{
	m_clientSocket = accept(m_listenSocket, NULL, NULL);

	if (m_clientSocket != INVALID_SOCKET)
	{
		UserInfo *p_ui = new UserInfo();
		p_ui->socket = m_clientSocket;
		p_ui->uid = id;
		p_ui->recv_message = new char[DEFAULT_BUFLEN];

		m_sessions.insert(std::pair<unsigned int, UserInfo*>(id, p_ui));
		return true;
	}
	return false;
}

int CServer::ReceiveFromClient(unsigned int client_id)
{
	if (m_sessions.find(client_id) != m_sessions.end())
	{
		auto currentUserInfo = m_sessions[client_id];
		SOCKET currentSocket = currentUserInfo->socket;
		int iResult = recv(currentSocket, currentUserInfo->recv_message, DEFAULT_BUFLEN, 0);
		currentUserInfo->bytes_recved = iResult;

		// Returns 0 when the connections was gracefully closed
		if (iResult == 0)
		{
			std::cout << "Connection " << client_id << " closed!" << std::endl;
			closesocket(currentSocket);
			m_sessions.erase(client_id);
			delete currentUserInfo;
		}
		// Return SOCKET_ERROR if an error occured
		else if (iResult == SOCKET_ERROR)
		{
			int error_code = WSAGetLastError();

			if (error_code == WSAEWOULDBLOCK)
			{
				// There are no new messages for a non blocking socket
				return -1;
			}
			else
			{
				// There was a real problem! 
				std::cout << "recv failed with error: " << error_code << std::endl;
				m_sessions.erase(client_id);
				closesocket(currentSocket);
				delete currentUserInfo;
				std::cout << "Deleted client" << client_id << " from the client list" << std::endl;
				return -1;
			}
		}
		// Return the number of bytes recieved!
		return iResult;
	}
	return -1;
}

int CServer::ReceiveFromAll(void)
{
	int successfullRecvs = 0;

	std::vector<unsigned int> toBeClosed;
	for (auto it = m_sessions.begin(); it != m_sessions.end(); it++)
	{
		auto currentUserInfo = it->second;
		auto currentSocket = currentUserInfo->socket;
		int iResult = recv(currentSocket, currentUserInfo->recv_message, DEFAULT_BUFLEN, 0);
		currentUserInfo->bytes_recved = iResult;

		// Returns 0 when the connection was greacefully closed
		if (iResult == 0)
		{
			toBeClosed.push_back(it->first);
		}
		else if (iResult == SOCKET_ERROR)
		{
			int error_code = WSAGetLastError();

			if (error_code == WSAEWOULDBLOCK)
			{
				// There are no new messages for a nonblocking socket
			}
			else
			{
				// There was a real problem
				std::cout << "recv failed with error: " << error_code << std::endl;
				toBeClosed.push_back(it->first);
			}
		}
		// iResult contains the number of bytes recieved. 
		else
		{
			successfullRecvs++;
		}
	}

	// Close the connections that need to be closed
	for (auto it = toBeClosed.begin(); it != toBeClosed.end(); it++)
	{
		auto userToDelete = m_sessions[*it];
		auto socketToClose = userToDelete->socket;
		m_sessions.erase(*it);
		closesocket(socketToClose);
		delete userToDelete;
		std::cout << "Deleted client" << *it << " from the client list" << std::endl;
	}

	return successfullRecvs;
}

void CServer::SendToClient(unsigned int client_id, char * message, int length)
{
}

void CServer::SendToAll(char * message, int length)
{
	std::vector<unsigned int> toBeDeleted;

	for (auto it = m_sessions.begin(); it != m_sessions.end(); it++)
	{
		auto currentUserInfo = it->second;
		auto currentSocket = currentUserInfo->socket;
		auto iResult = send(currentSocket, message, length, 0);

		if (iResult == SOCKET_ERROR)
		{
			std::cout << "send to " << it->first << " failed with error: " << WSAGetLastError() << std::endl;
			closesocket(currentSocket);
			delete currentUserInfo;
			toBeDeleted.push_back(it->first);
		}
	}

	for (auto it = toBeDeleted.begin(); it != toBeDeleted.end(); it++)
	{
		m_sessions.erase(*it);
		std::cout << "Deleted client " << *it << " from client list" << std::endl;
	}
}

void CServer::SendToAllExcept(char * message, int length, unsigned int except)
{
	std::vector<unsigned int> toBeDeleted;

	for (auto it = m_sessions.begin(); it != m_sessions.end(); it++)
	{
		if (it->first == except)
		{
			continue;
		}

		auto currentUserInfo = it->second;
		auto currentSocket = currentUserInfo->socket;
		auto iResult = send(currentSocket, message, length, 0);

		if (iResult == SOCKET_ERROR)
		{
			std::cout << "send to " << it->first << " failed with error: " << WSAGetLastError() << std::endl;
			closesocket(currentSocket);
			delete currentUserInfo;
			toBeDeleted.push_back(it->first);
		}
	}

	for (auto it = toBeDeleted.begin(); it != toBeDeleted.end(); it++)
	{
		m_sessions.erase(*it);
		std::cout << "Deleted client " << *it << " from client list" << std::endl;
	}
}

void CServer::Update()
{
	if (AcceptNewClient(m_clientId))
	{
		std::cout << "client " << m_clientId << " has been connected to the server" << std::endl;
		m_clientId += 1;
	}
	
	//SendToAll("tick", 5);

	//char buffer[1024];
	//ReceiveData(0, buffer);


	int numRecvs = ReceiveFromAll();
	if (numRecvs > 0)
	{
		PrintRecvMap(m_sessions);

		for (auto it = m_sessions.begin(); it != m_sessions.end(); it++)
		{
			auto currentUserInfo = it->second;
			SendToAllExcept(currentUserInfo->recv_message, DEFAULT_BUFLEN, currentUserInfo->uid);
		}
	}
}

void PrintRecvMap(std::map<unsigned int, UserInfo*> sessions)
{
	std::cout << "_______PRINT_RECV_MAP_______" << std::endl;
	for (auto it = sessions.begin(); it != sessions.end(); it++)
	{
		auto currentUserInfo = it->second;
		if (currentUserInfo->bytes_recved > 0)
		{
			std::cout << "Client(" << currentUserInfo->uid << ") send: " << currentUserInfo->recv_message << std::endl;
		}
	}
	std::cout << std::endl;
}


