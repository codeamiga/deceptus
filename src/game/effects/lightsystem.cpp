#include "lightsystem.h"

#include "framework/tmxparser/tmxobject.h"
#include "framework/tmxparser/tmxtools.h"
#include "framework/tmxparser/tmxproperties.h"
#include "framework/tmxparser/tmxproperty.h"
#include "game/debugdraw.h"
#include "game/level.h"
#include "game/player/player.h"
#include "texturepool.h"

#include <iostream>
#include <math.h>

#include <SFML/OpenGL.hpp>


//-----------------------------------------------------------------------------
namespace
{
static constexpr auto max_distance_m2 = 100.0f; // depends on the view dimensions
}


//-----------------------------------------------------------------------------
LightSystem::LightSystem()
{
   // prepare unit circle for circle shapes
   for (auto i = 0u; i < segments; i++)
   {
      auto angle = (2.0 * M_PI) * (i / static_cast<double>(segments));

      auto x = static_cast<float>(cos(angle));
      auto y = static_cast<float>(sin(angle));

      _unit_circle[i] = b2Vec2{x, y};
   }

   if (!_light_shader.loadFromFile("data/shaders/light.frag", sf::Shader::Fragment))
   {
      std::cout << "[!] error loading bump mapping shader" << std::endl;
   }
}


//-----------------------------------------------------------------------------
void LightSystem::drawShadowQuads(sf::RenderTarget& target, std::shared_ptr<LightSystem::LightInstance> light) const
{
   // do not draw lights that are too far away
   auto player_body = Player::getCurrent()->getBody();

   auto light_pos_m = light->_pos_m + light->_center_offset_m;

   for (b2Body* b = Level::getCurrentLevel()->getWorld()->GetBodyList(); b; b = b->GetNext())
   {
      if (b == player_body)
         continue;

      if (!b->IsActive())
      {
         continue;
      }

      for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
      {
         // if something doesn't collide, it probably shouldn't have any impact on lighting, too
         if (f->IsSensor())
         {
            continue;
         }

         auto shape = f->GetShape();

         auto shape_polygon = dynamic_cast<b2PolygonShape*>(shape);
         auto shape_chain = dynamic_cast<b2ChainShape*>(shape);
         auto shape_circle = dynamic_cast<b2CircleShape*>(shape);

         if (shape_circle)
         {
            auto center = shape_circle->GetVertex(0) + b->GetTransform().p;
            if ((light_pos_m - center).LengthSquared() > max_distance_m2)
               continue;

            std::array<b2Vec2, segments> circle_positions;
            for (auto i = 0u; i < segments; i++)
            {
               circle_positions[i] = b2Vec2{
                  center.x + _unit_circle[i].x * shape_circle->m_radius * 1.2f,
                  center.y + _unit_circle[i].y * shape_circle->m_radius * 1.2f
               };
            }

            for (auto pos_current = 0u; pos_current < circle_positions.size(); pos_current++)
            {
               auto pos_next = pos_current + 1;
               if (pos_next == circle_positions.size())
               {
                  pos_next = 0;
               }

               auto v0 = circle_positions[pos_current];
               auto v1 = circle_positions[pos_next];

               auto v0far = 10000.0f * (v0 - light_pos_m);
               auto v1far = 10000.0f * (v1 - light_pos_m);

               sf::Vertex quad[] =
               {
                  sf::Vertex(sf::Vector2f(v0.x, v0.y) * PPM, sf::Color::Black),
                  sf::Vertex(sf::Vector2f(v0far.x, v0far.y) * PPM, sf::Color::Black),
                  sf::Vertex(sf::Vector2f(v1far.x, v1far.y) * PPM, sf::Color::Black),
                  sf::Vertex(sf::Vector2f(v1.x, v1.y) * PPM, sf::Color::Black)
               };

               target.draw(quad, 4, sf::Quads);
            }
         }
         else if (shape_chain)
         {
            // for now it is assumed that chainshapes are static objects only.
            // therefore no transform is applied to chainshape based objects.

            for (auto pos_current = 0; pos_current < shape_chain->m_count; pos_current++)
            {
               auto pos_next = pos_current + 1;
               if (pos_next == shape_chain->m_count)
               {
                  pos_next = 0;
               }

               auto v0 = shape_chain->m_vertices[pos_current];
               auto v1 = shape_chain->m_vertices[pos_next];

               // printf("%f\n", (lightPos - v0).LengthSquared());

               if (
                     (light_pos_m - v0).LengthSquared() > max_distance_m2
                  && (light_pos_m - v1).LengthSquared() > max_distance_m2
               )
               {
                  continue;
               }

               auto v0_far = 10000.0f * (v0 - light_pos_m);
               auto v1_far = 10000.0f * (v1 - light_pos_m);

               sf::Vertex quad[] =
               {
                  sf::Vertex(sf::Vector2f(v0.x, v0.y) * PPM, sf::Color::Black),
                  sf::Vertex(sf::Vector2f(v0_far.x, v0_far.y) * PPM, sf::Color::Black),
                  sf::Vertex(sf::Vector2f(v1_far.x, v1_far.y) * PPM, sf::Color::Black),
                  sf::Vertex(sf::Vector2f(v1.x, v1.y) * PPM, sf::Color::Black)
               };

               target.draw(quad, 4, sf::Quads);
            }
         }
         else if (shape_polygon)
         {
            for (auto pos_current = 0; pos_current < shape_polygon->GetVertexCount(); pos_current++)
            {
               auto pos_next = pos_current + 1;
               if (pos_next == shape_polygon->GetVertexCount())
               {
                  pos_next = 0;
               }

               auto v0 = shape_polygon->GetVertex(pos_current) + b->GetTransform().p;

               // printf("%f\n", (lightPos - v0).LengthSquared());

               if ((light_pos_m - v0).LengthSquared() > max_distance_m2)
                  continue;

               auto v1 = shape_polygon->GetVertex(pos_next) + b->GetTransform().p;
               auto v0far = 10000.0f * (v0 - light_pos_m);
               auto v1far = 10000.0f * (v1 - light_pos_m);

               sf::Vertex quad[] =
               {
                  sf::Vertex(sf::Vector2f(v0.x, v0.y) * PPM, sf::Color::Black),
                  sf::Vertex(sf::Vector2f(v0far.x, v0far.y) * PPM, sf::Color::Black),
                  sf::Vertex(sf::Vector2f(v1far.x, v1far.y) * PPM, sf::Color::Black),
                  sf::Vertex(sf::Vector2f(v1.x, v1.y) * PPM, sf::Color::Black)
               };

               target.draw(quad, 4, sf::Quads);
            }
         }
      }
   }
}


