#include "Player.h"

#include <glm/ext/scalar_constants.hpp>

#include "rng.h"

#include <algorithm>
#include <vector>
#include <GLFW/glfw3.h>

#include "ConVars.h"
#include "Input.h"
#include "Level.h"

struct SavedState { glm::vec3 pos; glm::vec3 vel; glm::vec2 rot; };

Player::Player(leveldb::DB *db, ConVars *conVars, Level *level) : db_(db)
{
    conVars->setupVar("player_phys_speed_running", "Player Running Speed", &attrSpeedRunning_);
    conVars->setupVar("player_phys_speed_walking", "Player Walking Speed", &attrSpeedWalking_);
    conVars->setupVar("player_phys_speed_in_air", "Player Speed In Air", &attrSpeedInAir_);
    conVars->setupVar("player_phys_flying_speed", "Player Flying Speed", &attrFlyingSpeed_);
    conVars->setupVar("player_phys_drag_horizontal", "Player Horizontal Drag Coefficient", &attrDragHorizontal_);
    conVars->setupVar("player_phys_drag_vertical", "Player Vertical Drag Coefficient", &attrDragVertical_);
    conVars->setupVar("player_phys_drag_grounded_horizontal", "Player Additional Horizontal Drag while Grounded", &attrDragGroundedHorizontal_);
    conVars->setupVar("player_phys_jump_velocity", "Player Jump Velocity", &attrJumpVelocity_);
    conVars->setupVar("player_phys_gravity", "Player Gravity", &attrGravity_);
    conVars->setupVar("player_phys_gravity_in_water", "Player Gravity in Water", &attrGravityInWater_);
    conVars->setupVar("player_phys_swim_upward_max_velocity", "Player Maximum Velocity while Swimming Upward", &attrSwimUpwardMaxVelocity_);
    conVars->setupVar("player_phys_swim_upward_acceleration", "Player Acceleration while Swimming Upward", &attrSwimUpwardAcceleration_);
    conVars->setupVar("player_phys_radius", "Player Hitbox Half-Width", &attrPlayerRadius_);
    conVars->setupVar("player_phys_half_height", "Player Hitbox Half-Height", &attrPlayerHalfHeight_);
    conVars->setupVar("player_cam_max_pitch_angle", "Maximum Upward/Downward Look Angle", &attrMaxPitchAngle_);
    conVars->setupVar("player_cam_mouse_look_speed", "Mouse Look Sensitivity", &attrMouseLookSpeed_, ConVars::SaveType::UserPrefs);
    conVars->setupVar("player_state_flying", "Player is Flying", &flying_, ConVars::SaveType::None);

    level_ = level;
    std::string saved_pos_bytes;
    if (db_->Get({}, "playerState", &saved_pos_bytes).ok()) {
        SavedState savedPos{};
        memcpy(&savedPos, saved_pos_bytes.data(), sizeof(savedPos));
        setPos(savedPos.pos);
        vel_ = savedPos.vel;
        rot_ = savedPos.rot;
    } else {
        resetPos();
    }
}

void Player::save() const
{
    const SavedState state {
        pos_, vel_, rot_
    };
    db_->Put({}, "playerState", { reinterpret_cast<const char *>(&state), sizeof(state) });
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
    const glm::vec3 size { attrPlayerRadius_, attrPlayerHalfHeight_, attrPlayerRadius_ };

    pos_ = pos;
    posOld_ = pos;
    box_ = AABB { pos - size, pos + size };
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

    if (!noClip_) {
        level_->getCubes(box_.expanded(vel), cubes, COLLIDABLE);
        level_->getCubes(box_.expanded(vel), buoyantCubes, BUOYANT);
    }


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
    const bool nPressed = Input::getKey(GLFW_KEY_N);

    if (fPressed && !wasFPressed_) flying_ = !flying_;
    wasFPressed_ = fPressed;
    if (nPressed && !wasNPressed_) noClip_ = !noClip_;
    wasNPressed_ = nPressed;

    if (flying_)
        vel_.y = jumpPressed ? attrFlyingSpeed_ : Input::getKey(GLFW_KEY_LEFT_CONTROL) ? -attrFlyingSpeed_ : 0;
    else if (jumpPressed && onGround_ && !inWater_)
        vel_.y += attrJumpVelocity_;
    else if (jumpPressed && inWater_) {
        if (vel_.y < attrSwimUpwardMaxVelocity_) {
            vel_.y += attrSwimUpwardAcceleration_;
        }
    }

    moveRelative(movement, (onGround_ || flying_) ? (running ? attrSpeedRunning_ : attrSpeedWalking_) : attrSpeedInAir_);

    if (!flying_) {
        vel_.y = vel_.y - attrGravity_ * (inWater_ ? attrGravityInWater_ : 1.0f);
    }

    move();
    vel_.x *= attrDragHorizontal_;
    vel_.y *= attrDragVertical_;
    vel_.z *= attrDragHorizontal_;

    if (onGround_ || flying_) {
        vel_.x *= attrDragGroundedHorizontal_;
        vel_.z *= attrDragGroundedHorizontal_;
    }

}

void Player::turn(const glm::vec2 vec)
{
    rot_.y = rot_.y + vec.x * attrMouseLookSpeed_;
    rot_.x = rot_.x - vec.y * attrMouseLookSpeed_;

    rot_.x = std::max(rot_.x, -attrMaxPitchAngle_);
    rot_.x = std::min(rot_.x, attrMaxPitchAngle_);
}
