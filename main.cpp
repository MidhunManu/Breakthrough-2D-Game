#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

struct SDL_State
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int logW, logH, width, height;
};

void cleanup(const SDL_State &state)
{
    SDL_DestroyWindow(state.window);
    SDL_DestroyRenderer(state.renderer);
    SDL_Quit();
}

void cleanup_texture(SDL_Texture *texture) { SDL_DestroyTexture(texture); }

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
    SDL_Texture *idleTexture =
        IMG_LoadTexture(state.renderer, "assets/idle.png");
    SDL_SetTextureScaleMode(idleTexture, SDL_SCALEMODE_NEAREST);

    const bool* keys = SDL_GetKeyboardState(nullptr);
    float playerX = 150;
    float floor = state.logH;

    if (!idleTexture)
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
        float delta_time = (now_time - prev_time) / 1000.0f;
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

        SDL_RenderTextureRotated(state.renderer, idleTexture, &src, &dest, 0, nullptr, flip_horizontal ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);


        SDL_RenderPresent(state.renderer);
        prev_time = now_time;
    }

    cleanup_texture(idleTexture);
    cleanup(state);
    return 0;
}
