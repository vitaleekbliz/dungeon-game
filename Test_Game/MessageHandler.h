#pragma once
#include "Header.h"
#include <memory>

enum PROTOCOL {
	ESTEBLISH_CONNECTION_ID = 1,
	REPLICATE_MAP_SIZE,
	REPLICATE_MAP,
	CHECK_ALL_CLASSES,
	CREATE_CLASS,
	DESTROY_CLASS,
	REPLICATE_CLASS,
	REPLICATE_PLAYER_ID,
	REPLICATE_INPUTS,
	REPLICATE_MOVING_PLAYERS,
	REPLICATE_OBJECT_POS
};

struct Message {
	//int* m_buffer;
	int m_size;
	PROTOCOL protocol;
	int client_id;
	std::unique_ptr<int[]> m_buffer;

	Message() = delete;
	Message(const Message& other) = delete;
	Message(int size, int* buffer) : m_size(size) {
		//m_buffer = new int[size];
		m_buffer = std::make_unique<int[]>(size);
		memcpy_s(m_buffer.get(), sizeof(int) * size, buffer, sizeof(int) * size);
		protocol = *((PROTOCOL*)buffer + 2);
		client_id = *((int*)buffer + 3);
	}
	~Message() {
	}
};

class MessageHandler
{
public:
	void Clear_by_protocol(PROTOCOL p);
	std::vector<Message*>& Get_Message_que_by_protocol(PROTOCOL p);
	void Add_message_to_que(Message* message);
	void Remove_message_from_que(Message* message);

private:
	std::map<PROTOCOL, std::vector<Message*>> m_message_que;
};

