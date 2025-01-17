#pragma once

// Bubble cube
//
// - You can stay on top of it as long as you want.
// - Once you jump off/ stop colliding with it, it vanishes "pop animation"
// - Cube reappears after n seconds.
// - Basically is a 1-jump-platform before vanishes.


class GameNode;
struct TmxObject;

#include "fixturenode.h"
#include "gamedeserializedata.h"
#include "gamemechanism.h"

#include "Box2D/Box2D.h"

#include <filesystem>


class BubbleCube : public FixtureNode, public GameMechanism
{

public:

   BubbleCube(GameNode* parent, const GameDeserializeData& data);

   void draw(sf::RenderTarget& target, sf::RenderTarget& normal) override;
   void update(const sf::Time& dt) override;

   void beginContact(b2Contact* contact, FixtureNode* other);
   void endContact(FixtureNode* other);


private:

   void updatePosition();
   void updateRespawnCondition();
   void updatePoppedCondition();
   void updatePopOnCollisionCondition();
   void updatePushDownOffset(const sf::Time& dt);
   void updateMaxDurationCondition(const sf::Time& dt);
   void updateFootSensorContact();
   void updateJumpedOffPlatformCondition();

   void pop();

   float _x_px = 0.0f;
   float _y_px = 0.0f;
   float _push_down_offset_px = 0.0f;
   sf::IntRect _fixed_rect_px;
   sf::IntRect _foot_collision_rect_px;
   float _elapsed_s = 0.0f;
   float _pop_elapsed_s = 0.0f;
   sf::Time _pop_time;
   bool _popped = false;
   bool _foot_sensor_contact = false;
   bool _exceeded_max_contact_duration = false;
   bool _collided_with_surrounding_areas = false;
   bool _jumped_off_this_platform = false;
   std::optional<size_t> _colliding_body_count;

   int32_t _foot_sensor_triggered_counter = 0;
   bool _lost_foot_contact = false;

   // settings
   float _pop_time_respawn_s = 3.0f;
   float _animation_offset_s = 0.0f;
   float _mapped_value_normalized = 0.0f;
   bool _move_down_on_contact = true;
   float _move_down_velocity = 0.5f;
   float _push_down_offset_m = 0.0f;
   float _contact_duration_s = 0.0f;
   std::optional<float> _maximum_contact_duration_s;

   // sf
   std::shared_ptr<sf::Texture> _texture;
   sf::Sprite _sprite;

   // b2d
   b2Body* _body = nullptr;
   b2Fixture* _fixture = nullptr;
   b2Vec2 _position_m;
   b2PolygonShape _shape;
};

