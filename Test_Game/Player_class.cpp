#include "Player_class.h"

std::vector<Player_class*> Player_class::m_all_players;

Player_class::Player_class(int id, int player_id) : Base_class(id, PLAYER_CLASS)
{
	m_all_players.push_back(this);

    m_player_id = player_id;

    m_player_speed = 5;
    m_is_moving = false;

    current_animation_tick = 0;
    //if FPS is dynamicly changed animation will be off
    ticks_for_animation = GameMode::Get().m_tick_rate / 12;
    current_animation = 0;
}

Player_class::~Player_class()
{
	auto it = std::find(m_all_players.begin(), m_all_players.end(), this);
	if(it != m_all_players.end())
		m_all_players.erase(it);
}

void Player_class::replicatePlayers()
{
    if (server)
        checkForNewPlayersServer();

    handleInputs(); 
    replicateMovingPlayers();
}

void Player_class::replicateMovingPlayers() {
    if (server)
        replicateMovingPlayersServer();
    else
        replicateMovingPlayersClient();
}

void Player_class::replicateMovingPlayersServer() {
    int info_size = m_all_players.size() * 2;

    int buffer_size = info_size + 6;

    int* buffer = new int[buffer_size];

    buffer[0] = Header.h1;
    buffer[1] = Header.h2;

    buffer[2] = REPLICATE_MOVING_PLAYERS;
    buffer[3] = 0;// client id is set is sendMessage function;

    buffer[buffer_size - 2] = Footer.f1;
    buffer[buffer_size - 1] = Footer.f2;

    int offset = 4;

    for (int i = 0; i < m_all_players.size(); i++) {
        buffer[offset + i * 2] = m_all_players[i]->m_object_id;
        buffer[offset + i * 2 + 1] = m_all_players[i]->m_is_moving;
    }

    Network::Get().Send_message(buffer_size * sizeof(int), buffer);
}

void Player_class::replicateMovingPlayersClient() {
    auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(REPLICATE_MOVING_PLAYERS);

    if (messages.size() > 0) {
        auto& last_message = messages.back();

        std::unordered_map<int, bool> local_moving_players;

        //input current moving state of players
        for (int i = 0; i < m_all_players.size(); i++) {
            local_moving_players[m_all_players[i]->m_object_id] = m_all_players[i]->m_is_moving;
        }

        int offset = 4;

        //update moving states
        for (int i = offset; i < last_message->m_size - 1/*cause message contain object id and is_moving property in pairs*/; i += 2) {
            if (local_moving_players.find(last_message->m_buffer.get()[i]) != local_moving_players.end())
                local_moving_players[last_message->m_buffer.get()[i]] = last_message->m_buffer.get()[i + 1];
        }

        for (int i = 0; i < m_all_players.size(); i++) {
            if (local_moving_players.find(m_all_players[i]->m_object_id) != local_moving_players.end())
                m_all_players[i]->m_is_moving = local_moving_players[m_all_players[i]->m_object_id];
        }

        Network::Get().GetMessageHandler()->Clear_by_protocol(REPLICATE_MOVING_PLAYERS);
    }
}

void Player_class::handleInputs()
{
    if (server)
        handleInputsServer();
    else
        handleInputsClient();
}

void Player_class::handleInputsServer()
{
    if (server) {
        auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(REPLICATE_INPUTS);

        std::unordered_map<int, SDL_Point> players_inputs;
        std::unordered_map<int, bool> used;

        for (int i = 0; i < m_all_players.size(); i++) {
            int p_id = m_all_players[i]->m_player_id;
            players_inputs[p_id] = { 0, 0 };
            used[p_id] = false;

        }

        for (int i = messages.size() - 1; i >= 0; i--) {
            int c_id = messages[i]->client_id;
            if (used.find(c_id) == used.end() || used[c_id])
                continue;
            else {
                players_inputs[c_id] = { messages[i]->m_buffer.get()[4], messages[i]->m_buffer.get()[5] };
                used[c_id] = true;
            }
        }

        for (auto it = players_inputs.begin(); it != players_inputs.end(); it++) {
            Player_class* l_player = getPlayerByID(it->first);
            if (l_player)
                l_player->movePlayer(it->second);
        }

        Network::Get().GetMessageHandler()->Clear_by_protocol(REPLICATE_INPUTS);
    }
}

void Player_class::handleInputsClient()
{
    if (!server) {
        const int buffer_size = 8;
        int buffer[buffer_size];

        SDL_Point* inputs = GameMode::Get().inputs;

        buffer[0] = Header.h1;
        buffer[1] = Header.h2;
        buffer[2] = REPLICATE_INPUTS;
        buffer[3] = Network::Get().GetID();
        buffer[4] = inputs->x;
        buffer[5] = inputs->y;
        buffer[6] = Footer.f1;
        buffer[7] = Footer.f2;

        Network::Get().Send_message(buffer_size * sizeof(int), buffer);
    }
}

