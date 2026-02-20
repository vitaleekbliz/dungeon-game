#pragma once
#include <iostream>
#include <SDL.h>
#include <vector>
#include <Windows.h>

class main_menu
{
public:
	main_menu() : resolution(resolutions.back()) {};
	SDL_Point Run();
private:
	void Select_resolution();

	std::vector<SDL_Point> resolutions = { {1920, 1080},{1280, 720}, {854, 480} };
	SDL_Point resolution;
	std::vector<std::string> buttons {"Connect", "Resolution"};
};

