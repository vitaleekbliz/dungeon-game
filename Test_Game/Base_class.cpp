#include "Base_class.h"
#include "Player_class.h"
#include "GameMode.h"

std::vector<Base_class*> Base_class::m_all_objects;
int Base_class::m_current_object_id = 0;
std::unordered_map<int, Class_id> Base_class::m_class_id_map;

Base_class::Base_class(int object_id, Class_id classID)
{
#ifdef SERVER
	m_current_object_id++;
	m_object_id = m_current_object_id;
#else
	m_object_id = object_id;
#endif

	m_position = { 1, 1};
	m_rotation = 0;

	m_all_objects.push_back(this);

	m_class_id_map[m_object_id] = classID;
}

Base_class::~Base_class() 
{
	auto t = std::find(m_all_objects.begin(), m_all_objects.end(), this);
	if (t != m_all_objects.end())
		m_all_objects.erase(t);
}

Base_class* Base_class::Create_object(Class_id class_id, int id)
{
	switch (class_id)
	{
	case BASE_CLASS:
		return new Base_class(id);

		break;
	case PLAYER_CLASS:
		return new Player_class(id, 0);

		break;
	//case ENEMY_CLASS:
	//	return new Enemy_class(id);

	//	break;
	default:
		return nullptr;
		break;
	}
}

Base_class* Base_class::Get_class_by_id(Class_id& class_id, int id)
{
	auto it = std::find_if(m_all_objects.begin(), m_all_objects.end(), [id](const Base_class* c) {
		return id == c->m_object_id;
		});
	if (it != m_all_objects.end()) {
		if ((*it)->m_class_id_map.find(id) != (*it)->m_class_id_map.end())
			class_id = (*it)->m_class_id_map[id];

		return (*it);
	}
	return nullptr;
}

std::vector<Base_class*>& Base_class::Get_all_classes()
{
	return m_all_objects;
}

void Base_class::Replicate_all_classes()
{
	Replicate_classes_id();
	Replicate_certain_class();
	Replicate_object_pos();

}

void Base_class::Replicate_object_pos() {
	if (server) {
		const int buffer_size = 10;
		int buffer[buffer_size];

		buffer[0] = Header.h1;
		buffer[1] = Header.h2;
		buffer[2] = REPLICATE_OBJECT_POS;
		buffer[3] = 0;/*client id*/
		

		buffer[8] = Footer.f1;
		buffer[9] = Footer.f2;

		for (int i = 0; i < m_all_objects.size(); i++) {
			buffer[4] = m_all_objects[i]->m_object_id;

			memcpy_s(&buffer[5], sizeof(float) * 2, &m_all_objects[i]->m_position, sizeof(int) * 2);
			//buffer[5] = reinterpret_cast<int>(m_all_objects[i]->m_position.x);
			//buffer[6] = m_all_objects[i]->m_position.y;
			buffer[7] = m_all_objects[i]->m_rotation;

			Network::Get().Send_message(buffer_size * sizeof(int), buffer);
		}
	}
	else {
		auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(REPLICATE_OBJECT_POS);

		std::unordered_map<int, bool> used;

		for (int i = 0; i < m_all_objects.size(); i++) {
			used[m_all_objects[i]->m_object_id] = false;
		}

		for (int i = messages.size() - 1; i >= 0; i--) {
			int local_obj_id = messages[i]->m_buffer.get()[4];

			if (used.find(local_obj_id) != used.end() && !used[local_obj_id]) {
				used[local_obj_id] = true;
				Class_id temp;
				Base_class* local_object = Get_class_by_id(temp, local_obj_id);

				if (local_object) {

					memcpy_s(&local_object->m_position, sizeof(float) * 2, &messages[i]->m_buffer.get()[5], sizeof(int) * 2);
					//local_object->m_position = {reinterpret_cast<float*>(messages[i]->m_buffer)[5], reinterpret_cast<float*>(messages[i]->m_buffer)[6] };
					local_object->m_rotation = messages[i]->m_buffer.get()[7];
				}
			}
		}

		Network::Get().GetMessageHandler()->Clear_by_protocol(REPLICATE_OBJECT_POS);
	}
}

