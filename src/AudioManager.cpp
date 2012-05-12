#include "AudioManager.h"

#include "utils.h"

using namespace std;

AudioManager::AudioManager()
{
    initialize_fmod();
    initialize_background_ambience();
    initialize_channels();

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
    background_ambience->setVolumeTarget(1.0f, 0.1f);
    
    channels.push_back(background_ambience);
    
}

void AudioManager::initialize_channels()
{
    channels.push_back(new AudioChannel(system, "audio/C.wav"));
    channels.push_back(new AudioChannel(system, "audio/D.wav"));
    channels.push_back(new AudioChannel(system, "audio/Fmaj_C.wav"));
    channels.push_back(new AudioChannel(system, "audio/G.wav"));
    channels.push_back(new AudioChannel(system, "audio/Am.wav"));
}

void AudioManager::update_channels(float deltaTime)
{
    for(vector<AudioChannel*>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        (*it)->update(deltaTime);
    }
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