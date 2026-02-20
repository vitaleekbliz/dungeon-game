#include "MessageHandler.h"

void MessageHandler::Clear_by_protocol(PROTOCOL p)
{
	auto& messages = Get_Message_que_by_protocol(p);
	for (auto it = messages.begin(); it != messages.end(); it++) {
		delete (*it);
	}
	messages.clear();
}

std::vector<Message*>& MessageHandler::Get_Message_que_by_protocol(PROTOCOL p) {
	return m_message_que[p];
}

void MessageHandler::Add_message_to_que(Message* message)
{
	///temp
	/*nt size = message->m_size;
	int* buffer = message->m_buffer;
	std::cout << "Added message : ";
	for (int i = 0; i < size; i++)
		std::cout << buffer[i] << ' ';
	std::cout << "\n\n";*/
	///
	m_message_que[message->protocol].push_back(message);
}

void MessageHandler::Remove_message_from_que(Message* message)
{
	std::vector<Message*>& l_messages = m_message_que[message->protocol];
	auto it = std::find(l_messages.begin(), l_messages.end(), message);
	if (it != l_messages.end()) {
		///temp
		/*int size = message->m_size;
		int* buffer = message->m_buffer;
		std::cout << "Removed message : ";
		for (int i = 0; i < size; i++)
			std::cout << buffer[i] << ' ';
		std::cout << "\n\n";*/
		///
		l_messages.erase(it);
		delete message;
	}
	else
		std::cout << "Can't find message to delete\n";
}
