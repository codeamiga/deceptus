#include "menuscreenmain.h"

#include "game/messagebox.h"
#include "game/gamestate.h"
#include "game/savestate.h"
#include "menu.h"


#define DEV_SAVE_STATE 1


MenuScreenMain::MenuScreenMain()
{
   setFilename("data/menus/titlescreen.psd");
}


void MenuScreenMain::update(const sf::Time& /*dt*/)
{

}


void MenuScreenMain::keyboardKeyPressed(sf::Keyboard::Key key)
{
   if (key == sf::Keyboard::Up)
   {
      up();
   }

   else if (key == sf::Keyboard::Down)
   {
      down();
   }

   else if (key == sf::Keyboard::Return)
   {
      select();
   }
}


void MenuScreenMain::loadingFinished()
{
   SaveState::deserializeFromFile();
   updateLayers();
}


void MenuScreenMain::up()
{
   switch (mSelection)
   {
      case Selection::Start:
         mSelection = Selection::Quit;
         break;
      case Selection::Options:
         mSelection = Selection::Start;
         break;
      case Selection::Quit:
         mSelection = Selection::Options;
         break;
   }

   updateLayers();
}


void MenuScreenMain::down()
{
   switch (mSelection)
   {
      case Selection::Start:
         mSelection = Selection::Options;
         break;
      case Selection::Options:
         mSelection = Selection::Quit;
         break;
      case Selection::Quit:
         mSelection = Selection::Start;
         break;
   }

   updateLayers();
}


void MenuScreenMain::select()
{
   switch (mSelection)
   {
      case Selection::Start:
#ifdef DEV_SAVE_STATE
      Menu::getInstance()->show(Menu::MenuType::FileSelect);
#else
      Menu::getInstance()->hide();
      GameState::getInstance().enqueueResume();
#endif
         break;
      case Selection::Options:
         Menu::getInstance()->show(Menu::MenuType::Options);
         break;
      case Selection::Quit:
         MessageBox::question(
            "Are you sure you want to quit?",
            [this](MessageBox::Button button) {if (button == MessageBox::Button::Yes) _exit_callback();}
         );
         break;
   }
}


void MenuScreenMain::setExitCallback(MenuScreenMain::ExitCallback callback)
{
    _exit_callback = callback;
}


void MenuScreenMain::updateLayers()
{
   const auto canContinue = !SaveState::allEmpty();

   _layers["continue_0"]->_visible = canContinue && (mSelection != Selection::Start);
   _layers["continue_1"]->_visible = canContinue && (mSelection == Selection::Start);

   _layers["start_0"]->_visible = !canContinue && (mSelection != Selection::Start);
   _layers["start_1"]->_visible = !canContinue && (mSelection == Selection::Start);

   _layers["options_0"]->_visible = (mSelection != Selection::Options);
   _layers["options_1"]->_visible = (mSelection == Selection::Options);

   _layers["quit_0"]->_visible = (mSelection != Selection::Quit);
   _layers["quit_1"]->_visible = (mSelection == Selection::Quit);
}



/*
data/menus/titlescreen.psd
    bg_temp
    quit_0
    quit_1
    options_0
    options_1
    start_0
    start_1
    version
    credits
    logo
*/