//-----------------------------------------------------------------------------
sf::Vector2f mapCoordsToPixelNormalized(const sf::Vector2f& point, const sf::View& view)
{
    // first, transform the point by the view matrix
    sf::Vector2f normalized = view.getTransform().transformPoint(point);

    // then convert to viewport coordinates
    sf::Vector2f pixel;

    pixel.x = ( normalized.x + 1.0f) / 2.0f;
    pixel.y = (-normalized.y + 1.0f) / 2.0f;

    return pixel;
}


//-----------------------------------------------------------------------------
sf::Vector2f mapCoordsToPixelScreenDimension(sf::RenderTarget& target, const sf::Vector2f& point, const sf::View& view)
{
    // first, transform the point by the view matrix
    sf::Vector2f normalized = view.getTransform().transformPoint(point);

    // then convert to viewport coordinates
    sf::Vector2f pixel;
    const auto viewport = target.getViewport(view);
    pixel.x = ( normalized.x + 1.0f) / 2.0f * static_cast<float>(viewport.width) + static_cast<float>(viewport.left);
    pixel.y = (-normalized.y + 1.0f) / 2.0f * static_cast<float>(viewport.height) + static_cast<float>(viewport.top);

    return pixel;
}


//-----------------------------------------------------------------------------
void LightSystem::updateLightShader(sf::RenderTarget& target)
{
   int32_t light_id = 0;

   _light_shader.setUniform(
      "u_light_count",
      static_cast<int32_t>(_active_lights.size())
   );

   _light_shader.setUniform(
      "u_resolution",
      sf::Glsl::Vec2(
         static_cast<float>(target.getSize().x),
         static_cast<float>(target.getSize().y)
      )
   );

   _light_shader.setUniform(
      "u_ambient",
      sf::Glsl::Vec4(
         _ambient_color[0],
         _ambient_color[1],
         _ambient_color[2],
         _ambient_color[3]
      )
   );

   for (auto& light : _active_lights)
   {
      std::string id = "u_lights[" + std::to_string(light_id) + "]";

      // transform light coordinates from box2d to normalized screen coordinates
      sf::Vector2f light_screen_pos = mapCoordsToPixelNormalized(
         {
            light->_pos_m.x * PPM + light->_center_offset_px.x,
            light->_pos_m.y * PPM + light->_center_offset_px.y
         },
         *Level::getCurrentLevel()->getLevelView().get()
      );

      _light_shader.setUniform(
         id + "._position",
         sf::Glsl::Vec3(
            static_cast<float>(light_screen_pos.x),
            static_cast<float>(1.0f - light_screen_pos.y),
            0.075f // default z
         )
      );

      _light_shader.setUniform(
         id + "._color",
         sf::Glsl::Vec4(
            static_cast<float>(light->_color.r) / 255.0f,
            static_cast<float>(light->_color.g) / 255.0f,
            static_cast<float>(light->_color.b) / 255.0f,
            static_cast<float>(light->_color.a) / 255.0f
         )
      );

      _light_shader.setUniform(
         id + "._falloff",
         sf::Glsl::Vec3(
            light->_falloff[0],
            light->_falloff[1],
            light->_falloff[2]
         )
      );

      // std::cout
      //    << "light position on screen "
      //    << id << ": "
      //    << light_screen_pos.x << ", "
      //    << light_screen_pos.y << " | "
      //    << "render target size is: "
      //    << target.getSize().x << ", "
      //    << target.getSize().y
      //    << std::endl;

      light_id++;
   }
}


