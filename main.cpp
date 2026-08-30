#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <array>
#include <expected>
#include <format>

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
const float MAX_DELTA_TIME = 1.0f / 30.0f;
bool debug_mode = false;

struct GameState
{
    std::array<std::vector<GameObject>, 2> layers;
    std::vector<GameObject> background_tiles;
    std::vector<GameObject> foreground_tiles;
    std::vector<GameObject> bullets;
    int player_index;
    SDL_FRect mapViewport;
    float bg2_scroll, bg3_scroll, bg4_scroll;

    GameState(const SDL_State& state)
    {
        player_index = -1;
        mapViewport = SDL_FRect {
            .x = 0,
            .y = 0,
            .w = static_cast<float>(state.logW),
            .h = static_cast<float>(state.logH),
        };
        bg2_scroll = bg3_scroll = bg4_scroll = 0.0f;
    }

    GameObject& player()
    {
        return layers[LAYER_INDEX_CHARACTERS][player_index];
    }
};

struct Resources
{
    const int AN_PLAYER_IDLE = 0;
    const int AN_PLAYER_RUN = 1;
    const int AN_PLAYER_SLIDE = 2;
    const int AN_PLAYER_SHOOT = 3;
    const int AN_PLAYER_RUN_SHOOT = 4;
    const int AN_PLAYER_SLIDE_SHOOT = 5;
    const int AN_BULLET_MOVING = 0;
    const int AN_BULLET_HIT = 1;
    std::vector<Animation> playerAnimations;
    std::vector<Animation> bullet_animations;
    std::vector<SDL_Texture*> textures;
    SDL_Texture* idle_texture, *running_texture;
    SDL_Texture* texture_grass, *texture_panel, *texture_ground, *texture_brick,
    *texture_slide, *texture_bg1, *texture_bg2, *texture_bg3, *texture_bg4,
    *texture_bullet, *texture_bullet_hit, *texture_shoot, *texture_run_shoot,
    *texture_slide_shoot;

    SDL_Texture* load_texture(SDL_Renderer* renderer, const std::string& file_path)
    {
        SDL_Texture *texture = IMG_LoadTexture(renderer, file_path.c_str());
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        textures.push_back(texture);
        return texture;
    }

