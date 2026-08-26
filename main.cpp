#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "Animation.h"

#include <vector>
#include <string>

struct SDL_State
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int logW, logH, width, height;
};

struct Resources
{
    const int AN_PLAYER_IDLE = 0;
    std::vector<Animation> playerAnimations;
    std::vector<SDL_Texture*> textures;
    SDL_Texture* idle_texture;

    SDL_Texture* load_texture(SDL_Renderer* renderer, const std::string& file_path)
    {
        SDL_Texture *texture = IMG_LoadTexture(renderer, file_path.c_str());
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        textures.push_back(texture);
        return texture;
    }

    void load(SDL_State& state)
    {
        playerAnimations.resize(5);
        playerAnimations[AN_PLAYER_IDLE] = Animation(8, 1.6f);
        idle_texture = load_texture(state.renderer, "assets/idle.png");
    }

    void unload()
    {
        for (auto tex: textures)
        {
            SDL_DestroyTexture(tex);
        }
    }
};

void cleanup(const SDL_State &state)
{
    SDL_DestroyWindow(state.window);
    SDL_DestroyRenderer(state.renderer);
    SDL_Quit();
}

bool initialize(SDL_State& state)
{
    state.width = 1600;
    state.height = 900;
    state.logW = 640;
    state.logH = 320;

    bool init_success = true;
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                                 "Error Initializing SDL3", nullptr);
        init_success = false;
    }

    state.window = SDL_CreateWindow("Trench Invasion", state.width, state.height,
                                    SDL_WINDOW_RESIZABLE);
    if (!state.window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                                 "Can't create Window", nullptr);
        init_success = false;
    }

    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                                 "Can't create Renderer", state.window);
        cleanup(state);
        init_success = false;
    }

    SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH,SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return init_success;
}

int main(int argc, char *argv[])
{
    SDL_State state{};
    initialize(state);
    const float sprite_size = 32;


    const bool* keys = SDL_GetKeyboardState(nullptr);
    float playerX = 150;
    float floor = state.logH;

    Resources resources;
    resources.load(state);

    if (!resources.idle_texture)
    {
        SDL_Log("IMG_LoadTexture failed: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                                 "can't create texture", state.window);
    }

    bool running = true;
    uint64_t prev_time = SDL_GetTicks();
    bool flip_horizontal = false;
    while (running)
    {
        uint64_t now_time = SDL_GetTicks();
        float delta_time = static_cast<float>(now_time - prev_time) / 1000.0f;
        SDL_Event event {0};

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                {
                    running = false;
                    break;
                }
                case SDL_EVENT_WINDOW_RESIZED:
                {
                    state.width = event.window.data1;
                    state.height = event.window.data2;
                    break;;
                }
            }
        }

        float move_amount = 0;
        if (keys[SDL_SCANCODE_A])
        {
            move_amount -= 75;
            flip_horizontal = true;
        }
        if (keys[SDL_SCANCODE_D])
        {
            move_amount += 75;
            flip_horizontal = false;
        }

        playerX += move_amount * delta_time;

        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        SDL_FRect src{0, 0, sprite_size, sprite_size};

        SDL_FRect dest{playerX, floor - sprite_size, sprite_size, sprite_size};

        SDL_RenderTextureRotated(state.renderer, resources.idle_texture, &src, &dest, 0, nullptr, flip_horizontal ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);


        SDL_RenderPresent(state.renderer);
        prev_time = now_time;
    }

    resources.unload();
    cleanup(state);
    return 0;
}
