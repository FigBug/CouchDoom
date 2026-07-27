#include "SoundEngine.h"

//==============================================================================
void SoundEngine::Mixer::getNextAudioBlock (const juce::AudioSourceChannelInfo& info)
{
    info.clearActiveBufferRegion();

    if (info.numSamples <= 0 || info.buffer == nullptr)
        return;

    // A view of just the active region, so each engine can treat it as a whole
    // buffer starting at sample 0.
    juce::AudioBuffer<float> view (info.buffer->getArrayOfWritePointers(),
                                   info.buffer->getNumChannels(),
                                   info.startSample,
                                   info.numSamples);

    juce::ScopedLock sl (lock);

    for (auto* e : engines)
        if (e != nullptr)
            e->processBlock (view, (int) sr);

    view.applyGain (masterGain.load());
}

//==============================================================================
SoundEngine::SoundEngine()
{
    deviceManager.initialiseWithDefaultDevices (0, 2);
    player.setSource (&mixer);
    deviceManager.addAudioCallback (&player);
}

SoundEngine::~SoundEngine()
{
    deviceManager.removeAudioCallback (&player);
    player.setSource (nullptr);
    deviceManager.closeAudioDevice();
}

void SoundEngine::setEngines (std::array<gin::DoomAudioEngine*, kNumPlayers> engines)
{
    juce::ScopedLock sl (mixer.lock);
    mixer.engines = engines;
}

void SoundEngine::setMasterLevel (float level)
{
    mixer.masterGain.store (juce::jlimit (0.0f, 1.0f, level));
}
