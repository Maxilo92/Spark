-- Ball Control Script
-- Direct Physics Translation

local g_KeyPressed = false

-- Key Constants (Fallback falls nicht vom Engine-Binding vorhanden)
local Keys = Keys or {
    W = 87,
    S = 83,
    A = 65,
    D = 68,
    G = 71
}

function OnUpdate(dt)
    local rb = Internal_GetRigidbody(entityID)
    if not rb then return end

    -- W/A/S/D Steuerung ueber direkte Positions-Updates im Physik-System
    local moveX = 0
    local moveY = 0
    local speed = 10.0 -- 10 Einheiten pro Sekunde

    if Input.IsKeyPressed(Keys.W) then moveY = 1 end
    if Input.IsKeyPressed(Keys.S) then moveY = -1 end
    if Input.IsKeyPressed(Keys.A) then moveX = -1 end
    if Input.IsKeyPressed(Keys.D) then moveX = 1 end
    
    -- Nur wenn wir eine Taste druecken, greifen wir ein
    if moveX ~= 0 or moveY ~= 0 then
        local currentPos = rb:GetPosition()
        local newX = currentPos.x + (moveX * speed * dt)
        local newY = currentPos.y + (moveY * speed * dt)
        
        rb:SetTransform(vec2(newX, newY))
        rb:SetLinearVelocity(vec2(0, 0)) -- Verhindert Kampf gegen die Gravitation beim Steuern
    end
    
    -- Schwerkraft Toggle (G)
    if Input.IsKeyPressed(Keys.G) then 
        if not g_KeyPressed then
            local currentScale = rb:GetGravityScale()
            rb:SetGravityScale(currentScale * -1.0)
            g_KeyPressed = true
            Log("Gravity Toggled")
        end
    else
        g_KeyPressed = false
    end
end
