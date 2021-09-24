#pragma once

#include <cstdint>
#include <string>

struct PhysicsConfiguration
{
   PhysicsConfiguration() = default;

   float _time_step = 1.0f/60.0f;
   float _gravity = 8.5f;

   float _player_speed_max_walk = 2.5f;
   float _player_speed_max_run = 3.5f;
   float _player_speed_max_water = 1.5f;
   float _player_speed_max_air = 4.0f;
   float _player_friction = 0.0f;
   float _player_jump_strength = 3.3f;
   float _player_acceleration_ground = 0.1f;
   float _player_acceleration_air = 0.05f;
   float _player_deceleration_ground = 0.6f;
   float _player_deceleration_air = 0.65f;

   // jump
   int32_t _player_jump_steps = 9;
   int32_t _player_jump_after_contact_lost_ms = 100;
   int32_t _player_jump_buffer_ms = 100;
   int32_t _player_jump_minimal_duration_ms = 80;
   float _player_jump_falloff = 6.5f;
   float _player_jump_speed_factor = 0.1f;

   // dash
   int32_t _player_dash_frame_count = 20;
   float _player_dash_multiplier = 20.0f;
   float _player_dash_multiplier_increment_per_frame = -1.0f;
   float _player_dash_multiplier_scale_per_frame = 1.0f;
   float _player_dash_vector = 3.0f;

   // wall slide
   float _player_wall_slide_friction = 0.4f;

   // wall jump
   int32_t _player_wall_jump_frame_count = 20;
   float _player_wall_jump_vector_x = 6.0f;
   float _player_wall_jump_vector_y = 1.0f;
   float _player_wall_jump_multiplier = 20.0f;
   float _player_wall_jump_multiplier_increment_per_frame = -1.0f;
   float _player_wall_jump_multiplier_scale_per_frame = 1.0f;

   // double jump
   float _player_double_jump_factor = 6.0f;

   void deserializeFromFile(const std::string& filename = "data/config/physics.json");
   void serializeToFile(const std::string& filename = "data/config/physics.json");

   static PhysicsConfiguration& getInstance();
   static bool __initialized;


private:

   std::string serialize();
   void deserialize(const std::string& data);

};

