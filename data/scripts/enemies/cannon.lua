require "data/scripts/enemies/constants"
v2d = require "data/scripts/enemies/vectorial2"

-- place cannons here
-- 64,311
-- 78,311

------------------------------------------------------------------------------------------------------------------------
properties = {
   staticBody = true,
   sprite = "sprites/cannon.png",
   pointsUp = Alignment["AlignmentUp"],
   damage = 0
}


------------------------------------------------------------------------------------------------------------------------
mFireTimer = 1
mFireInterval = 3000
mFireReady = true
mPosition = v2d.Vector2D(0, 0)
mPlayerPosition = v2d.Vector2D(0, 0)


------------------------------------------------------------------------------------------------------------------------
function initialize()
   addShapeRect(0.2, 0.2, 0.0, 0.1) -- width, height, x, y
   updateSpriteRect(0, 0, 24, 24)    -- x, y, width, height
end


------------------------------------------------------------------------------------------------------------------------
function timeout(id)
   -- print(string.format("timeout: %d", id))
   if (id == mFireTimer) then
      fire()
   end
end


------------------------------------------------------------------------------------------------------------------------
function fire()
   print("boom.")
   mFireReady = true
end


------------------------------------------------------------------------------------------------------------------------
function retrieveProperties()
   updateProperties(properties)
end


------------------------------------------------------------------------------------------------------------------------
function movedTo(x, y)
   -- print(string.format("moved to: %f, %f", x, y))
   mPosition = v2d.Vector2D(x, y)
end


------------------------------------------------------------------------------------------------------------------------
function playerMovedTo(x, y)
   -- print(string.format("player moved to: %f, %f", x, y))
   mPlayerPosition = v2d.Vector2D(x, y)
end


------------------------------------------------------------------------------------------------------------------------
function update(dt)
   if (mFireReady == true) then
      mFireReady = false
      timer(mFireInterval, mFireTimer)
   end
end


------------------------------------------------------------------------------------------------------------------------
function setPath(name, table)
end

