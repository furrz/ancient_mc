#pragma once
#include <glm/glm.hpp>

#include "AABB.h"

class Level;

class Player
{
    Level *level_;
    glm::vec2 rot_{};
    glm::vec3 pos_{};
    glm::vec3 posOld_{};
    glm::vec3 vel_{};
    AABB box_{};
    bool onGround_{}, inWater_{}, flying_ = true;
    bool wasFPressed_ = false;

public:

    explicit Player(Level *level);
    void resetPos();
    void setPos(glm::vec3 pos);

    void moveRelative(glm::vec2 movement, float speed);
    void move();
    void tick();
    void turn(glm::vec2 vec);

    [[nodiscard]] glm::vec2 rot() const { return rot_; }
    [[nodiscard]] glm::vec3 pos() const { return pos_; }
    [[nodiscard]] glm::vec3 posOld() const { return posOld_; }
    [[nodiscard]] AABB box() const { return box_; }
};