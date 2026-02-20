#include "Main_map.h"
#include "Player_class.h"


Main_map::Main_map()
{
	this->x = 0;
	this->y = 0;
	map_replicated_on_client = false;
}

Main_map::Main_map(int x , int y)
{
	if (server) {
		this->x = x;
		this->y = y;

		Generate_map(x, y);
	}
	else {
		this->x = 0;
		this->y = 0;
	}
}

std::vector<std::vector<int>>& Main_map::Get_map()
{
	return m_map;
}

void Main_map::Replicate_map()
{
	if (server) {

		Replicate_map_size();

		Replicate_map_content();

	} else {
		if (x == 0 || y == 0) {

			Replicate_map_size();

		}
		if (!map_replicated_on_client) {

			Replicate_map_content();

		}
		
	}
}

void Main_map::Replicate_map_size()
{
	//TODO prevent package lost 
	if (server) {

		auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(REPLICATE_MAP_SIZE);

		for (int i = 0; i < messages.size(); i++) {
			if (messages[i] && messages[i]->protocol == REPLICATE_MAP_SIZE) {
				int buf_size = 8;

				int* buffer = new int[buf_size];
				buffer[0] = Header.h1;
				buffer[1] = Header.h2;
				buffer[2] = REPLICATE_MAP_SIZE;
				buffer[3] = messages[i]->client_id;
				buffer[4] = x;
				buffer[5] = y;
				buffer[6] = Footer.f1;
				buffer[7] = Footer.f2;
				Network::Get().Send_message(sizeof(int) * buf_size, buffer, messages[i]->client_id);
				delete[] buffer;

				Network::Get().GetMessageHandler()->Remove_message_from_que(messages[i]);
				break;
			}
		}
	}
	else {
		int buf_size = 6;
		int* buffer = new int[buf_size];

		buffer[0] = Header.h1;
		buffer[1] = Header.h2;
		buffer[2] = REPLICATE_MAP_SIZE;
		buffer[3] = Network::Get().GetID();
		buffer[4] = Footer.f1;
		buffer[5] = Footer.f2;

		Network::Get().Send_message(sizeof(int) * buf_size, buffer);

		delete[] buffer;

		auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(REPLICATE_MAP_SIZE);
		while (x == 0 || y == 0) {
			for (int i = 0; i < messages.size(); i++) {
				if (messages[i] && messages[i]->protocol == REPLICATE_MAP_SIZE && messages[i]->client_id == Network::Get().GetID()) {
					x = messages[i]->m_buffer.get()[4];
					y = messages[i]->m_buffer.get()[5];
					m_map.resize(x);
					for (int i = 0; i < x; i++) {
						m_map[i].resize(y);
					}
					//clear all messages within protocol below
					//Network::Get().GetMessageHandler()->Remove_message_from_que(messages[i]);
				}
			}
		}

		Network::Get().GetMessageHandler()->Clear_by_protocol(REPLICATE_MAP_SIZE);
	}
}

void Main_map::Replicate_map_content()
{
	if (server) {

		auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(REPLICATE_MAP);

		for (int i = 0; i < messages.size(); i++) {
			if (messages[i] && messages[i]->protocol == REPLICATE_MAP) {
				int n = messages[i]->m_buffer.get()[4];
				
				if (n >= 0 && n < x) {
					int buf_size = 7 + y;
					int* buffer = new int[buf_size];

					buffer[0] = Header.h1;
					buffer[1] = Header.h2;
					buffer[2] = REPLICATE_MAP;
					buffer[3] = messages[i]->client_id;
					buffer[4] = n;

					int offset = 5;

					for (int m = 0; m < y; m++) {
						buffer[offset + m] = m_map[n][m];
					}
					buffer[buf_size - 2] = Footer.f1;
					buffer[buf_size - 1] = Footer.f2;

					Network::Get().Send_message(sizeof(int) * buf_size, buffer, messages[i]->client_id);

					delete[] buffer;
				}

				Network::Get().GetMessageHandler()->Remove_message_from_que(messages[i]);
			}
		}
	}
	else {
		 
		auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(REPLICATE_MAP);

		int buf_size = 7;
		int* buffer = new int[buf_size];

		buffer[0] = Header.h1;
		buffer[1] = Header.h2;
		buffer[2] = REPLICATE_MAP;
		buffer[3] = Network::Get().GetID();
		//buffer[4] = row index//
		buffer[5] = Footer.f1;
		buffer[6] = Footer.f2;

		for (int i = 0; i < x; i++) {
			
			buffer[4] = i;

			Network::Get().Send_message(sizeof(int) * buf_size, buffer);

			int tryes = 0;

			while (true) {
				bool found = false;

				if (++tryes > 50) {
					i--;
					break;
				}

				for (int k = 0; k < messages.size(); k++) {
					if (messages[i] && messages[k]->protocol == REPLICATE_MAP && messages[k]->client_id == Network::Get().GetID() && messages[k]->m_buffer.get()[4] == i) {

						int offset = 5;

						for (int j = 0; j < y; j++) {
							m_map[i][j] = messages[k]->m_buffer.get()[offset + j];
						}

						//Network::Get().GetMessageHandler()->Remove_message_from_que(messages[k]);

						found = true;
						break;
					}
				}
				if (found)
					break;

				SDL_Delay(1);
			}
		}
		delete[] buffer;

		Network::Get().GetMessageHandler()->Clear_by_protocol(REPLICATE_MAP);

		map_replicated_on_client = true;

	}


}