void Base_class::Replicate_certain_class()
{
	if (server) {
		auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(CREATE_CLASS);

		for (int i = 0; i < messages.size(); i++) {

			int l_id = messages[i]->m_buffer.get()[4];
			Class_id l_classid;
			if (Get_class_by_id(l_classid, l_id)) {
				int size = 8;
				int* buffer = new int[size];

				buffer[0] = Header.h1;
				buffer[1] = Header.h2;
				buffer[2] = CREATE_CLASS;
				buffer[3] = messages[i]->client_id;
				buffer[4] = l_id;
				buffer[5] = l_classid;
				buffer[6] = Footer.f1;
				buffer[7] = Footer.f2;

				Network::Get().Send_message(size * sizeof(int), buffer, messages[i]->client_id);

				delete[] buffer;
			}

			Network::Get().GetMessageHandler()->Remove_message_from_que(messages[i]);
		}
	}
	else {
		auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(CREATE_CLASS);

		for (int i = 0; i < messages.size(); i++) {
			int local_id = messages[i]->m_buffer.get()[4];
			Class_id l_classID = (Class_id)messages[i]->m_buffer.get()[5];
			Class_id temp;
			if(!Get_class_by_id(temp, local_id))
				Create_object(l_classID, local_id);

			Network::Get().GetMessageHandler()->Remove_message_from_que(messages[i]);
		}
	}
}

void Base_class::Replicate_classes_id()
{
	if (server) {
		int size = m_all_objects.size();

		size += 6;

		int* buffer = new int[size];

		buffer[0] = Header.h1;
		buffer[1] = Header.h2;
		buffer[2] = CHECK_ALL_CLASSES;
		buffer[3] = 0;/*client id should be set in the send message to all clients(client_id=-1)*/

		buffer[size - 2] = Footer.f1;
		buffer[size - 1] = Footer.f2;

		int offset = 4;
		for (int i = 0; i < m_all_objects.size(); i++) {
			buffer[offset + i] = m_all_objects[i]->m_object_id;
		}

		Network::Get().Send_message(size * sizeof(int), buffer);

		delete[] buffer;
	}
	else {
		auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(CHECK_ALL_CLASSES);

		int offset = 4;

		if (messages.size() > 0) {
			auto* messG = messages.back();

			std::unordered_set<int> removed_objects;
			std::unordered_set<int> added_objects;

			std::vector<int> old_objects;
			std::vector<int> new_objects;

			for (int i = 0; i < m_all_objects.size(); i++)
				old_objects.push_back(m_all_objects[i]->m_object_id);

			for (int i = offset; i < messG->m_size - 2; i++)
				new_objects.push_back(messG->m_buffer.get()[i]);


			for (auto& a : old_objects)
				removed_objects.insert(a);

			for (auto& a : new_objects) {
				if (removed_objects.count(a) > 0) {
					removed_objects.erase(a);
				}
				else {
					added_objects.insert(a);
				}
			}

			for (int i = 0; i < m_all_objects.size(); i++) {
				if (removed_objects.count(m_all_objects[i]->m_object_id) > 0) {
					removed_objects.erase(m_all_objects[i]->m_object_id);
					delete m_all_objects[i];
				}
			}

			//temp if code above skipps objects
			if (removed_objects.size() > 0) {
				std::cout << "didnt remove " << removed_objects.size() << " objects\n";
			}
			//

			int size = 7;
			int* buffer = new int[size];

			buffer[0] = Header.h1;
			buffer[1] = Header.h2;
			buffer[2] = CREATE_CLASS;
			buffer[3] = Network::Get().GetID();
			buffer[4] = 0;//object id to be add
			buffer[5] = Footer.f1;
			buffer[6] = Footer.f2;


			for (auto& a : added_objects) {
				buffer[4] = a;
				Network::Get().Send_message(size * sizeof(int), buffer);
			}

			delete[] buffer;

			Network::Get().GetMessageHandler()->Clear_by_protocol(CHECK_ALL_CLASSES);
		}

	}
}


