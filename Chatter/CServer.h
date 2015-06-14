#pragma once

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

void PrintRecvMap(std::map<unsigned int, char*> recvmap);

class CServer
{
public:
	CServer();
	~CServer();
	int ReceiveData(unsigned int client_id, char * recvbuf);
	int ReceiveFromAll(std::map<unsigned int, char *> &recvbuf);
	void SendToClient(unsigned int client_id, char * message, int length);
	void SendToAll(char * message, int length);
	void SendToAllExcept(char * message, int length, unsigned int except);
	void Update();

private:
	bool AcceptNewClient(unsigned int & id);

	UINT32 m_clientId;
	SOCKET m_listenSocket;
	SOCKET m_clientSocket;
	std::map<unsigned int, SOCKET> m_sessions;
	
};