    void load(SDL_State& state)
    {
        playerAnimations.resize(6);
        bullet_animations.resize(2);
        playerAnimations[AN_PLAYER_IDLE] = Animation(8, 1.6f);
        playerAnimations[AN_PLAYER_RUN] = Animation(4, 0.5f);
        playerAnimations[AN_PLAYER_SLIDE] = Animation(1, 1.0f);
        playerAnimations[AN_PLAYER_SHOOT] = Animation(4, 0.5f);
        playerAnimations[AN_PLAYER_RUN_SHOOT] = Animation(4, 0.5f);
        playerAnimations[AN_PLAYER_SLIDE_SHOOT] = Animation(4, 0.5f);
        bullet_animations[AN_BULLET_MOVING] = Animation(4, 0.05f);
        bullet_animations[AN_BULLET_HIT] = Animation(4, 0.15f);        

        idle_texture = load_texture(state.renderer, "assets/idle.png");
        running_texture = load_texture(state.renderer, "assets/run.png");
        texture_brick = load_texture(state.renderer, "assets/brick.png");
        texture_grass = load_texture(state.renderer, "assets/grass.png");
        texture_panel = load_texture(state.renderer, "assets/panel.png");
        texture_ground = load_texture(state.renderer, "assets/ground.png");
        texture_slide = load_texture(state.renderer, "assets/slide.png");
        texture_bg1 = load_texture(state.renderer, "assets/bg_layer1.png");
        texture_bg2 = load_texture(state.renderer, "assets/bg_layer2.png");
        texture_bg3 = load_texture(state.renderer, "assets/bg_layer3.png");
        texture_bg4 = load_texture(state.renderer, "assets/bg_layer4.png");
        texture_bullet = load_texture(state.renderer, "assets/bullet.png");
        texture_bullet_hit = load_texture(state.renderer, "assets/bullet_hit.png");
        texture_shoot = load_texture(state.renderer, "assets/shoot.png");
        texture_slide_shoot = load_texture(state.renderer, "assets/slide_shoot.png");
        texture_run_shoot = load_texture(state.renderer, "assets/shoot_run.png");
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

    SDL_SetRenderVSync(state.renderer, 1);

    SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH,SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return init_success;
}

void drawObject(const SDL_State& state, GameState& game_state, GameObject& game_object, float width, float height, float delta_time);
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
void handle_key_input(
    const SDL_State& state,
    GameState& game_state,
    GameObject& game_object,
    SDL_Scancode key,
    bool key_down
);
void draw_paralax_bg(
    SDL_Renderer* renderer,
    SDL_Texture* texture,
    float x_velocity,
    float& scroll_pos,
    float scroll_factor,
    float delta_time
);

int main(int argc, char *argv[])
{
    SDL_State state{};
    Resources resources;

    initialize(state);
    resources.load(state);

    GameState game_state(state);
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
        if (delta_time > MAX_DELTA_TIME)
        {
            delta_time = MAX_DELTA_TIME;
        }
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
                    break;
                }
                case SDL_EVENT_KEY_DOWN:
                {
                    handle_key_input(state, game_state, game_state.player(), event.key.scancode, true);
                    if (event.key.scancode == SDL_SCANCODE_F1)
                    {
                        debug_mode = !debug_mode;
                    }
                    break;
                }
                case SDL_EVENT_KEY_UP:
                {
                    handle_key_input(state, game_state, game_state.player(), event.key.scancode, false);
                    break;
                }
            }
        }
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);
        SDL_RenderTexture(state.renderer, resources.texture_bg1, nullptr, nullptr);
        draw_paralax_bg(
            state.renderer,
            resources.texture_bg4,
            game_state.player().velocity.x,
            game_state.bg4_scroll,
            0.075f,
            delta_time
        );
        draw_paralax_bg(
            state.renderer,
            resources.texture_bg3,
            game_state.player().velocity.x,
            game_state.bg3_scroll,
            0.150f,
            delta_time
        );
        draw_paralax_bg(
            state.renderer,
            resources.texture_bg2,
            game_state.player().velocity.x,
            game_state.bg2_scroll,
            0.3f,
            delta_time
        );

        for(GameObject& obj: game_state.background_tiles)
        {
            SDL_FRect dest
            {
                .x = obj.position.x - game_state.mapViewport.x,
                .y = obj.position.y,
                .w = static_cast<float>(obj.texture->w),
                .h = static_cast<float>(obj.texture->h)
            };

            SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dest);
        }

        for (auto& layer: game_state.layers)
        {
            for (GameObject& object: layer)
            {
                update(state, game_state,resources, object, delta_time);
                if (object.currentAnimation != -1)
                    object.animations[object.currentAnimation].step(delta_time);
                drawObject(state, game_state, object, TILE_SIZE, TILE_SIZE, delta_time);
            }
        }

        for (GameObject& bullet: game_state.bullets)
        {
            update(state, game_state,resources, bullet, delta_time);
            if (bullet.currentAnimation != -1)
                bullet.animations[bullet.currentAnimation].step(delta_time);
            drawObject(state, game_state, bullet, bullet.collider.w, bullet.collider.h, delta_time);
        }

        for(GameObject& obj: game_state.foreground_tiles)
        {
            SDL_FRect dest
            {
                .x = obj.position.x - game_state.mapViewport.x,
                .y = obj.position.y,
                .w = static_cast<float>(obj.texture->w),
                .h = static_cast<float>(obj.texture->h)
            };

            SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dest);
        }

        game_state.mapViewport.x = (game_state.player().position.x + TILE_SIZE / 2) - game_state.mapViewport.w / 2;

        #ifdef DEBUG

        SDL_SetRenderDrawColor(state.renderer, 57, 255, 20, 255);
        SDL_RenderDebugText(
            state.renderer, 5, 5,
            std::format("State: {}, B_Size: {}, Grounded: {}",
                        static_cast<int>(game_state.player().data.player.state),
                        game_state.bullets.size(), game_state.player().grounded)
                .c_str());

        #endif

        SDL_RenderPresent(state.renderer);
        prev_time = now_time;
    }

    resources.unload();
    cleanup(state);
    return 0;
}

