#include "AudioManager.h"

#include "utils.h"
#include <cmath>

using namespace std;

AudioManager::AudioManager()
{
    initialize_fmod();
    initialize_channels();
    initialize_background_ambience();

    for(vector<AudioChannel*>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        AudioChannel *audioChannel = *it;
        audioChannel->playSound();
    }   
}

void AudioManager::initialize_fmod()
{
    FMOD_RESULT result;
    unsigned int version;
    
    result = FMOD::System_Create(&system);
    ERRCHECK(result);
    
    result = system->getVersion(&version);
    ERRCHECK(result);
    
    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        return;
    }
    
    result = system->init(32, FMOD_INIT_NORMAL, NULL);
    ERRCHECK(result);
}

void AudioManager::initialize_background_ambience()
{
    background_ambience = new AudioChannel(system, "audio/white-noise.wav");
    background_ambience->playSound();
    
    evil = new AudioChannel(system, "audio/evil.wav");
    evil->playSound();
    evil->setVolumeTarget(1.0f, 1.0f/3.0f);
    
    channels.push_back(evil);
}

void AudioManager::initialize_channels()
{
    channels.push_back(new AudioChannel(system, "audio/C.wav"));
    channels.push_back(new AudioChannel(system, "audio/Fmaj_C.wav"));
    channels.push_back(new AudioChannel(system, "audio/Am.wav"));
    channels.push_back(new AudioChannel(system, "audio/D.wav"));
    channels.push_back(new AudioChannel(system, "audio/G.wav"));
}

const float BACKGROUND_AMBIENCE_FLOOR = 0.2f;

void AudioManager::update_channels(float deltaTime)
{
    for(vector<AudioChannel*>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        (*it)->update(deltaTime);
    }
    
    float pulse = sin(glfwGetTime()*5.0f + 0.15f);
    pulse = pulse > 0.0f ? pulse : 0.0f;
    
    background_ambience->setVolume((pulse * (1.0f - BACKGROUND_AMBIENCE_FLOOR) + BACKGROUND_AMBIENCE_FLOOR) * 0.1f) ;
}

AudioManager::~AudioManager()
{
    FMOD_RESULT result;

    for(vector<AudioChannel*>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        delete (*it);
    }    
    
    result = system->close();
    ERRCHECK(result);
    
    result = system->release();
    ERRCHECK(result);
}