-- Player.lua - The Spark Adventure
-- Controls the player character with physics-based movement

local speed = 10.0 -- Increased speed for better visibility
local jumpImpulse = 15.0
local isGrounded = false
local health = 3
local score = 0

-- Fallback Keys in case global table is missing
local Keys = Keys or {
    W = 87, S = 83, A = 65, D = 68, Space = 32,
    Left = 263, Right = 262, Up = 265, Down = 264
}

function OnStart()
    Log("Player initialized! EntityID: " .. entityID)
end

function OnUpdate(dt)
    local rb = Internal_GetRigidbody(entityID)
    if not rb then return end

    local currentVel = rb:GetLinearVelocity()
    local moveX = 0

    -- Horizontal Movement (Checking both Letters and Arrows)
    if Input.IsKeyPressed(Keys.A) or Input.IsKeyPressed(Keys.Left) then
        moveX = -speed
    elseif Input.IsKeyPressed(Keys.D) or Input.IsKeyPressed(Keys.Right) then
        moveX = speed
    end

    -- Jumping
    if (Input.IsKeyPressed(Keys.Space) or Input.IsKeyPressed(Keys.W) or Input.IsKeyPressed(Keys.Up)) and isGrounded then
        rb:ApplyLinearImpulse(vec2(0, jumpImpulse), true)
        isGrounded = false
        Log("JUMP!")
        
        local audio = Internal_GetAudio(entityID)
        if audio then audio:Play() end
    end

    -- Apply horizontal velocity while keeping vertical (falling) velocity
    -- We use a slight lerp for smoother movement if needed, but direct is fine for a demo
    rb:SetLinearVelocity(vec2(moveX, currentVel.y))

    -- Reset on fall
    local pos = rb:GetPosition()
    if pos.y < -15 then
        rb:SetTransform(vec2(0, 5))
        rb:SetLinearVelocity(vec2(0, 0))
        Log("Player fell out of world! Resetting...")
    end
end

-- Collision handling
function OnCollisionBegin(otherID)
    local otherTag = Internal_GetTag(otherID)
    
    -- Ground check
    if otherTag == "Ground" or otherTag == "Platform" then
        isGrounded = true
    end

    -- Enemy hit
    if otherTag == "Enemy" then
        health = health - 1
        Log("Hit by enemy! Health: " .. health)
        
        local rb = Internal_GetRigidbody(entityID)
        if rb then
            rb:ApplyLinearImpulse(vec2(0, 10), true)
        end

        if health <= 0 then
            Log("Game Over!")
            rb:SetTransform(vec2(0, 5))
            health = 3
        end
    end
end

function AddScore(amount)
    score = score + amount
    Log("Score: " .. score)
end
