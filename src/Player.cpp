#include "Player.h"

#include <glm/ext/scalar_constants.hpp>

#include "rng.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>

#include "Input.h"
#include "Level.h"

constexpr float SPEED_RUNNING = 0.035f;
constexpr float SPEED_WALKING = 0.02F;
constexpr float SPEED_IN_AIR = 0.005F;
constexpr float DRAG_HORIZONTAL = 0.91F;
constexpr float DRAG_VERTICAL = 0.98F;
constexpr float DRAG_GROUNDED_HORIZONTAL = 0.8F;
constexpr float GRAVITY = 0.005;
constexpr float JUMP_VELOCITY = 0.12F;
constexpr float FLYING_SPEED = 0.1f;
constexpr float SWIM_UPWARD_MAX_VELOCITY = 0.07f;
constexpr float SWIM_UPWARD_ACCELERATION = 0.01f;
constexpr float LOOK_SPEED = 0.15;
constexpr float MAX_PITCH_ANGLE = 90.0f;
constexpr float PLAYER_RADIUS = 0.3f;
constexpr float PLAYER_HALF_HEIGHT = 0.9f;
constexpr float GRAVITY_IN_WATER = 0.4f;

Player::Player(Level *level)
{
    level_ = level;
    resetPos();
}

void Player::resetPos()
{
    float x = RNG::randomFloat() * static_cast<float>(level_->size().x);
    float y = static_cast<float>(level_->size().y);
    float z = RNG::randomFloat() * static_cast<float>(level_->size().z);
    setPos({ x, y, z });
}

void Player::setPos(const glm::vec3 pos)
{
    pos_ = pos;
    posOld_ = pos;
    constexpr glm::vec3 SIZE { PLAYER_RADIUS, PLAYER_HALF_HEIGHT, PLAYER_RADIUS };

    box_ = AABB { pos - SIZE, pos + SIZE };
}

void Player::moveRelative(glm::vec2 movement, const float speed)
{
    if (movement == glm::vec2 {0, 0}) return;

    movement = glm::normalize(movement) * speed;

    const float sin = sinf(rot_.y * glm::pi<float>() / 180.0f);
    const float cos = cosf(rot_.y * glm::pi<float>() / 180.0f);
    vel_.x += movement.x * cos - movement.y * sin;
    vel_.z += movement.y * cos + movement.x * sin;
}

void Player::move()
{
    static std::vector<AABB> cubes;
    static std::vector<AABB> buoyantCubes;

    cubes.clear();
    buoyantCubes.clear();

    auto vel = vel_;
    const auto org = vel_;
    level_->getCubes(box_.expanded(vel), cubes, COLLIDABLE);
    level_->getCubes(box_.expanded(vel), buoyantCubes, BUOYANT);

    // Handle collision with blocks
    for(const auto& bb : cubes) {
        vel.y = bb.clipAxisCollide<1, 0, 2>(box_, vel.y);
    }

    box_ = box_.moved({ 0, vel.y, 0 });

    for(const auto& bb : cubes) {
        vel.x = bb.clipAxisCollide<0, 1, 2>(box_, vel.x);
    }

    box_ = box_.moved({ vel.x, 0, 0 });

    for(const auto& bb : cubes) {
        vel.z = bb.clipAxisCollide<2, 1, 0>(box_, vel.z);
    }

    box_ = box_.moved({ 0, 0, vel.z });

    // Check if we're underwater
    inWater_ = false;
    for (const auto& bb : buoyantCubes) {
        if (bb.intersects(box_)) {
            inWater_ = true;
            break;
        }
    }

    onGround_ = org.y != vel.y && org.y < 0;

    if (org.x != vel.x) vel_.x = 0;
    if (org.y != vel.y) vel_.y = 0;
    if (org.z != vel.z) vel_.z = 0;

    pos_ = glm::vec3 {
        (box_.a.x + box_.b.x) / 2.0f,
        (box_.a.y + 1.62f),
        (box_.a.z + box_.b.z) / 2.0f
    };
}


void Player::tick()
{
    posOld_ = pos_;

    glm::vec2 movement{};
    if (Input::getKey(GLFW_KEY_R)) resetPos();

    if (Input::getKey(GLFW_KEY_W)) movement.y--;
    if (Input::getKey(GLFW_KEY_S)) movement.y++;
    if (Input::getKey(GLFW_KEY_A)) movement.x--;
    if (Input::getKey(GLFW_KEY_D)) movement.x++;

    const bool jumpPressed = Input::getKey(GLFW_KEY_SPACE);
    const bool running = Input::getKey(GLFW_KEY_LEFT_SHIFT);
    const bool fPressed = Input::getKey(GLFW_KEY_F);

    if (fPressed && !wasFPressed_) flying_ = !flying_;
    wasFPressed_ = fPressed;

    if (flying_)
        vel_.y = jumpPressed ? FLYING_SPEED : Input::getKey(GLFW_KEY_LEFT_CONTROL) ? -FLYING_SPEED : 0;
    else if (jumpPressed && onGround_ && !inWater_)
        vel_.y += JUMP_VELOCITY;
    else if (jumpPressed && inWater_) {
        if (vel_.y < SWIM_UPWARD_MAX_VELOCITY) {
            vel_.y += SWIM_UPWARD_ACCELERATION;
        }
    }

    moveRelative(movement, (onGround_ || flying_) ? (running ? SPEED_RUNNING : SPEED_WALKING) : SPEED_IN_AIR);

    if (!flying_) {
        vel_.y = vel_.y - GRAVITY * (inWater_ ? GRAVITY_IN_WATER : 1.0f);
    }

    move();
    vel_.x *= DRAG_HORIZONTAL;
    vel_.y *= DRAG_VERTICAL;
    vel_.z *= DRAG_HORIZONTAL;

    if (onGround_ || flying_) {
        vel_.x *= DRAG_GROUNDED_HORIZONTAL;
        vel_.z *= DRAG_GROUNDED_HORIZONTAL;
    }

}

void Player::turn(const glm::vec2 vec)
{
    rot_.y = rot_.y + vec.x * LOOK_SPEED;
    rot_.x = rot_.x - vec.y * LOOK_SPEED;

    rot_.x = std::max(rot_.x, -MAX_PITCH_ANGLE);
    rot_.x = std::min(rot_.x, MAX_PITCH_ANGLE);
}
