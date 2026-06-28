#include "Player.h"

#include "rng.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    constexpr float W = 0.3f;
    constexpr float H = 0.9f;
    constexpr glm::vec3 SIZE { W, H, W };

    box_ = AABB { pos - SIZE, pos + SIZE };
}

void Player::moveRelative(glm::vec2 movement, const float speed)
{
    if (movement == glm::vec2 {0, 0}) return;

    movement = glm::normalize(movement) * speed;

    const float sin = sinf(rot_.y * static_cast<float>(M_PI) / 180.0f);
    const float cos = cosf(rot_.y * static_cast<float>(M_PI) / 180.0f);
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
        vel_.y = jumpPressed ? 0.1f : Input::getKey(GLFW_KEY_LEFT_CONTROL) ? -0.1f : 0;
    else if (jumpPressed && onGround_ && !inWater_)
        vel_.y += JUMP_VELOCITY;
    else if (jumpPressed && inWater_) {
        if (vel_.y < 0.07f) vel_.y += 0.01f;
    }

    moveRelative(movement, (onGround_ || flying_) ? (running ? SPEED_RUNNING : SPEED_WALKING) : SPEED_IN_AIR);

    if (!flying_) vel_.y = vel_.y - GRAVITY * (inWater_ ? 0.4f : 1.0f);

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
    rot_.y = static_cast<float>(static_cast<double>(rot_.y) + static_cast<double>(vec.x) * 0.15);
    rot_.x = static_cast<float>(static_cast<double>(rot_.x) - static_cast<double>(vec.y) * 0.15);

    rot_.x = std::max(rot_.x, -90.0f);
    rot_.x = std::min(rot_.x, 90.0f);
}