void Player_class::movePlayer(SDL_Point inputs_vector)
{
    if (inputs_vector.x == 0 && inputs_vector.y == 0) {
        m_is_moving = false;
        return;
    }
    else {
        m_is_moving = true;
    }

    //set rotation
    if (inputs_vector.x == 1) {
        if (inputs_vector.y == 1) {
            m_rotation = 45;
        }
        else if (inputs_vector.y == 0) {
            m_rotation = 90;
        }
        else if (inputs_vector.y == -1) {
            m_rotation = 135;
        }
    }
    else if (inputs_vector.x == 0) {
        if (inputs_vector.y == 1) {
            m_rotation = 0;
        }
        else if (inputs_vector.y == -1) {
            m_rotation = 180;
        }
    }
    else if (inputs_vector.x == -1) {
        if (inputs_vector.y == 1) {
            m_rotation = 315;
        }
        else if (inputs_vector.y == 0) {
            m_rotation = 270;
        }
        else if (inputs_vector.y == -1) {
            m_rotation = 225;
        }
    }

    float dx = m_player_speed * inputs_vector.x * GameMode::Get().tick_duration / 1000;
    float dy = m_player_speed * inputs_vector.y * GameMode::Get().tick_duration / 1000;

    dx += this->m_position.x;
    dy += this->m_position.y;

    if (GameMode::Get().map->isCellWalkable(std::round(this->m_position.x), std::round(dy))) {
        this->m_position.y = dy;
    }
    if (GameMode::Get().map->isCellWalkable(std::round(dx), std::round(this->m_position.y))) {
        this->m_position.x = dx;
    }

}

Base_class* Player_class::replicateID()
{
    if (server) {
        auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(REPLICATE_PLAYER_ID);

        for (int i = 0; i < messages.size(); i++) {
            int player_id = messages[i]->m_buffer.get()[3];

            Player_class* local_player = getPlayerByID(player_id);

            if (local_player != nullptr) {
                int object_id = local_player->m_object_id;


                int buffer_size = 7;

                int* buffer = new int[buffer_size];
                buffer[0] = Header.h1;
                buffer[1] = Header.h2;

                buffer[2] = REPLICATE_PLAYER_ID;
                buffer[3] = messages[i]->client_id;

                buffer[4] = object_id;

                buffer[5] = Footer.f1;
                buffer[6] = Footer.f2;

                Network::Get().Send_message(buffer_size * sizeof(int), buffer);

                delete[] buffer;
            }
        }

        return nullptr;
    }
    else {
        int buffer_size = 6;

        int* buffer = new int[buffer_size];
        buffer[0] = Header.h1;
        buffer[1] = Header.h2;

        buffer[2] = REPLICATE_PLAYER_ID;
        buffer[3] = Network::Get().GetID();

        buffer[4] = Footer.f1;
        buffer[5] = Footer.f2;

        Network::Get().Send_message(buffer_size * sizeof(int), buffer);

        delete[] buffer;


        SDL_Delay(200);

        auto& messages = Network::Get().GetMessageHandler()->Get_Message_que_by_protocol(REPLICATE_PLAYER_ID);

        int object_id = messages.back()->m_buffer.get()[4];
        Class_id local_class_id;

        Network::Get().GetMessageHandler()->Clear_by_protocol(REPLICATE_PLAYER_ID);
        
        return Base_class::Get_class_by_id(local_class_id, object_id);
    }
}

void Player_class::checkForNewPlayersServer()
{
	auto active_players = Network::Get().getActivePlayersListID();

	std::unordered_set<int> existing_players;

	for (int i = 0; i < m_all_players.size(); i++)
		existing_players.insert(m_all_players[i]->m_player_id);

    std::unordered_set<int> players_to_erase;

    for (auto& a : active_players) {
        if (existing_players.count(a) > 0) {
            players_to_erase.insert(a);
        }
    }

    for (auto& a : players_to_erase) {
        existing_players.erase(a);
        active_players.erase(a);
    }

    /*std::cout << "Added players: ";
    for (auto& a : active_players) {
        std::cout << a << ' ';
    }

    std::cout << "\nRemoved Players: ";
    for (auto& a : existing_players) {
        std::cout << a << ' ';
    }*/

    for (auto& a : active_players) {
        new Player_class(0, a);
    }

    for (auto& a : existing_players) {
        Player_class* l_p = getPlayerByID(a);
        if (l_p)
            delete l_p;
    }
}

Player_class* Player_class::getPlayerByID(int id)
{
    auto it = std::find_if(m_all_players.begin(), m_all_players.end(), [&id](const Player_class* other) {
        return id == other->m_player_id;
        });

    if (it != m_all_players.end())
        return (*it);

    return nullptr;
}

void Player_class::drawPlayers()
{
    for (int i = 0; i < m_all_players.size(); i++) {
        m_all_players[i]->drawPlayer();
    }
}

void Player_class::drawPlayer()
{
    TEXTURES mesh;

    if (this->m_is_moving) {
        current_animation_tick++;
        if (current_animation_tick > ticks_for_animation) {
            current_animation_tick = 0;
            current_animation++;
            if (current_animation > 1)
                current_animation = -1;
        }
    }
    else {
        current_animation = 0;
        current_animation_tick = 0;
    }

    switch (current_animation)
    {
    case 1:
        mesh = PLAYER_MOVING_1;
        break;
    case -1:
        mesh = PLAYER_MOVING_2;
        break;
    default:
        mesh = PLAYER_STAYING;
        break;
    }

    Engine::Get().Draw_object(m_position, mesh, m_rotation);
}

std::vector<Player_class*>& Player_class::getAllPlayers()
{
    return m_all_players;
}