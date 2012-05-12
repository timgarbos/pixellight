#pragma once

#include <fmod.hpp>
#include "fmod_errors.h"

class AudioChannel 
{
public:
    AudioChannel(FMOD::System *system, const char *name);
    void playSound();
    void setVolume(float volume);
    void setVolumeTarget(float volume_target, float speed);
    void update(float deltaTime);
    void stop();
    
private:
    FMOD::System *system;
    FMOD::Sound* sound;
    FMOD::Channel* channel;
    
    float volume;
    
    float volume_target;
    float volume_tolerance;
    float volume_change_speed;
};
