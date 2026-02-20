#include "main_menu.h"

SDL_Point main_menu::Run()
{
	int selected_button = 0;
	int previosly_selected_button = 100000;
	while (true) {
		if (previosly_selected_button != selected_button) {
			system("cls");
			std::cout << "     VI LIK BLIZ presents     \n   Greates Game Of All Times  \n           DUNGEON           \n__________Main Menu___________\n\n";
			for (int i = 0; i < buttons.size(); i++) {
				std::cout << buttons[i] << (selected_button == i ? "<==\n" : "\n");
			}
			previosly_selected_button = selected_button;

		}
		
		if (GetKeyState(VK_RETURN)<0) {
			while (GetKeyState(VK_RETURN) < 0);

			if (selected_button == 0) {
				return resolution;
			}
			else if (selected_button == 1) {
				Select_resolution();
				previosly_selected_button++;
			}

			
		}
		if (GetKeyState(VK_UP)<0) {
			while (GetKeyState(VK_UP) < 0);

			selected_button = (--selected_button < 0 ? buttons.size() - 1 : selected_button);
		}
		if (GetKeyState(VK_DOWN) < 0) {
			while (GetKeyState(VK_DOWN) < 0);

			selected_button = (++selected_button >= buttons.size() ? 0 : selected_button);
		}
		Sleep(10);
	}
	return resolution;
}

void main_menu::Select_resolution()
{
	int selected_button = 0;
	int previosly_selected_button = 100000;
	while (true) {
		if (previosly_selected_button != selected_button) {
			system("cls");
			std::cout << "==========Select Resolution===========\n\n";
			for (int i = 0; i < resolutions.size(); i++) {
				std::cout << resolutions[i].x << 'x' << resolutions[i].y << (selected_button == i ? "<==\n" : "\n");
			}
			previosly_selected_button = selected_button;
		}
		
		if (GetKeyState(VK_RETURN) < 0) {
			while (GetKeyState(VK_RETURN) < 0);

			resolution = resolutions[selected_button];
			return;
		}
		if (GetKeyState(VK_UP) < 0) {
			while (GetKeyState(VK_UP) < 0);

			selected_button = (--selected_button < 0 ? resolutions.size() - 1 : selected_button);
		}
		if (GetKeyState(VK_DOWN) < 0) {
			while (GetKeyState(VK_DOWN) < 0);

			selected_button = (++selected_button >= resolutions.size() ? 0 : selected_button);
		}
		Sleep(10);
	}
}
