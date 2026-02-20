#include "Header.h"
#include "GameMode.h"

int main() {
    
    GameMode::Get().InitGame();

    while (1) {

        if (!GameMode::Get().Replicate())
            continue;

        GameMode::Get().Update();

        if (!GameMode::Get().Render())
            return 0;

        GameMode::Get().tick();
    }

    GameMode::Get().ClearGame();

}