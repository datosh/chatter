// ChatterClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CClient.h"

enum WM_MSGS {
	WM_HELLO_WORLD = WM_USER + 1,
	WM_REGISTER_DATA_THREAD,
	WM_REGISTER_NETW_THREAD,
	WM_SEND_MSG,
};

class NetworkPackage
{
public:
	NetworkPackage(UINT32 data1, UINT32 data2);
	~NetworkPackage();

	void SumNumbers();

private:
	UINT32 m_data1;
	UINT32 m_data2;
};

NetworkPackage::NetworkPackage(UINT32 data1, UINT32 data2) : m_data1(data1), m_data2(data2)
{
}

NetworkPackage::~NetworkPackage()
{
}

void NetworkPackage::SumNumbers()
{
	std::cout << m_data1 << "+" << m_data2 << "=" << m_data1 + m_data2;
}

class DataPackage
{
public:
	DataPackage(UINT32 data1, UINT32 data2);
	~DataPackage();

	void SumNumbers();

private:
	UINT32 m_data1;
	UINT32 m_data2;
};

DataPackage::DataPackage(UINT32 data1, UINT32 data2) : m_data1(data1), m_data2(data2)
{
}

DataPackage::~DataPackage()
{
}

void DataPackage::SumNumbers()
{
	std::cout << m_data1 << "+" << m_data2 << "=" << m_data1 + m_data2;
}

DWORD WINAPI DataStart(LPVOID lpParam)
{
	MSG msg;
	BOOL running = TRUE;

	DWORD networkThreadId = 0;

	while (running)
	{
		// Check incommming messages
		while (PeekMessage(&msg, NULL, WM_USER, 0, PM_REMOVE) > 0)
		{
			switch (msg.message)
			{
			case WM_HELLO_WORLD:
				std::cout << "Data::HelloWorld" << std::endl;
				break;
			case WM_REGISTER_NETW_THREAD:
				networkThreadId = *reinterpret_cast<DWORD*>(msg.lParam);
				break;
			default:
				break;
			}
		}

		// Do the other stuff
		char message[DEFAULT_BUFLEN];
		std::cin.getline(message, DEFAULT_BUFLEN);
		if (networkThreadId != 0)
		{
			PostThreadMessage(networkThreadId, WM_SEND_MSG, 0, reinterpret_cast<LPARAM>(message));
		}
	}

	return 0;
}

DWORD WINAPI NetworkStart(LPVOID lpParam)
{
	MSG msg;
	BOOL running = TRUE;
	DWORD dataThreadId;
	CClient *cc = new CClient();

	while (running)
	{
		// Check incommming messages
		while (PeekMessage(&msg, NULL, WM_USER, 0, PM_REMOVE) > 0)
		{
			switch (msg.message)
			{
			case WM_HELLO_WORLD:
				std::cout << "Hello World" << std::endl;
				break;
			case WM_REGISTER_DATA_THREAD:
				dataThreadId = *reinterpret_cast<DWORD*>(msg.lParam);
				break;
			case WM_SEND_MSG:
				cc->sendData(reinterpret_cast<char*>(msg.lParam), DEFAULT_BUFLEN);
			default:
				break;
			}
		}

		// Do other stuff
		char recvBuffer[DEFAULT_BUFLEN];
		cc->recvData(recvBuffer, DEFAULT_BUFLEN);
		Sleep(10);
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{

	DWORD NetworkThreadId;
	HANDLE hNetworkThread = CreateThread(NULL, 0, NetworkStart, NULL, 0, &NetworkThreadId);

	DWORD DataThreadId;
	HANDLE hDataThread = CreateThread(NULL, 0, DataStart, NULL, 0, &DataThreadId);

	// Wait for the threads to be created
	Sleep(100);

	PostThreadMessage(NetworkThreadId, WM_REGISTER_DATA_THREAD, 0, reinterpret_cast<LPARAM>(&DataThreadId));
	PostThreadMessage(DataThreadId, WM_REGISTER_NETW_THREAD, 0, reinterpret_cast<LPARAM>(&NetworkThreadId));

	WaitForSingleObject(hNetworkThread, INFINITE);
	WaitForSingleObject(hDataThread, INFINITE);

	return 0;
}