void drawObject(const SDL_State& state, GameState& game_state, GameObject& game_object, float width, float height, float delta_time)
{
    float srcX = game_object.currentAnimation != -1
        ? game_object.animations[game_object.currentAnimation].current_frame() * width : 0.0f;
    SDL_FRect src{srcX, 0, width, height};
    SDL_FRect dest{game_object.position.x - game_state.mapViewport.x, game_object.position.y, width, height};
    SDL_FlipMode flip_mode = game_object.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderTextureRotated(state.renderer, game_object.texture, &src, &dest, 0, nullptr, flip_mode);

    #ifdef DEBUG
    if (debug_mode)
    {
        SDL_FRect rectA {
            game_object.position.x + game_object.collider.x - game_state.mapViewport.x,
            game_object.position.y + game_object.collider.y,
            game_object.collider.w,
            game_object.collider.h
        };
        SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 150);
        SDL_RenderFillRect(state.renderer, &rectA);
        SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);
    }
    #endif
}

void update(const SDL_State& state, GameState& game_state, Resources& resources, GameObject& game_object, float delta_time)
{
    float current_direction = 0;
    if (game_object.has_gravity && !game_object.grounded)
    {
        game_object.velocity += glm::vec2(0, 500) * delta_time;
    }

    if (game_object.type == ObjectType::player)
    {
        if (state.keys[SDL_SCANCODE_A])
        {
            current_direction--;
        }
        if (state.keys[SDL_SCANCODE_D])
        {
            current_direction++;
        }

        Timer& player_gun_timer = game_object.data.player.gun_timer;
        player_gun_timer.step(delta_time);
        const auto handle_shooting = [&state, &game_state, &resources, &game_object, &player_gun_timer]
        (
            SDL_Texture* texture,
            SDL_Texture* shooting_texture,
            int animation_index,
            int shooting_animation_index
        )
        {
            if (state.keys[SDL_SCANCODE_J])
            {
                game_object.texture = shooting_texture;
                game_object.currentAnimation = shooting_animation_index;
                if (player_gun_timer.is_time_out())
                {
                    player_gun_timer.reset_timer();

                    GameObject bullet;
                    bullet.data.bullet = BulletData();
                    bullet.type = ObjectType::bullet;
                    bullet.direction = game_state.player().direction;
                    bullet.currentAnimation = resources.AN_BULLET_MOVING;
                    bullet.texture = resources.texture_bullet;
                    bullet.collider = SDL_FRect{
                        .x = 0,
                        .y = 0,
                        .w = static_cast<float>(resources.texture_bullet->h),
                        .h = static_cast<float>(resources.texture_bullet->h)};
                    bullet.velocity =
                        glm::vec2((game_state.player().velocity.x + 600.0f) *
                                    game_state.player().direction,
                                0);
                    bullet.animations = resources.bullet_animations;
                    const float left = 0.4f;
                    const float right = 24.0f;
                    const float t = (game_object.direction + 1) / 2.0f;
                    float lerp = left + right * t;
                    bullet.position =
                        glm::vec2(game_object.position.x + lerp,
                                game_object.position.y + TILE_SIZE / 2);
                    bullet.max_speed_x = 1000.0f;
                    bool found_inactive = false;
                    for(int i = 0; i < game_state.bullets.size() && !found_inactive; i++)
                    {
                        if (game_state.bullets[i].data.bullet.state == BulletState::in_active)
                        {
                            found_inactive = true;
                            game_state.bullets[i] = bullet;
                        }
                    }
                    if (!found_inactive)
                    {
                        game_state.bullets.push_back(bullet);
                    }
                }
            }
            else
            {
                game_object.texture = texture;
                game_object.currentAnimation = animation_index;
            }
        };

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

                game_object.texture = resources.idle_texture;
                game_object.currentAnimation = resources.AN_PLAYER_IDLE;
                handle_shooting(resources.idle_texture, resources.texture_shoot, resources.AN_PLAYER_IDLE, resources.AN_PLAYER_SHOOT);
                break;
            }

            case PlayerState::running:
            {
                if (!current_direction)
                {
                    game_object.data.player.state = PlayerState::idle;
                }

                if (game_object.velocity.x * game_object.direction < 0 && game_object.grounded)
                {
                    game_object.texture = resources.texture_slide;
                    game_object.currentAnimation = resources.AN_PLAYER_SLIDE;

                    handle_shooting(
                        resources.texture_slide,
                        resources.texture_slide_shoot,
                        resources.AN_PLAYER_SLIDE,
                        resources.AN_PLAYER_SLIDE_SHOOT
                    );
                }
                else
                {
                    game_object.texture = resources.running_texture;
                    game_object.currentAnimation = resources.AN_PLAYER_RUN;

                    handle_shooting(
                        resources.running_texture,
                        resources.texture_run_shoot,
                        resources.AN_PLAYER_RUN,
                        resources.AN_PLAYER_RUN_SHOOT
                    );
                }
                break;
            }

            case PlayerState::jumping:
            {
                game_object.texture = resources.running_texture;
                game_object.currentAnimation = resources.AN_PLAYER_RUN;
                handle_shooting(
                    resources.running_texture,
                    resources.texture_run_shoot,
                    resources.AN_PLAYER_RUN,
                    resources.AN_PLAYER_RUN_SHOOT
                );
                break;
            }
        }
    }
    else if (game_object.type == ObjectType::bullet)
    {
        if (game_object.position.x - game_state.mapViewport.x < 0 ||
            game_object.position.x - game_state.mapViewport.x > state.logW ||
            game_object.position.y - game_state.mapViewport.y < 0 ||
            game_object.position.y - game_state.mapViewport.y > state.logH
        )
        {
            game_object.data.bullet.state = BulletState::in_active;
        }
    }

    if (current_direction)
    {
        game_object.direction = current_direction;
    }

    game_object.velocity += current_direction * game_object.acceleration * delta_time;
    if (std::abs(game_object.velocity.x) > game_object.max_speed_x)
    {
        game_object.velocity.x = current_direction * game_object.max_speed_x;
    }

    game_object.position += game_object.velocity * delta_time;
    bool ground_found = false;
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

                if (objB.type == ObjectType::level)
                {
                    SDL_FRect sensor {
                        .x = game_object.position.x + game_object.collider.x,
                        .y = game_object.position.y + game_object.collider.y + game_object.collider.h,
                        .w = game_object.collider.w,
                        .h = 1
                    };

                    SDL_FRect rectB {
                        .x = objB.position.x + objB.collider.x,
                        .y = objB.position.y + objB.collider.y,
                        .w = objB.collider.w,
                        .h = objB.collider.h,
                    };

                    SDL_FRect rectC {0};

                    if (SDL_GetRectIntersectionFloat(&sensor, &rectB, &rectC))
                    {
                        ground_found = true;
                    }
                }
            }
        }
    }
    if (game_object.grounded != ground_found)
    {
        game_object.grounded = ground_found;
        if (game_object.type == ObjectType::player)
        {
            game_object.data.player.state = PlayerState::running;
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
    const auto generic_response = [&]()
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
    };

    // const auto generic_response = [&]()
    // {
    //     bool horizontal_hit = game_objectA.type == ObjectType::bullet || rectC.w < rectC.h;

    //     if (horizontal_hit)
    //     {
    //         if (game_objectA.velocity.x > 0)
    //         {
    //             game_objectA.position.x -= rectC.w;
    //         }
    //         else if (game_objectA.velocity.x < 0)
    //         {
    //             game_objectA.position.x += rectC.w;
    //         }
    //         game_objectA.velocity.x = 0;
    //     }
    //     else
    //     {
    //         if (game_objectA.velocity.y > 0)
    //         {
    //             game_objectA.position.y -= rectC.h;
    //         }
    //         else if (game_objectA.velocity.y < 0)
    //         {
    //             game_objectA.position.y += rectC.h;
    //         }
    //         game_objectA.velocity.y = 0;
    //     }
    // };

    if (game_objectA.type == ObjectType::player)
    {
        switch(game_objectB.type)
        {
            case ObjectType::level:
            {
                generic_response();
                break;
            }
        }
    }
    else if (game_objectA.type == ObjectType::bullet)
    {
        switch (game_objectB.type)
        {
        case ObjectType::level:
        {
            switch (game_objectA.data.bullet.state)
            {
            case BulletState::moving:
            {
                if (game_objectA.velocity.x > 0)
                    game_objectA.position.x -= rectC.w;
                else if (game_objectA.velocity.x < 0)
                    game_objectA.position.x += rectC.w;
                game_objectA.velocity.x = 0;
                break;
            }
            }
            break;
        }
        default:
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
        {0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0},
        {0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    };

    short foreground[MAP_ROWS][MAP_COLS] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {5,5,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };

    short background[MAP_ROWS][MAP_COLS] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,6,6,6,0,0},
        {0,0,0,0,0,6,6,0,0,0,0,0,0,0,6,6,6,0,0,0,0,0,0,6,6,0,0,0,0,0,6,6,6,0,0,0,0,0,6,6,0,0,0,0,0,6,6,6,0,0},
        {0,0,0,0,0,6,6,0,0,0,0,0,0,0,6,6,6,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,6,6,6,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };

    const auto load_map = [&state, &game_state, &resources] (short layer[MAP_ROWS][MAP_COLS])
    {


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
                switch (layer[r][c])
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
                        game_state.player_index = game_state.layers[LAYER_INDEX_CHARACTERS].size() - 1;
                        break;
                    }

                    case 5:
                    {
                        GameObject grass = create_object(r, c, resources.texture_grass, ObjectType::level);
                        game_state.foreground_tiles.push_back(grass);
                        break;
                    }

                    case 6:
                    {
                        GameObject brick = create_object(r, c, resources.texture_brick, ObjectType::level);
                        game_state.background_tiles.push_back(brick);
                        break;
                    }
                }
            }
        }
    };
    load_map(map);
    load_map(foreground);
    load_map(background);
}

