#include "Components.h"
#include <box2d/box2d.h>
#include "miniaudio.h"

void Rigidbody2DComponent::ApplyLinearImpulse(const glm::vec2& impulse, bool wake)
{
    if (RuntimeBody)
    {
        static_cast<b2Body*>(RuntimeBody)->ApplyLinearImpulseToCenter({ impulse.x, impulse.y }, wake);
    }
}

void Rigidbody2DComponent::ApplyLinearImpulse(const glm::vec2& impulse, const glm::vec2& point, bool wake)
{
    if (RuntimeBody)
    {
        static_cast<b2Body*>(RuntimeBody)->ApplyLinearImpulse({ impulse.x, impulse.y }, { point.x, point.y }, wake);
    }
}

void Rigidbody2DComponent::SetLinearVelocity(const glm::vec2& velocity)
{
    if (RuntimeBody)
    {
        b2Body* body = static_cast<b2Body*>(RuntimeBody);
        body->SetLinearVelocity({ velocity.x, velocity.y });
        body->SetAwake(true);
    }
}

glm::vec2 Rigidbody2DComponent::GetLinearVelocity() const
{
    if (RuntimeBody)
    {
        const b2Vec2& vel = static_cast<b2Body*>(RuntimeBody)->GetLinearVelocity();
        return { vel.x, vel.y };
    }
    return { 0.0f, 0.0f };
}

void Rigidbody2DComponent::SetTransform(const glm::vec2& position)
{
    if (RuntimeBody)
    {
        b2Body* body = static_cast<b2Body*>(RuntimeBody);
        body->SetTransform({ position.x, position.y }, body->GetAngle());
        body->SetAwake(true);
    }
}

glm::vec2 Rigidbody2DComponent::GetPosition() const
{
    if (RuntimeBody)
    {
        const b2Vec2& pos = static_cast<b2Body*>(RuntimeBody)->GetPosition();
        return { pos.x, pos.y };
    }
    return { 0.0f, 0.0f };
}

void Rigidbody2DComponent::SetGravityScale(float scale)
{
    if (RuntimeBody)
    {
        static_cast<b2Body*>(RuntimeBody)->SetGravityScale(scale);
    }
}

float Rigidbody2DComponent::GetGravityScale() const
{
    if (RuntimeBody)
    {
        return static_cast<b2Body*>(RuntimeBody)->GetGravityScale();
    }
    return 1.0f;
}

void AudioSourceComponent::Play()
{
    if (RuntimeSound)
    {
        ma_sound_start(static_cast<ma_sound*>(RuntimeSound));
    }
}

void AudioSourceComponent::Stop()
{
    if (RuntimeSound)
    {
        ma_sound_stop(static_cast<ma_sound*>(RuntimeSound));
        ma_sound_seek_to_pcm_frame(static_cast<ma_sound*>(RuntimeSound), 0);
    }
}

bool AudioSourceComponent::IsPlaying() const
{
    if (RuntimeSound)
    {
        return ma_sound_is_playing(static_cast<ma_sound*>(RuntimeSound));
    }
    return false;
}

void AudioSourceComponent::SetVolume(float volume)
{
    Volume = volume;
    if (RuntimeSound)
    {
        ma_sound_set_volume(static_cast<ma_sound*>(RuntimeSound), volume);
    }
}

void AudioSourceComponent::SetPitch(float pitch)
{
    Pitch = pitch;
    if (RuntimeSound)
    {
        ma_sound_set_pitch(static_cast<ma_sound*>(RuntimeSound), pitch);
    }
}
