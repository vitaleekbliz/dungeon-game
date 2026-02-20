#pragma once
#include "Header.h"
#include "Base_class.h"
#include "Main_map.h"
#include "GameMode.h"

class Player_class : public Base_class
{
public:
	Player_class() = delete;
	Player_class(int id, int player_id);
	~Player_class();

	static void replicatePlayers();
	static Base_class* replicateID();

	//currently doesnt work on clients cause player id doesnt replicate!!
	static Player_class* getPlayerByID(int id);

	void movePlayer(SDL_Point inputs_vector);

	static void drawPlayers();
	static std::vector<Player_class*>& getAllPlayers();

private:
	static void handleInputs();
	static void handleInputsServer();
	static void handleInputsClient();

	static void replicateMovingPlayers();
	static void replicateMovingPlayersServer();
	static void replicateMovingPlayersClient();

	static void checkForNewPlayersServer();

	void drawPlayer();
	
	int m_player_id;
	static std::vector<Player_class*> m_all_players;

	bool m_is_moving;
	float m_player_speed;

	int current_animation_tick;
	//if FPS is dynamicly changed animation will be slower or faster
	int ticks_for_animation;
	int current_animation;
};

