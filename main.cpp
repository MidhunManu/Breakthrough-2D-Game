#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

// int WIDTH = 800;
// int HEIGHT = 600;

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
    SDL_Event event;
    SDL_Texture *idleTexture =
        IMG_LoadTexture(state.renderer, "assets/idle.png");
    SDL_SetTextureScaleMode(idleTexture, SDL_SCALEMODE_NEAREST);
    if (!idleTexture)
    {
        SDL_Log("IMG_LoadTexture failed: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                                 "can't create texture", state.window);
    }

    bool running = true;

    while (running)
    {
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

        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        SDL_FRect src{0, 0, 32, 32};

        SDL_FRect dest{0, 0, 32, 32};

        SDL_RenderTexture(state.renderer, idleTexture, &src, &dest);

        SDL_RenderPresent(state.renderer);
    }

    cleanup_texture(idleTexture);
    cleanup(state);
    return 0;
}
