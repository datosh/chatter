#pragma once

class CClient
{
public:
	CClient();
	~CClient();

	int sendData(char * buffer, int length);
	int recvData(char * buffer, int length);

private:
	SOCKET m_connectSocket;
};

