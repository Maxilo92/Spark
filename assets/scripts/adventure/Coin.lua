-- Coin.lua
-- Handles collection by the player

function OnCollisionBegin(otherID)
    local otherTag = Internal_GetTag(otherID)
    
    if otherTag == "Player" then
        Log("Coin collected!")
        
        -- Play sound (on the player or global audio manager)
        local audio = Internal_GetAudio(entityID)
        if audio then
            audio:Play()
        end

        -- Destroy this coin
        Internal_DestroyEntity(entityID)
    end
end