//-----------------------------------------------------------------------------
void LightSystem::draw(sf::RenderTarget& target, sf::RenderStates /*states*/) const
{
   _active_lights.clear();

   auto player_body = Player::getCurrent()->getBody();

   for (const auto& light : _lights)
   {
      // don't draw lights that are too far away
      auto distanceToPlayer = (player_body->GetWorldCenter() - light->_pos_m).LengthSquared();

      if (distanceToPlayer > max_distance_m2)
      {
         continue;
      }

      _active_lights.push_back(light);

      // fill stencil buffer
      glClear(GL_STENCIL_BUFFER_BIT);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glEnable(GL_STENCIL_TEST);
      glStencilFunc(GL_ALWAYS, 1, 1);
      glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

      drawShadowQuads(target, light);

      // draw light quads with stencil boundaries
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glStencilFunc(GL_EQUAL, 0, 1);
      glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

      sf::RenderStates lightRs{sf::BlendAdd};
      target.draw(light->_sprite, lightRs);
   }

   glDisable(GL_STENCIL_TEST);

   // std::cout << _active_lights.size() << " active light sources " << std::endl;
}


//-----------------------------------------------------------------------------
void LightSystem::draw(
   sf::RenderTarget& target,
   const std::shared_ptr<sf::RenderTexture>& color_map,
   const std::shared_ptr<sf::RenderTexture>& light_map,
   const std::shared_ptr<sf::RenderTexture>& normal_map
)
{
   // MOVE THIS IN FUNCTION BELOW
   _light_shader.setUniform("color_map", color_map->getTexture());
   _light_shader.setUniform("light_map", light_map->getTexture());
   _light_shader.setUniform("normal_map", normal_map->getTexture());

   // update shader uniforms
   updateLightShader(target);

   sf::Sprite sprite;
   sprite.setTexture(color_map->getTexture());
   target.draw(sprite, &_light_shader);
}


//-----------------------------------------------------------------------------
void LightSystem::LightInstance::updateSpritePosition()
{
   _sprite.setPosition(
      sf::Vector2f(
         _pos_m.x * PPM - _width_px  * 0.5f,
         _pos_m.y * PPM - _height_px * 0.5f
      )
   );
}


