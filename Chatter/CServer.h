#pragma once

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

struct UserInfo 
{
	unsigned int uid;
	SOCKET socket;
	int bytes_recved;
	char * recv_message;

	~UserInfo() { delete recv_message; }
};

void PrintRecvMap(std::map<unsigned int, UserInfo*> recvmap);

class CServer
{
public:
	CServer();
	~CServer();
	int ReceiveFromClient(unsigned int client_id);
	int ReceiveFromAll(void);
	void SendToClient(unsigned int client_id, char * message, int length);
	void SendToAll(char * message, int length);
	void SendToAllExcept(char * message, int length, unsigned int except);
	void Update();

private:
	bool AcceptNewClient(unsigned int & id);

	UINT32 m_clientId;
	SOCKET m_listenSocket;
	SOCKET m_clientSocket;
	std::map<unsigned int, UserInfo*> m_sessions;
};

