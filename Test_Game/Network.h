#pragma once
#include "Header.h"
#include "MessageHandler.h"

#ifdef SERVER
#define server true
#else 
#define server false
#endif

struct{
	const int h1 = 0xABCD;
	const int h2 = 0xEF8B;
}Header;

struct {
	const int f1 = 0xABCD;
	const int f2 = 0xABCD;
}Footer;

class Network {
public:
	static Network& Get();

	void getClientID();
	int GetID();

	bool Init();
	void Clear();

	MessageHandler* GetMessageHandler();
	void Add_message_to_que(int size, int* buffer);

	void Send_message(int size, int* buffer, int client_id = -1);
	void Receive_messages();

	std::unordered_set<int> getActivePlayersListID();

private:
	int id = 0;
	IPaddress m_serverIP;
	TCPsocket m_serverSocket;
	int m_serverPort = 49777; 

	int* m_buffer;
	const int m_buffer_size = 2000;

	MessageHandler* m_message_handler;
	std::thread* m_receive_message;

#ifdef SERVER
	std::map<int, TCPsocket> m_connected_clients;
#endif

#ifdef CLIENT
	const char* m_serverIPString = "10.190.9.230";// "212.90.60.47";"192.168.0.108";
#endif
	Network() { m_buffer = new int[m_buffer_size]; };
	Network(const Network& other) = delete;
	~Network() { delete[] m_buffer; };
};