//-----------------------------------------------------------------------------
std::shared_ptr<LightSystem::LightInstance> LightSystem::createLightInstance(TmxObject* tmx_object)
{
   auto light = std::make_shared<LightSystem::LightInstance>();

   std::array<uint8_t, 4> rgba = {255, 255, 255, 255};
   std::string texture = "data/light/smooth.png";

   if (tmx_object && tmx_object->_properties != nullptr)
   {
      auto it = tmx_object->_properties->_map.find("color");
      if (it != tmx_object->_properties->_map.end())
      {
         rgba = TmxTools::color(it->second->_value_string.value());
      }

      it = tmx_object->_properties->_map.find("texture");
      if (it != tmx_object->_properties->_map.end())
      {
         texture = (std::filesystem::path("data/light/") / it->second->_value_string.value()).string();
      }

      // A) center of the physical light is in the center of the textured quad
      //
      //   +----+----+
      //   |    |    |
      //   |   \|/   |
      //   +----+----+
      //   |   /|\   |
      //   |    |    |
      //   +----+----+
      //
      // B) center of the phyisical light is not in the center of the textured quad
      //    but has some offset to it. here the center is, say -24px, higher
      //
      //   +----+----+
      //   |   \|/   |
      //   +----+----+ center_offset_x_px = 0
      //   |   /|\   | center_offset_y_px= -24
      //   |    |    |
      //   |    |    |
      //   +----+----+

      // read offset
      it = tmx_object->_properties->_map.find("center_offset_x_px");
      if (it != tmx_object->_properties->_map.end())
      {
         light->_center_offset_px.x = it->second->_value_int.value();
         light->_center_offset_m.x = it->second->_value_int.value() * MPP;
      }

      it = tmx_object->_properties->_map.find("center_offset_y_px");
      if (it != tmx_object->_properties->_map.end())
      {
         light->_center_offset_px.y = it->second->_value_int.value();
         light->_center_offset_m.y = it->second->_value_int.value() * MPP;
      }

      // read falloff
      //
      // constant falloff:   basically ambient light amount
      // linear falloff:    light = 1 / distance to light
      // quadratic falloff: light = 1 / (distance to light)^2
      //
      //                                                                1.0
      // attenuation = --------------------------------------------------------------------------------------------
      //                (constant falloff + (linear falloff * distance) + (quadratic falloff * distane * distance))
      //
      it = tmx_object->_properties->_map.find("falloff_constant");
      if (it != tmx_object->_properties->_map.end())
      {
         light->_falloff[0] = it->second->_value_float.value();
      }

      it = tmx_object->_properties->_map.find("falloff_linear");
      if (it != tmx_object->_properties->_map.end())
      {
         light->_falloff[1] = it->second->_value_float.value();
      }

      it = tmx_object->_properties->_map.find("falloff_quadratic");
      if (it != tmx_object->_properties->_map.end())
      {
         light->_falloff[2] = it->second->_value_float.value();
      }
   }

   if (tmx_object)
   {
      light->_width_px  = static_cast<int>(tmx_object->_width_px);
      light->_height_px = static_cast<int>(tmx_object->_height_px);

      // set up the box2d position of the light
      light->_pos_m = b2Vec2(
         tmx_object->_x_px * MPP + (tmx_object->_width_px  * 0.5f) * MPP,
         tmx_object->_y_px * MPP + (tmx_object->_height_px * 0.5f) * MPP
      );
   }

   light->_color.r = rgba[0];
   light->_color.g = rgba[1];
   light->_color.b = rgba[2];
   light->_color.a = rgba[3];

   // for now the sprite color is left white since it'll be used as attenuation value in the light shader
   //
   // light->_sprite.setColor(light->_color);

   light->_texture = TexturePool::getInstance().get(texture);
   light->_sprite.setTexture(*light->_texture);
   light->_sprite.setTextureRect(
      sf::IntRect(
         0,
         0,
         static_cast<int32_t>(light->_texture->getSize().x),
         static_cast<int32_t>(light->_texture->getSize().y)
      )
   );

   light->updateSpritePosition();

   auto scale = static_cast<float>(light->_width_px) / light->_texture->getSize().x;
   light->_sprite.setScale(scale, scale);

   return light;
}


void LightSystem::increaseAmbient(float amount)
{
   for (auto& val : _ambient_color)
   {
      val += amount;
   }
}


void LightSystem::decreaseAmbient(float amount)
{
   for (auto& val : _ambient_color)
   {
      val -= amount;
   }
}


