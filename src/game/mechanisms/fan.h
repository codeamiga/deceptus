#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include <filesystem>
#include <optional>
#include <memory>

#include "gamedeserializedata.h"
#include "gamemechanism.h"
#include "gamenode.h"

struct TmxLayer;
struct TmxObject;
struct TmxTileSet;


class Fan : public GameMechanism, public GameNode
{
   public:

      enum class TileDirection
      {
         Up    =  0,
         Right =  8,
         Left  = 16,
         Down  = 24,
      };

      struct FanTile
      {
         sf::Vector2i mPosition;
         sf::Vector2f mDirection;
         sf::Rect<int32_t> mRect;
         b2Body* mBody = nullptr;
         TileDirection mDir;

         ~FanTile() = default;
      };

      Fan(GameNode* parent = nullptr);

      void draw(sf::RenderTarget& color, sf::RenderTarget& normal) override;
      void update(const sf::Time& dt) override;
      const sf::Rect<int32_t>& getPixelRect() const;
      void setEnabled(bool enabled) override;

      static void load(const GameDeserializeData& data);

      static void resetAll();
      static void addObject(GameNode* parent, const GameDeserializeData& data);
      static std::optional<sf::Vector2f> collide(const sf::Rect<int32_t>& player_rect);
      static void collide(const sf::Rect<int32_t>& playerRect, b2Body* body);
      static void merge();

      static std::vector<std::shared_ptr<GameMechanism>>& getFans();


   private:

      void updateSprite();

      static void createPhysics(const std::shared_ptr<b2World>& world, const std::shared_ptr<FanTile>& item);

      static std::vector<std::shared_ptr<GameMechanism>> __fan_instances;
      static std::vector<std::shared_ptr<FanTile>> __tile_instances;
      static std::vector<std::shared_ptr<TmxObject>> __object_instances;
      static std::vector<sf::Vector2f> __weight_instances;

      std::vector<std::shared_ptr<FanTile>> _tiles;

      sf::Vector2f _direction;
      sf::Rect<int32_t> _pixel_rect;
      float _speed = 1.0f;
      float _lever_lag = 1.0f;

      std::vector<sf::Sprite> _sprites;
      std::vector<float> _x_offsets_px;

      std::shared_ptr<sf::Texture> _texture;
};