void Main_map::Generate_map(int l_x, int l_y)
{
	x = l_x;
	y = l_y;

	if (x % 2 == 0)
		x++;
	if (y % 2 == 0)
		y++;
	m_map.resize(x);
	for (int i = 0; i < x; i++) {
		m_map[i].resize(y);
		for (int j = 0; j < y; j++) {
			if (i == 0 || j == 0 || i == x - 1 || j == y - 1)
				m_map[i][j] = 0;
			else if (i % 2 == 1 && j % 2 == 1)
				m_map[i][j] = 1;
			else
				m_map[i][j] = 0;
		}
	}

	std::vector<SDL_Point> step_trace;

	SDL_Point start{ x - 2, y - 2 };

	step_trace.push_back(start);

	std::vector<std::vector<int>> visisted_cells;

	visisted_cells.resize(x);
	for (int i = 0; i < x; i++) {
		visisted_cells[i].resize(y);
	}

	visisted_cells[start.x][start.y]++;

	srand(time(0));

	while (!step_trace.empty()) {
		auto res = Get_available_directions(step_trace.back(), visisted_cells);

		if (!res.empty()) {
			SDL_Point next = res[rand() % res.size()];

			m_map[(next.x + step_trace.back().x) / 2][(next.y + step_trace.back().y) / 2] = 1;

			visisted_cells[next.x][next.y]++;

			step_trace.push_back(next);
		}
		else {
			step_trace.pop_back();
		}
	}
}

std::vector<SDL_Point> Main_map::Get_available_directions(SDL_Point current_pos, std::vector<std::vector<int>>& visisted_cells)
{
	std::vector<SDL_Point> ret;

	for (int i : {2, 0, -2}) {
		for (int j : {2, 0, -2}) {
			if (i == 0 || j == 0) {
				SDL_Point local_point{ current_pos.x + i, current_pos.y + j };
				if (local_point.x >= 0 && local_point.x < x && local_point.y >= 0 && local_point.y < y)
					if (visisted_cells[local_point.x][local_point.y] == 0)
						ret.push_back(local_point);
			}
		}
	}

	return ret;
}

bool Main_map::isCellWalkable(int l_x, int l_y)
{
	if (l_x < 0 || l_x >= x || l_y < 0 || l_y >= y)
		return false;
	else
		return m_map[l_x][l_y] > 0;
}

void Main_map::makePassage()
{
	auto& all_players = Player_class::getAllPlayers();

	for (int i = 0; i < all_players.size(); i++) {
		SDL_Point location = { (int)std::round(all_players[i]->m_position.x), (int)std::round(all_players[i]->m_position.y) };

		if (location.x >= 0 && location.x < x && location.y >= 0 && location.y < y) {
			if (m_map[location.x][location.y] == 1)
				m_map[location.x][location.y] = 2;
		}
	}
}

void Main_map::DrawMap()
{
	//Draw cells 
	for (int i = 0; i < m_map.size(); i++) {
		for (int j = 0; j < m_map[i].size(); j++) {
			switch (m_map[i][j])
			{
			case 0:
				Engine::Get().Draw_object({ (float)i, (float)j }, WALL);
				break;
			case 1:
				Engine::Get().Draw_object({ (float)i, (float)j }, GRASS);
				break;
			case 2:
				Engine::Get().Draw_object({ (float)i, (float)j }, MUD);
				break;
			default:
				break;
			}
		}
	}
	//TODO draw end game Trophie
	//Engine::Get().Draw_object({(float)x - 2, (float)y - 2}, TROPHIE);
}