#pragma once

#include <filesystem>
#include <functional>
#include <stdint.h>

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "gamedeserializedata.h"
#include "gamemechanism.h"
#include "gamenode.h"


struct TmxLayer;
struct TmxObject;
struct TmxTileSet;


class Lever : public GameMechanism, public GameNode
{

public:

   using Callback = std::function<void(int32_t)>;

   enum class Type {
      TwoState,
      TriState
   };

   enum class State {
      Left   = -1,
      Middle = 0,
      Right  = 1,
   };

   Lever(GameNode* parent = nullptr);

   void update(const sf::Time& dt) override;
   void draw(sf::RenderTarget& color, sf::RenderTarget& normal) override;
   void setEnabled(bool enabled) override;

   void toggle();
   void setCallbacks(const std::vector<Callback>& callbacks);
   const sf::Rect<int32_t>& getPixelRect() const;

   static void addSearchRect(const std::shared_ptr<TmxObject>& rect);

   // requires a unified datastructure/mechanism in the future!
   static void merge(
      const std::vector<std::shared_ptr<GameMechanism>>& levers,
      const std::vector<std::shared_ptr<GameMechanism>>& lasers,
      const std::vector<std::shared_ptr<GameMechanism>>& platforms,
      const std::vector<std::shared_ptr<GameMechanism>>& fans,
      const std::vector<std::shared_ptr<GameMechanism>>& belts,
      const std::vector<std::shared_ptr<GameMechanism>>& spikes,
      const std::vector<std::shared_ptr<GameMechanism>>& spike_blocks,
      const std::vector<std::shared_ptr<GameMechanism>>& on_off_blocks,
      const std::vector<std::shared_ptr<GameMechanism>>& rotating_blades,
      const std::vector<std::shared_ptr<GameMechanism>>& doors
   );

   [[deprecated]] static std::vector<std::shared_ptr<GameMechanism>> load(GameNode* parent, const GameDeserializeData& data);
   void setup(const GameDeserializeData& data);

   void updateReceivers();

   void serializeState(nlohmann::json& j) override;
   void deserializeState(const nlohmann::json& j) override;


private:

   void updateSprite();
   void updateDirection();
   void updateTargetPositionReached();

   Type _type = Type::TwoState;

   State _target_state = State::Left;
   State _state_previous = State::Left;

   std::vector<Callback> _callbacks;
   sf::Rect<int32_t> _rect;
   bool _player_at_lever = false;
   sf::Sprite _sprite;
   int32_t _offset = 0;
   int32_t _dir = 0;
   bool _reached = false;
   bool _reached_previous = false;
   float _idle_time_s = 0.0f;

   std::shared_ptr<sf::Texture> _texture;

   static std::vector<std::shared_ptr<TmxObject>> __rectangles;
};


