#pragma once

#include "Header.h"
#include "Engine.h"
#include "main_menu.h"
#include "Network.h"
#include "MessageHandler.h"
#include "Main_map.h"
#include "Player_class.h"

class GameMode
{
public:
	static GameMode& Get();

	void InitGame();
	void ClearGame();

	//return false if game is not ready to run yet
	bool Replicate();

	//return false if program is shut down
	bool Render();

	void Update();

	void tick();

	int tick_duration;
	Main_map* map;
	SDL_Point* inputs;
	Base_class* local_player;

	int m_tick_rate;

private:
	GameMode();
	GameMode(const GameMode& other) = delete;
	
	int m_tick_delay;

	float m_tick_start;
	float m_tick_end;

	const SDL_Point m_screen_size = { 1100, 600 };
	const SDL_Point m_map_size = { 40, 15 };
};

