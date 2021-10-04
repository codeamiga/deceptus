#include "room.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "camerasystem.h"
#include "gameconfiguration.h"
#include "screentransition.h"
#include "fadetransitioneffect.h"
#include "framework/tmxparser/tmxproperty.h"
#include "framework/tmxparser/tmxproperties.h"
#include "framework/tools/timer.h"

/*

   room a_0
   +--------------------------------------------------------------------------+
   |                                                                          |
   |  screen                                                                  |
   |  +---------------/.\----------------+                                    |
   |  |                |                 |                                    |
   |  |                |                 |                                    |
   |  |                |                 |                                    |
   |  <----------------+----------------->                                    |
   |  |                |                 |                                    |
   |  |                |                 |                                    |
   |  |                |                 |                                    |
   |  +---------------\./----------------+                                    |
   |                                                                          |
   |                                    room a_1                              |
   +------------------------------------+-------------------------------------+
                                        |                                     |
                                        |                                     |
                                        |                                     |
                                        |                                     |
                                        |                                     |
                                        |                                     |
                                        |                                     |
                                        |                                     |
                                        |                                     |
                                        +-------------------------------------+

*/


Room::Room(const sf::FloatRect& rect)
{
   static int32_t __id = 0;

   _rects.push_back(rect);
   _id = __id;
   __id++;
}


std::vector<sf::FloatRect>::const_iterator Room::findRect(const sf::Vector2f& p) const
{
   const auto it = std::find_if(
         _rects.begin(),
         _rects.end(),
         [p](const sf::FloatRect& rect){
            return rect.contains(p);
         }
      );

   return it;
}


bool Room::correctedCamera(float& x, float& y, float focusOffset, float view_ratio_y) const
{

/*

      +--------------->----+-----<--------------------------------+
      |               |    |     |                                |
      |               |    |     |                                |
      |               |    |     |                                |
      |               |    |     |                                |
      |               |    |     |                                |
      |               |    |     |                                |
      |               |    |     |                                |
      |               |    |     |                                |
      |               |    |     |                                |
      |               |    |     |                                |
      +--------------->----+-----<--------------------------------+--- y = player y + screen height / 1.5
      |               |    |     |                                |
      |               |    |     |                                |
      |               |    |     |                                |
      |               |    |     |                                |
      +--------------->----+-----<--------------------------------+
                      |    |     |
                      |    |     |
                   focus   |   focus
                   zone 0  |   zone 1
                           |
                           |
                         player x
*/


   if (_rects.empty())
   {
      return false;
   }

   // workflow (only for 'current' room)
   //
   // 1) check which in which rectangle the current camera center lies
   //    -> find the right FloatRect
   auto pos = sf::Vector2f{x, y};
   const auto rect_it =  findRect(pos);
   if (rect_it == _rects.end())
   {
      // that's an error.
      return false;
   }

   // 2) check if
   //    a) screen's right is within room bounds, assign if necessary
   //    b) screen's left is within room bounds, assign if necessary
   //    c) screen's top is within room bounds, assign if necessary
   //    d) screen's bottom is within room bounds, assign if necessary

   const auto rect = *rect_it;

   const auto config = GameConfiguration::getInstance();

   // need to incorporate the focus offset here because the player is not
   // necessarily in the middle of the screen but maybe a little more to the
   // left or to the right depending on its orientation
   const auto half_width  = static_cast<float>(config._view_width / 2.0f);
   const auto height = static_cast<float>(config._view_height);

   const auto l = pos + sf::Vector2f{- half_width - focusOffset, 0.0f};
   const auto r = pos + sf::Vector2f{  half_width - focusOffset, 0.0f};

   const auto height_top = height * (1.0f - 1.0f / view_ratio_y);
   const auto height_bottom = height / view_ratio_y;

   const auto u = pos + sf::Vector2f{0.0f, -height_bottom};
   const auto d = pos + sf::Vector2f{0.0f, height_top};

   auto corrected = false;

   if (!rect.contains(l))
   {
      // camera center is out of left boundary
      x = rect.left + half_width + focusOffset;
      corrected = true;
   }
   else if (!rect.contains(r))
   {
      // camera center is out of right boundary
      x = rect.left + rect.width - half_width + focusOffset;
      corrected = true;
   }

   if (!rect.contains(u))
   {
      // camera center is out of upper boundary
      y = rect.top + height_bottom;
      corrected = true;
   }
   else if (!rect.contains(d))
   {
      // camera center is out of lower boundary
      y = rect.top + rect.height - height_top;
      corrected = true;
   }

   return corrected;
}


std::shared_ptr<Room> Room::find(const sf::Vector2f& p, const std::vector<std::shared_ptr<Room>>& rooms)
{
   const auto room_it = std::find_if(rooms.begin(), rooms.end(), [p, rooms](const std::shared_ptr<Room>& r){
         const auto& it = r->findRect(p);
         return (it != r->_rects.end());
      }
   );

   return (room_it != rooms.end()) ? (*room_it) : nullptr;
}


