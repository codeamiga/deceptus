#pragma once

#include "constants.h"

#include "json/json.hpp"
#include "SFML/Graphics.hpp"

#include <cstdint>


class GameMechanism
{
   public:

      GameMechanism() = default;
      virtual ~GameMechanism() = default;

      virtual void draw(sf::RenderTarget& target, sf::RenderTarget& normal);
      virtual void update(const sf::Time& dt);

      virtual bool isEnabled() const;
      virtual void setEnabled(bool enabled);

      virtual int32_t getZ() const;
      virtual void setZ(const int32_t& z);

      virtual void serializeState(nlohmann::json&){}
      virtual void deserializeState(const nlohmann::json&){}
      virtual bool isSerialized() const;


   protected:

      int32_t _z_index = 0;
      bool _enabled = true;
      bool _serialized = false;
      MechanismVersion _version = MechanismVersion::Version1;
};

