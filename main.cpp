#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <array>

#include "GameObject.h"

struct SDL_State
{
    SDL_State(): keys(SDL_GetKeyboardState(nullptr)) {};

    const bool* keys;
    SDL_Window *window;
    SDL_Renderer *renderer;
    int logW, logH, width, height;
};

const size_t LAYER_INDEX_LEVEL = 0;
const size_t LAYER_INDEX_CHARACTERS = 1;
const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 32;

struct GameState
{
    std::array<std::vector<GameObject>, 2> layers;
    int player_index;

    GameState()
    {
        player_index = 0;
    }
};

struct Resources
{
    const int AN_PLAYER_IDLE = 0;
    const int AN_PLAYER_RUN = 1;
    std::vector<Animation> playerAnimations;
    std::vector<SDL_Texture*> textures;
    SDL_Texture* idle_texture, *running_texture;
    SDL_Texture* texture_grass, *texture_panel, *texture_ground, *texture_brick;

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
        playerAnimations[AN_PLAYER_RUN] = Animation(4, 0.5f);
        idle_texture = load_texture(state.renderer, "assets/idle.png");
        running_texture = load_texture(state.renderer, "assets/run.png");
        texture_brick = load_texture(state.renderer, "assets/brick.png");
        texture_grass = load_texture(state.renderer, "assets/grass.png");
        texture_panel = load_texture(state.renderer, "assets/panel.png");
        texture_ground = load_texture(state.renderer, "assets/ground.png");
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

void drawObject(const SDL_State& state, GameState& game_state, GameObject& game_object, float delta_time);
void update(const SDL_State& state, GameState& game_state,Resources& resources, GameObject& game_object, float delta_time);
void create_tiles(SDL_State& state, GameState& game_state, Resources& resources);
void check_collision(
    const SDL_State& state,
    GameState& game_state,
    Resources& resources,
    GameObject& A,
    GameObject& B,
    float delta_time
);

int main(int argc, char *argv[])
{
    SDL_State state{};
    Resources resources;

    initialize(state);
    resources.load(state);

    GameState game_state;
    create_tiles(state, game_state, resources);

    if (!resources.idle_texture)
    {
        SDL_Log("IMG_LoadTexture failed: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                                 "can't create texture", state.window);
    }

    bool running = true;
    uint64_t prev_time = SDL_GetTicks();
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
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);
        for (auto& layer: game_state.layers)
        {
            for (GameObject& object: layer)
            {
                update(state, game_state,resources, object, delta_time);
                if (object.currentAnimation != -1)
                    object.animations[object.currentAnimation].step(delta_time);
                drawObject(state, game_state, object, delta_time);
            }
        }



        SDL_RenderPresent(state.renderer);
        prev_time = now_time;
    }

    resources.unload();
    cleanup(state);
    return 0;
}

void drawObject(const SDL_State& state, GameState& game_state, GameObject& game_object, float delta_time)
{
    const float sprite_size = 32;
    float srcX = game_object.currentAnimation != -1
        ? game_object.animations[game_object.currentAnimation].current_frame() * sprite_size : 0.0f;
    SDL_FRect src{srcX, 0, sprite_size, sprite_size};
    SDL_FRect dest{game_object.position.x, game_object.position.y, sprite_size, sprite_size};
    SDL_FlipMode flip_mode = game_object.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderTextureRotated(state.renderer, game_object.texture, &src, &dest, 0, nullptr, flip_mode);
}

void update(const SDL_State& state, GameState& game_state, Resources& resources, GameObject& game_object, float delta_time)
{
    if (game_object.has_gravity)
    {
        game_object.velocity += glm::vec2(0, 500) * delta_time;
    }

    if (game_object.type == ObjectType::player)
    {
        float current_direction = 0;
        if (state.keys[SDL_SCANCODE_A])
        {
            current_direction--;
        }
        if (state.keys[SDL_SCANCODE_D])
        {
            current_direction++;
        }
        if (current_direction)
        {
            game_object.direction = current_direction;
        }

        switch (game_object.data.player.state)
        {
            case PlayerState::idle:
            {
                if (current_direction)
                {
                    game_object.data.player.state = PlayerState::running;
                    game_object.texture = resources.running_texture;
                    game_object.currentAnimation = resources.AN_PLAYER_RUN;
                } else
                {
                    if (game_object.velocity.x)
                    {
                        const float factor = game_object.velocity.x > 0 ? -1.5f : 1.5f;
                        float amount = factor * game_object.acceleration.x * delta_time;
                        if (std::abs(game_object.velocity.x) < std::abs(amount))
                        {
                            game_object.velocity.x = 0;
                        } else
                        {
                            game_object.velocity.x += amount;
                        }
                    }
                }
                break;
            }

            case PlayerState::running:
            {
                if (!current_direction)
                {
                    game_object.data.player.state = PlayerState::idle;
                    game_object.texture = resources.idle_texture;
                    game_object.currentAnimation = resources.AN_PLAYER_IDLE;
                }
                break;
            }
        }

        game_object.velocity += current_direction * game_object.acceleration * delta_time;
        if (std::abs(game_object.velocity.x) > game_object.max_speed_x)
        {
            game_object.velocity.x = current_direction * game_object.max_speed_x;
        }
    }
    game_object.position += game_object.velocity * delta_time;
     for(auto& layer: game_state.layers)
    {
        for(GameObject& objB: layer)
        {
            if (&game_object != &objB)
            {
                check_collision(
                    state,
                    game_state,
                    resources,
                    game_object,
                    objB,
                    delta_time
                );
            }
        }
    }
}

