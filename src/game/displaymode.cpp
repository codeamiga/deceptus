#include "displaymode.h"


//-----------------------------------------------------------------------------
DisplayMode DisplayMode::sInstance;


//-----------------------------------------------------------------------------
void DisplayMode::toggle(Display mode)
{
  if (_mode & mode)
  {
     _mode &= ~ mode;
  }
  else
  {
     _mode |= mode;
  }
}


//-----------------------------------------------------------------------------
void DisplayMode::sync()
{
   for (auto& mode : _queued_set)
   {
      _mode |= mode;
   }

   for (auto& mode : _queued_unset)
   {
      _mode &= ~ mode;
   }

   for (auto& mode : _queued_toggle)
   {
      toggle(mode);
   }

   _queued_set.clear();
   _queued_unset.clear();
   _queued_toggle.clear();
}


//-----------------------------------------------------------------------------
void DisplayMode::enqueueSet(Display mode)
{
   _queued_set.push_back(mode);
}


//-----------------------------------------------------------------------------
void DisplayMode::enqueueUnset(Display mode)
{
   _queued_unset.push_back(mode);
}


void DisplayMode::enqueueToggle(Display mode)
{
   _queued_toggle.push_back(mode);
}


//-----------------------------------------------------------------------------
int32_t DisplayMode::get()
{
  return _mode;
}


//-----------------------------------------------------------------------------
bool DisplayMode::isSet(Display mode)
{
   return _mode & mode;
}


//-----------------------------------------------------------------------------
DisplayMode& DisplayMode::getInstance()
{
  return sInstance;
}
