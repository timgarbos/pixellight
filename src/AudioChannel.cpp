#include "AudioChannel.h"

#include <string>
#include <fmod.hpp>
#include "fmod_errors.h"
#include "fmod_utils.h"

AudioChannel::AudioChannel(FMOD::System *system, const char* name)
{
    this->system = system;
    
    ERRCHECK(system->createSound(name, FMOD_SOFTWARE, 0, &sound));
    
    sound->setMode(FMOD_LOOP_NORMAL);
 
    volume_tolerance = 0.001f;
    volume_change_speed = 0.2f;
    
    volume = 0.0f;
    volume_target = 0.0f;
}

void AudioChannel::playSound()
{
    system->playSound(FMOD_CHANNEL_FREE, sound, 0, &channel);
    channel->setVolume(volume);
}

void AudioChannel::setVolume(float volume)
{
    this->volume = volume;
    this->volume_target = volume_target;
    
    channel->setVolume(volume);
}

void AudioChannel::setVolumeTarget(float volume_target, float speed)
{
    this->volume_target = volume_target;
    this->volume_change_speed = speed;
}

void AudioChannel::update(float deltaTime)
{
    float diff = volume_target - volume;
    
    float dir = diff > 0.0f ? 1.0f : -1.0f;
    volume = volume + dir * volume_change_speed * deltaTime;
     
    channel->setVolume(volume);
}