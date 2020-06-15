#include "playerclimb.h"

#include "audio.h"
#include "savestate.h"

namespace
{

class PlayerAABBQueryCallback : public b2QueryCallback
{
   public:
      std::set<b2Body*> mBodies;

   public:
      bool ReportFixture(b2Fixture* fixture)
      {
         // foundBodies.push_back(fixture->GetBody());
         mBodies.insert(fixture->GetBody());

         // keep going to find all fixtures in the query area
         return true;
      }
};

}


//----------------------------------------------------------------------------------------------------------------------
void PlayerClimb::update(b2Body* playerBody, int32_t keysPressed, bool inAir)
{
   if (!(SaveState::getPlayerInfo().mExtraTable.mSkills.mSkills & ExtraSkill::SkillClimb))
   {
      return;
   }

   mKeysPressed = keysPressed;

   // http://www.iforce2d.net/b2dtut/world-querying

//   // player must indicate the direction he wants to go to
//   bool rightPressed = mKeysPressed & KeyPressedRight;
//   bool leftPressed = mKeysPressed & KeyPressedLeft;
//   if (!(leftPressed || rightPressed))
//   {
//      removeJoint();
//      return;
//   }

   // if player is standing somewhere, remove joint
   if (!inAir)
   {
      removeClimbJoint();
      return;
   }

   // remove that joint if player is moving 'up'
   if (playerBody->GetLinearVelocity().y < -0.51f)
   {
      removeClimbJoint();
      return;
   }

   if (mClimbJoint != nullptr)
   {
      return;
   }


   // hold
   if (mClimbJoint != nullptr)
   {
      // remove that joint if it points down from the player perspective
      auto jointDir = (mClimbJoint->GetAnchorA() - mClimbJoint->GetAnchorB());
      if (jointDir.y < -0.1f)
      {
         removeClimbJoint();
      }
      // printf("joint dir: %f \n", jointDir.y);

      return;
   }

   // TODO: generate accurate aabb to simulate player 'hands'
   //   float smallerX = 0.0f;
   //   float smallerY = 0.0f;
   //   float largerX = 0.0f;
   //   float largerY = 0.0f;
   //
   //   b2Vec2 lower(smallerX, smallerY);
   //   b2Vec2 upper(largerX, largerY);
   //
   //   b2AABB aabb;
   //   aabb.lowerBound = lower;
   //   aabb.upperBound = upper;

   // query nearby region
   PlayerAABBQueryCallback queryCallback;


   // player aabb is: 19.504843, 158.254532 to 19.824877 158.966980
   // x: 0.320034 * 0.5 -> 0.1600171
   // y: 0.712448 * 0.5 -> 0.356224
   b2AABB aabb;
   float w = 0.1600171f;
   float h = 0.356224f;
   b2Vec2 center = playerBody->GetWorldCenter();
   aabb.lowerBound = b2Vec2(center.x - w, center.y - h);
   aabb.upperBound = b2Vec2(center.x + w, center.y + h);

//   b2AABB aabb;
//   aabb.lowerBound = b2Vec2(FLT_MAX,FLT_MAX);
//   aabb.upperBound = b2Vec2(-FLT_MAX,-FLT_MAX);
//   auto fixture = mBody->GetFixtureList();
//   while (fixture != nullptr)
//   {
//       aabb.Combine(aabb, fixture->GetAABB(0));
//       fixture = fixture->GetNext();
//   }

   // printf("player aabb is: %f, %f to %f %f\n", aabb.lowerBound.x, aabb.lowerBound.y, aabb.upperBound.x, aabb.upperBound.y);

   playerBody->GetWorld()->QueryAABB(&queryCallback, aabb);

   // std::remove_if(queryCallback.foundBodies.begin(), queryCallback.foundBodies.end(), [this](b2Body* body){return body == mBody;});
   queryCallback.mBodies.erase(playerBody);

   // printf("bodies in range:\n");
   for (auto body : queryCallback.mBodies)
   {
      if (body->GetType() == b2_staticBody)
      {
         // printf("- static body: %p\n", body);

         auto found = false;
         auto distMax = 0.16f;
         auto fixture = body->GetFixtureList();
         while (fixture)
         {
            auto shape = dynamic_cast<b2ChainShape*>(fixture->GetShape());

            if (shape)
            {
               b2Vec2 closest;

               auto edgeLengthMinimum = 1000.0f;
               for (auto index = 0; index < shape->m_count; index++)
               {
                  auto curr = shape->m_vertices[index];
                  auto edgeLengthSquared = (aabb.GetCenter() - curr).LengthSquared();

                  if (edgeLengthSquared >= distMax)
                  {
                     continue;
                  }

                  // does player point to edge direction?
                  if (!edgeMatchesMovement(aabb.GetCenter() - curr))
                  {
                     continue;
                  }

                  // joint in spe needs to point up, since we're holding somewhere4
                  auto jointDir = (aabb.GetCenter() - curr);
                  if  (jointDir.y <= 0.0f)
                  {
                     continue;
                  }

                  // is the analyzed edge actually climbable?
                  if (!isClimbableEdge(shape, index))
                  {
                     continue;
                  }

                  // printf("joint dir: %f \n", jointDir.y);

                  // if all that is the case, we have a joint
                  if (edgeLengthSquared < edgeLengthMinimum)
                  {
                     edgeLengthMinimum = edgeLengthSquared;
                     closest = curr;
                     found = true;
                  }
               }

               // printf("    - closest vtx: %f, %f, distance: %f\n", closest.x, closest.y, distMin);

               // situation a)
               //
               //    v
               //    +------+ prev/next, same y, greater x
               //    |
               //    |
               //    +
               //  prev/next
               //
               //  same x, greater y
               //
               //
               // situation b)
               //                                        v
               //    prev/next, same y, smaller x +------+
               //                                        |
               //                                        |
               //                                        +
               //                                      prev/next
               //
               //                                      same x, greater y
               //
               //
               // prev or current needs to have greater y
               // other vertex must have different x

               if (found)
               {
                  b2DistanceJointDef jointDefinition;
                  jointDefinition.Initialize(playerBody, body, aabb.GetCenter(), closest);
                  jointDefinition.collideConnected = true;
                  // jointDefinition.dampingRatio = 0.5f;
                  // jointDefinition.frequencyHz = 5.0f;
                  // jointDefinition.length = 0.01f;

                  Audio::getInstance()->playSample("impact.wav");
                  mClimbJoint = playerBody->GetWorld()->CreateJoint(&jointDefinition);
               }

               // no need to continue processing
               break;
            }

            fixture = fixture->GetNext();
         }
      }
   }

   // mDistanceJoint->SetLength(distanceJoint.GetLength() * 0.99f);
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerClimb::removeClimbJoint()
{
   if (mClimbJoint)
   {
      mClimbJoint->GetBodyA()->GetWorld()->DestroyJoint(mClimbJoint);
      mClimbJoint = nullptr;
   }
}



//----------------------------------------------------------------------------------------------------------------------
bool PlayerClimb::isClimbableEdge(b2ChainShape* shape, int i)
{
   /*
      climbable edges:

         c     n             c     p
         x-----+             x-----+
         |     |             |     |
         |     |             |     |
      p  +-----+ nn       n  +-----+ pp
         |     |             |     |
         |     |             |     |
      pp +-----+          nn +-----+

               p.y > c.y
            && n.x != c.x
            && pp.y > p.y

                n.y > c.y
            && p.x != c.x
            && nn.y > n.y


      c     n     nn      c     p     pp     pp    p     c      nn    n     c
      x-----+-----+       x-----+-----+      +-----+-----x      +-----+-----x
      |     |     |       |     |     |      |     |     |      |     |     |
      |     |     |       |     |     |      |     |     |      |     |     |
    p +-----+-----+     n +-----+-----+      +-----+-----+ n    +-----+-----+ p
            pp                  nn                 nn                pp

               p.y > c.y
            && n.x != c.x
            && pp.x == n.x

               n.y > c.y
            && n.x != c.x
            && p.x == nn.x


     example:

         pp: 45.5; 158.5
         p:  45.0; 158.5
         c:  44.5; 158.5
         n:  44.5; 159.0
         nn: 45.0; 159.0

         c	   p     pp
         44.5  45.0  45.5
         158.5 158.5 158.5
         +-----+-----+-----+
         |     |     |
         |     |     |
         +-----+-----+-----+
         n     nn
         44.5  45.0
         159.0 159.0

   */

   // the last vertex of a shape is duplicated for some weird reson.
   // probably needs some more investigation. for now it's just not taken into regard
   shape->m_count--;

   auto index = [shape](int i) -> int {
      if (i >= shape->m_count)
      {
         return i - shape->m_count;
      }
      if (i < 0)
      {
         return shape->m_count + i;
      }
      return i;
   };

   auto pp = shape->m_vertices[index(i - 2)];
   auto p = shape->m_vertices[index(i - 1)];
   auto c = shape->m_vertices[i];
   auto n = shape->m_vertices[index(i + 1)];
   auto nn = shape->m_vertices[index(i + 2)];

   shape->m_count++;

   auto climbable =
         (p.y > c.y && (fabs(n.x - c.x) > 0.001f) && pp.y > p.y)
      || (n.y > c.y && (fabs(p.x - c.x) > 0.001f) && nn.y > n.y)
      || (p.y > c.y && (fabs(n.x - c.x) > 0.001f) && (fabs(pp.x - n.x) < 0.0001f))
      || (n.y > c.y && (fabs(p.x - c.x) > 0.001f) && (fabs(p.x - nn.x) < 0.0001f));

//   if (!climbable)
//   {
//      if (shape->m_count < 100)
//      {
//         for (int i = 0; i < shape->m_count; i++)
//         {
//            printf("shape v(%d): %f, %f\n", i, shape->m_vertices[i].x, shape->m_vertices[i].y);
//         }
//      }
//      printf("boo\n");
//   }

   return climbable;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerClimb::edgeMatchesMovement(const b2Vec2& edgeDir)
{
   bool rightPressed = mKeysPressed & KeyPressedRight;
   bool leftPressed = mKeysPressed & KeyPressedLeft;

   auto matchesMovement = false;
   // auto edgeType = Edge::None;

   if (edgeDir.x < -0.01f)
   {
      // edgeType = Edge::Right;
      matchesMovement = rightPressed;
   }
   else if (edgeDir.x > 0.01f)
   {
      // edgeType = Edge::Left;
      matchesMovement = leftPressed;
   }

   return matchesMovement;
}



//----------------------------------------------------------------------------------------------------------------------
bool PlayerClimb::isClimbing() const
{
   return mClimbJoint != nullptr;
}
