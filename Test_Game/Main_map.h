#pragma once
#include "Header.h"
#include "Engine.h"
#include "Network.h"
#include <random>

class Main_map
{
public:
	Main_map();
	Main_map(int x, int y);
	~Main_map() {};

	std::vector<std::vector<int>>& Get_map();
	bool isCellWalkable(int x, int y);

	void Replicate_map();

	void makePassage();
	void DrawMap();

private:
	void Generate_map(int x, int y);
	std::vector<SDL_Point> Get_available_directions(SDL_Point current_pos, std::vector<std::vector<int>>& visisted_cells);

	void Replicate_map_size();
	void Replicate_map_content();

	int x;
	int y;
	std::vector < std::vector<int>> m_map;
	bool map_replicated_on_client;
};

