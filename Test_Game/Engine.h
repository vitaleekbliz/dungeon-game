#pragma once
#include "Header.h"

struct Position {
	float x; float y;
};

enum TEXTURES {
	WALL,
	GRASS,
	MUD,
	PLAYER_STAYING,
	PLAYER_MOVING_1,
	PLAYER_MOVING_2
};

class Engine
{
public:
	void Init(int window_w, int window_h);
	void Clear();
	static Engine& Get();

	//before drawing objects, need to calculate camera position ==>> drawing start point
	bool Draw_object(Position object, TEXTURES texture, int rotation = 0);
	SDL_Point convert_global_pos_to_screen(Position object);
	//once per tick
	void calculateDrawingStartPoint(Position camera);
	
	bool HandleEvents(SDL_Point* inputs);
	void ClearRenderer();
	void Render();

private:
	Engine();
	Engine(const Engine& other) = delete;

	SDL_Rect getTextureRect(TEXTURES texture);

	const int m_cell_size = 64;

	int m_window_width;
	int m_window_height;

	float m_screen_start_x;
	float m_screen_start_y;

	SDL_Window* m_window;
	SDL_Renderer* m_renderer;
	SDL_Surface* m_imageSurface;
	SDL_Texture* m_texture;
};

