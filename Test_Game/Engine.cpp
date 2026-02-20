#include "Engine.h"

Engine::Engine()
{
    m_screen_start_x = 0;
    m_screen_start_y = 0;
}


void Engine::Init(int window_w, int window_h) 
{
    m_window_height = window_h;
    m_window_width = window_w;

    // Step 1: Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        throw 1;
    }

    // Step 2: Create a window and renderer
    m_window = SDL_CreateWindow("SDL Texture Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_window_width, m_window_height, SDL_WINDOW_SHOWN);
    if (!m_window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        throw 1;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw 1;
    }

    // Step 3: Load a texture from an image file
    m_imageSurface = IMG_Load("textures/All_textures.png");
    if (!m_imageSurface) {
        std::cerr << "Unable to load image! SDL_Image Error: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw 1;
    }

    m_texture = SDL_CreateTextureFromSurface(m_renderer, m_imageSurface);
    SDL_FreeSurface(m_imageSurface);
    if (!m_texture) {
        std::cerr << "Unable to create texture! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw 1;
    }

}

void Engine::Clear()
{
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();

}

SDL_Rect Engine::getTextureRect(TEXTURES texture)
{
    switch (texture)
    {
    case WALL:
        return { m_cell_size * 0, 0, m_cell_size, m_cell_size };
        break;

    case GRASS:
        return { m_cell_size * 1, 0, m_cell_size, m_cell_size };
        break;

    case MUD:
        return { m_cell_size * 2, 0, m_cell_size, m_cell_size };
        break;

    //case PLAYER_STAYING:
    //    return { m_cell_size * 3, 0, m_cell_size, m_cell_size };
    //    break;

    //case PLAYER_MOVING_1:
    //    return { m_cell_size * 4, 0, m_cell_size, m_cell_size };
    //    break;

    //case PLAYER_MOVING_2:
    //    return { m_cell_size * 5, 0, m_cell_size, m_cell_size };
    //    break;

    case PLAYER_STAYING:
        return { m_cell_size * 3 + m_cell_size / 4, m_cell_size / 4, m_cell_size / 2, m_cell_size / 2 };
        break;

    case PLAYER_MOVING_1:
        return { m_cell_size * 4 + m_cell_size / 4, m_cell_size / 4, m_cell_size / 2, m_cell_size / 2 };
        break;

    case PLAYER_MOVING_2:
        return { m_cell_size * 5 + m_cell_size / 4, m_cell_size / 4, m_cell_size / 2, m_cell_size / 2 };
        break;

    default:
        return { 0, 0, 0, 0 };
    }
}

Engine& Engine::Get()
{
    static Engine instance;
    return instance;
}

bool Engine::Draw_object(Position object, TEXTURES texture, int rotation)
{
    SDL_Point p = convert_global_pos_to_screen(object);

    //if object is within the screen
    if (p.x > -m_cell_size && p.y > -m_cell_size && p.x < m_window_width && p.y < m_window_height) {

        SDL_Rect screen = { p.x, p.y, m_cell_size, m_cell_size };
        SDL_Rect txt = getTextureRect(texture);

        if (rotation > 0 && rotation <= 359) {
            const SDL_Point rotation_center = { m_cell_size / 2, m_cell_size / 2};
            SDL_RenderCopyEx(m_renderer, m_texture, &txt, &screen, rotation, &rotation_center, SDL_FLIP_NONE);
        }
        else
            SDL_RenderCopy(m_renderer, m_texture, &txt, &screen);

        return true;
    }
    return false;
}

void Engine::calculateDrawingStartPoint(Position camera)
{
    int m_cell_count_w = m_window_width / m_cell_size;
    int m_cell_count_h = m_window_height / m_cell_size;

    m_screen_start_x = camera.x - (float)m_cell_count_w / 2;
    m_screen_start_y = camera.y + (float)m_cell_count_h / 2;
}

SDL_Point Engine::convert_global_pos_to_screen(Position object)
{
    SDL_Point ret;

    float dif_x = object.x - m_screen_start_x;
    float dif_y = m_screen_start_y - object.y;

    //TODO hard code all the textures are 64 pixels if object has custom size should change the rectangle
    ret.x = m_cell_size * dif_x - m_cell_size / 2;
    ret.y = m_cell_size * dif_y + m_cell_size / 2;

    return ret;
}

bool Engine::HandleEvents(SDL_Point* inputs)
{
    SDL_Event event;

    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            //Stop();
            return false;
        }
        else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
                if(inputs->y == 0)
                    inputs->y = 1;
                break;
            case SDLK_DOWN:
                if (inputs->y == 0)
                    inputs->y = -1;
                break;
            case SDLK_LEFT:
                if (inputs->x == 0)
                    inputs->x = -1;
                break;
            case SDLK_RIGHT:
                if (inputs->x == 0)
                    inputs->x = 1;
                break;
            default:
                break;
            }
        }
        else if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
                if(inputs->y == 1)
                    inputs->y = 0;
                break;
            case SDLK_DOWN:
                if(inputs->y == -1)
                    inputs->y = 0;
                break;
            case SDLK_LEFT:
                if(inputs->x == -1)
                    inputs->x = 0;
                break;
            case SDLK_RIGHT:
                if(inputs->x == 1)
                    inputs->x = 0;
                break;
            default:
                break;
            }
        }
    }
    return true;
}

void Engine::ClearRenderer()
{
    SDL_RenderClear(m_renderer);
}
void Engine::Render()
{
    SDL_RenderPresent(m_renderer);
}

