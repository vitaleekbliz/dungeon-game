#include "GameMode.h"

GameMode::GameMode()
{
	tick_duration = 0;

	m_tick_rate = 120;
	m_tick_delay = 1000 / m_tick_rate;

	m_tick_start = 0;
	m_tick_end = 0;

	map = nullptr;
	inputs = nullptr;
	local_player = nullptr;
}

GameMode& GameMode::Get()
{
	static GameMode instance;
	return instance;
}

bool GameMode::Render()
{
	if (!server) {

		if (!Engine::Get().HandleEvents(inputs))
			return false;

		Engine::Get().ClearRenderer();

		Engine::Get().calculateDrawingStartPoint(local_player->m_position);

		map->DrawMap();

		Player_class::drawPlayers();

		Engine::Get().Render();
	}

	return true;
}

void GameMode::InitGame()
{
	//INIT engine and network
	if (!server)
		Engine::Get().Init(m_screen_size.x, m_screen_size.y);

	Network::Get().Init();

	//create map 
	map = (server ? new Main_map(m_map_size.x, m_map_size.y) : new Main_map());

	inputs = (server? nullptr: new SDL_Point{ 0, 0 });

	Network::Get().getClientID();
	
}

bool GameMode::Replicate()
{
	map->Replicate_map();

	Base_class::Replicate_all_classes();

	Player_class::replicatePlayers();

	if (server) {
		Player_class::replicateID();
	}
	else {
		if (local_player == nullptr) {
			local_player = Player_class::replicateID();
			return false;
		}
	}

	return true;
}

void GameMode::ClearGame()
{
	delete map;

	if (!server)
		delete inputs;

	Network::Get().Clear();

	if (!server)
		Engine::Get().Clear();
}

void GameMode::tick()
{
	m_tick_end = SDL_GetTicks();
	tick_duration = m_tick_end - m_tick_start;

	if (tick_duration < m_tick_delay) {
		SDL_Delay(m_tick_delay - tick_duration);

		tick_duration = m_tick_delay;
	}

	m_tick_start = SDL_GetTicks();
}

void GameMode::Update()
{
	map->makePassage();
}
