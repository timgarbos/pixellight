#pragma once

#include <iostream>
#include <vector>

#include <GL/glfw.h>

#include <fmod.hpp>
#include "fmod_errors.h"

#include "fmod_utils.h"
#include "AudioChannel.h"

class AudioManager
{
public:
    AudioManager();
    ~AudioManager();
    
    AudioChannel *background_ambience;
    std::vector<AudioChannel*> channels;

    void update_channels(float deltaTime);
    
private:
    FMOD::System *system;
    
    void initialize_fmod();
    void initialize_background_ambience();
    void initialize_channels();
};