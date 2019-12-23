#pragma once

#include <functional>
#include <memory>

#include <SFML/Graphics.hpp>

#include "image/layer.h"


class MessageBox
{
   public:

      enum class Button {
         Invalid = 0x00,
         Ok      = 0x01,
         Cancel  = 0x02,
         Yes     = 0x04,
         No      = 0x08,
      };

      enum class Type {
         Info,
         Warning,
         Error
      };

      using MessageBoxCallback = std::function<void(Button)>;

      MessageBox(Type type, const std::string& message, MessageBoxCallback cb, int32_t buttons);
      virtual ~MessageBox();

      static void draw(sf::RenderTarget& window, sf::RenderStates = sf::RenderStates::Default);
      static bool keyboardKeyPressed(sf::Keyboard::Key key);

      static void info(
         const std::string& message,
         MessageBoxCallback callback,
         int buttons = static_cast<int32_t>(Button::Ok)
      );

      static void question(
         const std::string& message,
         MessageBoxCallback callback,
         int buttons = (static_cast<int32_t>(Button::Yes) | static_cast<int32_t>(Button::No))
      );


   private:

      MessageBox();
      static void messageBox(Type type, const std::string& message, MessageBoxCallback callback, int32_t buttons);
      static void initializeLayers();

      Type mType;
      std::string mMessage;
      MessageBoxCallback mCallback;
      int mButtons = 0;
      bool mDrawn = false;
      std::function<void(void)> mButtonCallbackA;
      std::function<void(void)> mButtonCallbackB;

      static std::unique_ptr<MessageBox> mActive;
      static std::vector<std::shared_ptr<Layer>> sLayerStack; // SLAYER!
      static std::map<std::string, std::shared_ptr<Layer>> sLayers;
      static sf::Font sFont;
      static sf::Text sText;

      static bool sInitialized;
      void initializeControllerCallbacks();
};