void handle_key_input(
    const SDL_State& state,
    GameState& game_state,
    GameObject& game_object,
    SDL_Scancode key,
    bool key_down
)
{
    const float JUMP_FORCE = -200.0f;
    if (game_object.type == ObjectType::player)
    {
        switch(game_object.data.player.state)
        {
            case PlayerState::idle:
            {
                if (key == SDL_SCANCODE_W && key_down)
                {
                    game_object.data.player.state = PlayerState::jumping;
                    game_object.velocity.y += JUMP_FORCE;
                }
                break;
            }

            case PlayerState::running:
            {
                if (key == SDL_SCANCODE_W && key_down)
                {
                    game_object.data.player.state = PlayerState::jumping;
                    game_object.velocity.y += JUMP_FORCE;
                }
                break;
            }
        }
    }
}

void draw_paralax_bg(
    SDL_Renderer* renderer,
    SDL_Texture* texture,
    float x_velocity,
    float& scroll_pos,
    float scroll_factor,
    float delta_time
)
{
    scroll_pos -= x_velocity * scroll_factor * delta_time;
    if (scroll_pos <= -texture->w)
    {
        scroll_pos = 0;
    }

    SDL_FRect dest {
        .x = scroll_pos,
        .y = 35,
        .h = static_cast<float>(texture->h),
        .w = texture->w * 2.0f,
    };

    SDL_RenderTextureTiled(renderer, texture, nullptr, 1, &dest);
}