void Room::deserialize(TmxObject* tmx_object, std::vector<std::shared_ptr<Room>>& rooms)
{
   // ignore invalid rects
   const auto config = GameConfiguration::getInstance();
   if (tmx_object->_width_px < config._view_width)
   {
      std::cerr << "[!] ignoring rect, room width smaller than screen width" << std::endl;
      return;
   }

   if (tmx_object->_height_px < config._view_height)
   {
      std::cerr << "[!] ignoring rect, room height smaller than screen height" << std::endl;
      return;
   }

   // read key from tmx object
   std::istringstream f(tmx_object->_name);
   std::string key;
   if (!getline(f, key, '_'))
   {
      key = tmx_object->_name;
   }

   if (key.empty())
   {
      std::cerr << "[!] ignoring unnamed room" << std::endl;
      return;
   }

   auto rect = sf::FloatRect{
      tmx_object->_x_px,
      tmx_object->_y_px,
      tmx_object->_width_px,
      tmx_object->_height_px
   };

   // check if room already exists
   const auto it = std::find_if(rooms.begin(), rooms.end(), [key](const std::shared_ptr<Room>& r){
         return (r->_name == key);
      }
   );

   if (it == rooms.end())
   {
      // create new room
      auto room = std::make_shared<Room>(rect);
      room->_name = key;

      // deserialize room properties
      if (tmx_object->_properties)
      {
         const auto transition_it = tmx_object->_properties->_map.find("transition");
         if (transition_it != tmx_object->_properties->_map.end())
         {
            if (transition_it->second->_value_string == "fade_out_fade_in")
            {
               room->_transition_effect = TransitionEffect::FadeOutFadeIn;
            }
         }

         const auto fade_in_speed_it = tmx_object->_properties->_map.find("fade_in_speed");
         if (fade_in_speed_it != tmx_object->_properties->_map.end())
         {
            room->_fade_in_speed = fade_in_speed_it->second->_value_float.value();
         }

         const auto fade_out_speed_it = tmx_object->_properties->_map.find("fade_out_speed");
         if (fade_out_speed_it != tmx_object->_properties->_map.end())
         {
            room->_fade_out_speed = fade_out_speed_it->second->_value_float.value();
         }

         const auto delay_between_effects_it = tmx_object->_properties->_map.find("delay_between_effects_ms");
         if (delay_between_effects_it != tmx_object->_properties->_map.end())
         {
            room->_delay_between_effects_ms = std::chrono::milliseconds{*delay_between_effects_it->second->_value_int};
         }

         const auto camera_sync_after_fade_out_it = tmx_object->_properties->_map.find("camera_sync_after_fade_out");
         if (camera_sync_after_fade_out_it != tmx_object->_properties->_map.end())
         {
            room->_camera_sync_after_fade_out = camera_sync_after_fade_out_it->second->_value_bool.value();
         }

         const auto delay_it = tmx_object->_properties->_map.find("camera_lock_delay_ms");
         if (delay_it != tmx_object->_properties->_map.end())
         {
            room->_camera_lock_delay = std::chrono::milliseconds{*delay_it->second->_value_int};
         }
      }

      rooms.push_back(room);
      // std::cout << "[i] adding room: " << key << std::endl;
   }
   else
   {
      // merge room
      auto& room = *it;

      // test for overlaps
      if (std::any_of(room->_rects.begin(), room->_rects.end(), [rect](const sf::FloatRect& r){
               return r.intersects(rect);
            }
         )
      )
      {
         std::cerr << "[!] bad rect intersection for room " << key << std::endl;
      }

      room->_rects.push_back(rect);
   }
}


void Room::startTransition()
{
   if (!_transition_effect.has_value())
   {
      return;
   }

   switch (_transition_effect.value())
   {
      case TransitionEffect::FadeOutFadeIn:
      {
         auto screen_transition = std::make_unique<ScreenTransition>();
         auto fade_out = std::make_shared<FadeTransitionEffect>();
         auto fade_in = std::make_shared<FadeTransitionEffect>();

         fade_out->_direction = FadeTransitionEffect::Direction::FadeOut;
         fade_out->_speed = _fade_out_speed;

         fade_in->_direction = FadeTransitionEffect::Direction::FadeIn;
         fade_in->_value = 1.0f;
         fade_in->_speed = _fade_in_speed;

         screen_transition->_effect_1 = fade_out;
         screen_transition->_effect_2 = fade_in;

         screen_transition->_delay_between_effects_ms = _delay_between_effects_ms;
         screen_transition->startEffect1();

         screen_transition->_callbacks_effect_1_ended.push_back(
            [this](){
               if (_camera_sync_after_fade_out)
               {
                  _camera_locked = false;
                  auto& camera = CameraSystem::getCameraSystem();
                  camera.setRoom(getptr());
                  camera.syncNow();
               }
            }
         );

         screen_transition->_callbacks_effect_2_ended.push_back(
            [](){
               ScreenTransitionHandler::getInstance()._transition.reset();
            }
         );

         ScreenTransitionHandler::getInstance()._transition = std::move(screen_transition);

         break;
      }
   }
}


void Room::lockCamera()
{
   _camera_locked = true;

   // delay shall be retrieved from room properties
   Timer::add(_camera_lock_delay.value(),
      [this](){
         // could be that the camera has been unlocked in the meantime
         if (_camera_locked)
         {
            auto& cameraSystem = CameraSystem::getCameraSystem();
            cameraSystem.setRoom(getptr());
            _camera_locked = false;
         }
      },
      Timer::Type::Singleshot
   );
}