void collision_response(
    const SDL_State& state,
    GameState& game_state,
    GameObject& game_objectA,
    GameObject& game_objectB,
    Resources& resources,
    SDL_FRect& rectA,
    SDL_FRect& rectB,
    SDL_FRect& rectC,
    float delta_time
)
{
    if (game_objectA.type == ObjectType::player)
    {
        switch(game_objectB.type)
        {
            case ObjectType::level:
            {
                if (rectC.w < rectC.h)
                {
                    if (game_objectA.velocity.x > 0)
                    {
                        game_objectA.position.x -= rectC.w;
                    }
                    else if (game_objectA.velocity.x < 0)
                    {
                        game_objectA.position.x += rectC.w;
                    }
                    game_objectA.velocity.x = 0;
                }
                else
                {
                    if (game_objectA.velocity.y > 0)
                    {
                        game_objectA.position.y -= rectC.h;
                    }
                    else if (game_objectA.velocity.y < 0)
                    {
                        game_objectA.position.y += rectC.h;
                    }
                    game_objectA.velocity.y = 0;
                }
            }
            break;
        }
    }
}

void check_collision(
    const SDL_State& state,
    GameState& game_state,
    Resources& resources,
    GameObject& A,
    GameObject& B,
    float delta_time
)
{
    SDL_FRect rectA {
        .x = A.position.x + A.collider.x,
        .y = A.position.y + A.collider.y,
        .w = A.collider.w,
        .h = A.collider.h,
    };

    SDL_FRect rectB {
        .x = B.position.x + B.collider.x,
        .y = B.position.y + B.collider.y,
        .w = B.collider.w,
        .h = B.collider.h,
    };

    SDL_FRect rectC {0};

    if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &rectC))
    {
        collision_response(
            state,
            game_state,
            A,
            B,
            resources,
            rectA,
            rectB,
            rectC,
            delta_time
        );
    }
}

void create_tiles(SDL_State& state, GameState& game_state, Resources& resources)
{
    /*
      O -> empty tile
      1 -> Ground
      2 -> Panel
      3 -> Enemy
      4 -> Player
      5 -> Grass
      6 -> Brick
     */

    short map[MAP_ROWS][MAP_COLS] = {
        {0,0,4,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,2,0,0,0,0,0,0,0},
        {0,2,2,0,0,0,0,0,0,2},
        {1,1,1,1,1,1,1,1,1,1},
    };

    const auto create_object = [&state] (int r, int c, SDL_Texture* tex, ObjectType type)
    {
        GameObject game_object;
        game_object.type = type;
        game_object.texture = tex;
        game_object.position = glm::vec2(c * TILE_SIZE, state.logH - (MAP_ROWS - r) * TILE_SIZE);
        game_object.collider = {
            .x = 0,
            .y = 0,
            .w = TILE_SIZE,
            .h = TILE_SIZE
        };

        return game_object;
    };

    for (int r = 0; r < MAP_ROWS; r++)
    {
        for (int c = 0; c < MAP_COLS; c++)
        {
            switch (map[r][c])
            {
                case 1:
                {
                    GameObject ground = create_object(r, c, resources.texture_ground, ObjectType::level);
                    game_state.layers[LAYER_INDEX_LEVEL].push_back(ground);
                    break;
                }

                case 2:
                {
                    GameObject panel = create_object(r, c, resources.texture_panel, ObjectType::level);
                    game_state.layers[LAYER_INDEX_LEVEL].push_back(panel);
                    break;
                }

                case 3:
                {
                    break;
                }

                case 4:
                {
                    GameObject player = create_object(r, c, resources.idle_texture, ObjectType::player);

                    player.has_gravity = true;
                    player.data.player = PlayerData();
                    player.animations = resources.playerAnimations;
                    player.currentAnimation = resources.AN_PLAYER_IDLE;
                    player.acceleration = glm::vec2(300, 0);
                    player.max_speed_x = 100;
                    player.collider = {
                        .x = 11,
                        .y = 6,
                        .w = 10,
                        .h = 26
                    };
                    game_state.layers[LAYER_INDEX_CHARACTERS].push_back(player);
                    break;
                }

                case 5:
                {
                    GameObject grass = create_object(r, c, resources.texture_grass, ObjectType::level);
                    game_state.layers[LAYER_INDEX_LEVEL].push_back(grass);
                    break;
                }

                case 6:
                {
                    GameObject brick = create_object(r, c, resources.texture_brick, ObjectType::level);
                    game_state.layers[LAYER_INDEX_LEVEL].push_back(brick);
                    break;
                }
            }
        }
    }
}
