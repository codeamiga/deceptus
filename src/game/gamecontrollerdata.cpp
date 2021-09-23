#include "gamecontrollerdata.h"


//-----------------------------------------------------------------------------
GameControllerData& GameControllerData::getInstance()
{
   static GameControllerData sInstance;
   return sInstance;
}


//-----------------------------------------------------------------------------
const GameControllerInfo& GameControllerData::getJoystickInfo() const
{
   return _joystick_info;
}


//-----------------------------------------------------------------------------
void GameControllerData::setJoystickInfo(const GameControllerInfo &joystickInfo)
{
   _joystick_info = joystickInfo;
}


//-----------------------------------------------------------------------------
bool GameControllerData::isControllerUsed() const
{
  return !_joystick_info.getAxisValues().empty();
}
