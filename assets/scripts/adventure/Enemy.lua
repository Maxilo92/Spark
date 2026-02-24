-- Enemy.lua
-- Simple patrol enemy

local patrolSpeed = 3.0
local direction = 1
local maxRange = 5.0
local startX = 0

function OnStart()
    local rb = Internal_GetRigidbody(entityID)
    if rb then
        local pos = rb:GetPosition()
        startX = pos.x
    end
end

function OnUpdate(dt)
    local rb = Internal_GetRigidbody(entityID)
    if not rb then return end

    local currentPos = rb:GetPosition()
    
    -- Ping-Pong movement
    if currentPos.x > startX + maxRange then
        direction = -1
    elseif currentPos.x < startX - maxRange then
        direction = 1
    end

    rb:SetLinearVelocity(vec2(direction * patrolSpeed, 0))
end
