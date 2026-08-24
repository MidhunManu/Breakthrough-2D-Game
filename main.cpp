#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

int WIDTH = 800;
int HEIGHT = 600;

struct SDL_State
{
    SDL_Window *window;
    SDL_Renderer *renderer;
};

void cleanup(const SDL_State &state)
{
    SDL_DestroyWindow(state.window);
    SDL_DestroyRenderer(state.renderer);
    SDL_Quit();
}

void cleanup_texture(SDL_Texture *texture) { SDL_DestroyTexture(texture); }

int main(int argc, char *argv[])
{
    SDL_State state{};

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                                 "Error Initializing SDL3", nullptr);
        return 1;
    }

    state.window = SDL_CreateWindow("Trench Invasion", WIDTH, HEIGHT,
                                    SDL_WINDOW_RESIZABLE);
    SDL_Event event;
    if (!state.window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                                 "Can't create Window", nullptr);
    }

    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                                 "Can't create Renderer", state.window);
        cleanup(state);
    }

    int logW = 640;
    int logH = 320;
    SDL_SetRenderLogicalPresentation(state.renderer, logW, logH,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

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
                    WIDTH = event.window.data1;
                    HEIGHT = event.window.data2;
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
