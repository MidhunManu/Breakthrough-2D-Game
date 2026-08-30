#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL3/SDL.h>

#include "Animation.h"

enum class ObjectType
{
    player,
    level,
    enemy,
    bullet,
};

enum class PlayerState
{
    idle,
    running,
    jumping,
};

enum class BulletState
{
    moving,
    hit,
    in_active,
};

enum class EnemyState
{
    alive,
    damaged,
    dead,
};

struct PlayerData
{
    PlayerState state;
    Timer gun_timer;
    PlayerData(): gun_timer(0.1f)
    {
        state = PlayerState::idle;
    }
};

struct BulletData
{
    BulletState state;
    BulletData(): state(BulletState::moving)
    {
    };
};

struct LevelData {};
struct EnemyData {
    EnemyState state;
    Timer damaged_state_timer;
    int health_points;

    EnemyData(): state(EnemyState::alive), damaged_state_timer(0.5f) {
        health_points = 100;
    };
};

union ObjectData
{
    PlayerData player;
    LevelData level;
    EnemyData enemy;
    BulletData bullet;
};

struct GameObject
{
    ObjectType type;
    ObjectData data;
    glm::vec2 position, velocity, acceleration;
    float direction;
    float max_speed_x;
    std::vector<Animation> animations;
    int currentAnimation;
    SDL_Texture* texture;
    bool has_gravity;
    bool grounded;
    SDL_FRect collider;
    Timer flash_timer;
    bool should_flash;
    int sprite_frame;

    GameObject(): data{.level = LevelData()}, collider{0}, flash_timer(0.05f)
    {
        type = ObjectType::level;
        max_speed_x = 0;
        direction = 1;
        position = velocity = acceleration = glm::vec2(0);
        currentAnimation = -1;
        texture = nullptr;
        has_gravity = false;
        grounded = false;
        should_flash = false;
        sprite_frame = 1;
    }
};
