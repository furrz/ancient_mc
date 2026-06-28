#pragma once
#include <glm/glm.hpp>
#include <leveldb/db.h>

#include "AABB.h"

class ConVars;
class Level;

class Player
{
    Level *level_;
    leveldb::DB *db_;
    glm::vec2 rot_{};
    glm::vec3 pos_{};
    glm::vec3 posOld_{};
    glm::vec3 vel_{};
    AABB box_{};
    bool onGround_{}, inWater_{}, flying_ = false;
    bool wasFPressed_ = false;

    float attrSpeedRunning_ = 0.035f;
    float attrSpeedWalking_ = 0.02F;
    float attrSpeedInAir_ = 0.005F;
    float attrDragHorizontal_ = 0.91F;
    float attrDragVertical_ = 0.98F;
    float attrDragGroundedHorizontal_ = 0.8F;
    float attrGravity_ = 0.005;
    float attrGravityInWater_ = 0.4f;
    float attrJumpVelocity_ = 0.12F;
    float attrFlyingSpeed_ = 0.1f;
    float attrSwimUpwardMaxVelocity_ = 0.07f;
    float attrSwimUpwardAcceleration_ = 0.01f;
    float attrMouseLookSpeed_ = 0.15;
    float attrMaxPitchAngle_ = 90.0f;
    float attrPlayerRadius_ = 0.3f;
    float attrPlayerHalfHeight_ = 0.9f;

public:
    explicit Player(leveldb::DB *db, ConVars *conVars, Level *level);
    void save() const;
    void resetPos();
    void setPos(glm::vec3 pos);

    void moveRelative(glm::vec2 movement, float speed);
    void move();
    void tick();
    void turn(glm::vec2 vec);

    [[nodiscard]] glm::vec2 rot() const { return rot_; }
    [[nodiscard]] glm::vec3 pos() const { return pos_; }
    [[nodiscard]] AABB box() const { return box_; }

    [[nodiscard]] glm::vec3 posInterpolated(const float dt) const
    {
        return posOld_ + (pos_ - posOld_) * dt;
    }
};