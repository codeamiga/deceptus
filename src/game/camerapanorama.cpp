#include "camerapanorama.h"

#include "displaymode.h"
#include "gameconfiguration.h"
#include "gamecontrollerdata.h"
#include "gamecontrollerintegration.h"
#include "framework/joystick/gamecontroller.h"
#include "framework/math/sfmlmath.h"
#include "tweaks.h"


CameraPanorama CameraPanorama::__instance;


//-----------------------------------------------------------------------------
CameraPanorama& CameraPanorama::getInstance()
{
   return __instance;
}


//-----------------------------------------------------------------------------
void CameraPanorama::update()
{
   const auto& tweaks = Tweaks::instance();

   auto limit_look_vector = [&](sf::Vector2f& look_vector){
      if (!DisplayMode::getInstance().isSet(Display::Map))
      {
         const auto len = SfmlMath::length(look_vector);
         if (len > tweaks._cpan_max_distance_px)
         {
            look_vector = SfmlMath::normalize(look_vector);
            look_vector *= tweaks._cpan_max_distance_px;
         }
      }
   };

   if (_look_state & static_cast<int32_t>(Look::Active))
   {
      constexpr auto speed = 3.0f;

      sf::Vector2f desired_look_vector = _look_vector;
      if (_look_state & static_cast<int32_t>(Look::Up))
      {
         desired_look_vector += sf::Vector2f(0.0f, -speed);
      }
      if (_look_state & static_cast<int32_t>(Look::Down))
      {
         desired_look_vector += sf::Vector2f(0.0f, speed);
      }
      if (_look_state & static_cast<int32_t>(Look::Left))
      {
         desired_look_vector += sf::Vector2f(-speed, 0.0f);
      }
      if (_look_state & static_cast<int32_t>(Look::Right))
      {
         desired_look_vector += sf::Vector2f(speed, 0.0f);
      }

      limit_look_vector(desired_look_vector);
      updateLookVector(desired_look_vector);
   }
   else if (GameControllerIntegration::getInstance().isControllerConnected())
   {
      const auto& axis_values = GameControllerData::getInstance().getJoystickInfo().getAxisValues();
      const auto x_axis = GameControllerIntegration::getInstance().getController()->getAxisIndex(SDL_CONTROLLER_AXIS_RIGHTX);
      const auto y_axis = GameControllerIntegration::getInstance().getController()->getAxisIndex(SDL_CONTROLLER_AXIS_RIGHTY);
      const auto x_normalized = axis_values[static_cast<uint32_t>(x_axis)] / 32767.0f;
      const auto y_normalized = axis_values[static_cast<uint32_t>(y_axis)] / 32767.0f;
      const auto tolerance_x = tweaks._cpan_tolerance_x;
      const auto tolerance_y = tweaks._cpan_tolerance_y;

      if (fabs(x_normalized) > tolerance_x || fabs(y_normalized) > tolerance_y)
      {
         const auto x_direction = std::signbit(x_normalized) ? -1.0f : 1.0f;
         const auto y_direction = std::signbit(y_normalized) ? -1.0f : 1.0f;
         const auto x_relative = x_direction * (fabs(x_normalized) - tolerance_x) / (1.0f - tolerance_x);
         const auto y_relative = y_direction * (fabs(y_normalized) - tolerance_y) / (1.0f - tolerance_y);

         sf::Vector2f desired_look_vector = _look_vector;
         desired_look_vector.x += x_relative * tweaks._cpan_look_speed_x;
         desired_look_vector.y += y_relative * tweaks._cpan_look_speed_y;

         limit_look_vector(desired_look_vector);
         updateLookVector(desired_look_vector);
      }
      else
      {
         _look_vector *= tweaks._cpan_snap_back_factor;
      }
   }
   else
   {
      _look_vector *= tweaks._cpan_snap_back_factor;
   }
}


//-----------------------------------------------------------------------------
void CameraPanorama::processKeyPressedEvents(const sf::Event& event)
{
   switch (event.key.code)
   {
      case sf::Keyboard::LShift:
      {
         updateLookState(Look::Active, true);
         break;
      }
      case sf::Keyboard::Left:
      {
         updateLookState(Look::Left, true);
         break;
      }
      case sf::Keyboard::Right:
      {
         updateLookState(Look::Right, true);
         break;
      }
      case sf::Keyboard::Up:
      {
         updateLookState(Look::Up, true);
         break;
      }
      case sf::Keyboard::Down:
      {
         updateLookState(Look::Down, true);
         break;
      }
      default:
      {
         break;
      }
   }
}


//-----------------------------------------------------------------------------
void CameraPanorama::processKeyReleasedEvents(const sf::Event& event)
{
   switch (event.key.code)
   {
      case sf::Keyboard::LShift:
      {
         updateLookState(Look::Active, false);
         break;
      }
      case sf::Keyboard::Left:
      {
         updateLookState(Look::Left, false);
         break;
      }
      case sf::Keyboard::Right:
      {
         updateLookState(Look::Right, false);
         break;
      }
      case sf::Keyboard::Up:
      {
         updateLookState(Look::Up, false);
         break;
      }
      case sf::Keyboard::Down:
      {
         updateLookState(Look::Down, false);
         break;
      }
      default:
      {
         break;
      }
   }
}


//-----------------------------------------------------------------------------
void CameraPanorama::updateLookState(Look look, bool enable)
{
   if (enable)
   {
      _look_state |= static_cast<int32_t>(look);
   }
   else
   {
      _look_state &= ~static_cast<int32_t>(look);
   }
}

//#include "cameraroomlock.h"
//#include "camerasystem.h"
//#include "player/player.h"


//-----------------------------------------------------------------------------
void CameraPanorama::updateLookVector(const sf::Vector2f& desired)
{
   // need focus offset from camera system
   // auto player = Player::getCurrent();
   // auto player_x = player->getPixelPositionf().x + desired.x;
   // auto player_y = player->getPixelPositionf().y + desired.y;
   // auto focus_offset = CameraSystem::getCameraSystem().getFocusOffset();
   // CameraRoomLock::instance().correctedCamera(player_x, player_y, focus_offset);
   //
   // _look_vector.x = player_x - player->getPixelPositionf().x;
   // _look_vector.y = player_y - player->getPixelPositionf().y;

   _look_vector = desired;
}


//-----------------------------------------------------------------------------
bool CameraPanorama::isLookActive() const
{
   return (_look_state & static_cast<int32_t>(Look::Active));
}


//-----------------------------------------------------------------------------
const sf::Vector2f& CameraPanorama::getLookVector() const
{
    return _look_vector;
}
