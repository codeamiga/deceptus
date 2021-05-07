#include "tmxpolyline.h"

#include <sstream>
#include "tmxtools.h"

void TmxPolyLine::deserialize(tinyxml2::XMLElement *element)
{
   const auto points = element->Attribute("points");
   const auto pairs = TmxTools::split(points, ' ');

   for (auto& pair : pairs)
   {
      const auto two_points = TmxTools::splitPair(pair, ',');
      if (two_points.size() == 2)
      {
         const auto x = std::stof(two_points.at(0));
         const auto y = std::stof(two_points.at(1));

         _polyline.push_back(sf::Vector2f(x, y));
      }
      else
      {
         printf("bad polygon data\n");
         break;
      }
   }
}
