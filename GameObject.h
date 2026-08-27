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
};

enum class PlayerState
{
    idle,
    running,
    jumping,
};

struct PlayerData
{
    PlayerState state;
    PlayerData()
    {
        state = PlayerState::idle;
    }
};

struct LevelData {};
struct EnemyData {};

union ObjectData
{
    PlayerData player;
    LevelData level;
    EnemyData enemy;
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
    SDL_FRect collider;

    GameObject(): data{.level = LevelData()}, collider{0}
    {
        type = ObjectType::level;
        max_speed_x = 0;
        direction = 1;
        position = velocity = acceleration = glm::vec2(0);
        currentAnimation = -1;
        texture = nullptr;
        has_gravity = false;
    }
};
