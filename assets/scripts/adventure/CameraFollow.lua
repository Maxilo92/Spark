-- CameraFollow.lua
-- Smoothly follows the player

local targetName = "Player"
local targetID = 0
local smoothSpeed = 0.1 -- 10% movement per frame
local verticalOffset = 2.0

function OnUpdate(dt)
    -- Find player once
    if targetID == 0 then
        targetID = Internal_FindEntityByName(targetName)
    end

    if targetID ~= 0 then
        local targetT = Internal_GetTransform(targetID)
        local ownT = Internal_GetTransform(entityID)
        
        if targetT and ownT then
            local targetPos = targetT.Translation
            local currentPos = ownT.Translation
            
            -- Manual Lerp for smooth camera
            local newX = currentPos.x + (targetPos.x - currentPos.x) * smoothSpeed
            local newY = currentPos.y + (targetPos.y + verticalOffset - currentPos.y) * smoothSpeed
            
            ownT.Translation = vec3(newX, newY, currentPos.z)
        end
    end
end
