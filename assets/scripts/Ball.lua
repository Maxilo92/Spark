-- Ball Control Script
-- Test Command

local g_KeyPressed = false

function OnStart()
    Log("Ball Script Started!")
    if Input == nil then
        Log("Error: Input is nil!")
    else
        Log("Input is available.")
    end
end

function OnUpdate(dt)
    local rb = Internal_GetRigidbody(entityID)
    local transform = Internal_GetTransform(entityID)

    if not rb then return end

    -- W/A/S/D Steuerung (Impulse fuer sofortiges Feedback)
    -- Kraefte deutlich angepasst
    if Input.IsKeyPressed(87) then 
        rb:ApplyImpulse(vec2(0, 0.5))
    end
    if Input.IsKeyPressed(83) then 
        rb:ApplyImpulse(vec2(0, -0.5))
    end
    if Input.IsKeyPressed(65) then 
        rb:ApplyImpulse(vec2(-0.75, 0))
    end
    if Input.IsKeyPressed(68) then 
        rb:ApplyImpulse(vec2(0.75, 0))
    end
    
    -- Schwerkraft Toggle (G)
    -- Wenn G gedrueckt wird, drehen wir das Vorzeichen um
    if Input.IsKeyPressed(71) then 
        local currentScale = rb:GetGravityScale()
        -- Kleiner Hack: Wir warten kurz, damit es nicht in jedem Frame toggelt
        -- (Schoener waere IsKeyReleased, aber wir nutzen was wir haben)
        if not g_KeyPressed then
            rb:SetGravityScale(currentScale * -1.0)
            g_KeyPressed = true
            Log("Gravity Scale Toggled to: " .. tostring(currentScale * -1.0))
        end
    else
        g_KeyPressed = false
    end
end
