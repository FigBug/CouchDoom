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

    const int nch = view.getNumChannels();
    const int n   = info.numSamples;
    sfxTemp.setSize (juce::jmax (2, nch), n, false, false, true);

    // Pan each player's sound effects toward their screen quadrant's column
    // (2x2 grid: players 0,2 = left, 1,3 = right). Stereo out can only place
    // left/right, so top/bottom rows share a side. Music is left unpanned
    // (only player 0 has any) - the user wants it centred.
    constexpr float nearGain = 1.0f;   // channel on the player's side
    constexpr float farGain  = 0.4f;   // the opposite channel, attenuated

    for (size_t i = 0; i < engines.size(); ++i)
    {
        auto* e = engines[i];
        if (e == nullptr)
            continue;

        sfxTemp.clear();
        e->processSfx (sfxTemp, (int) sr);

        const bool  leftCol = (i == 0 || i == 2);
        const float gL = leftCol ? nearGain : farGain;
        const float gR = leftCol ? farGain  : nearGain;

        view.addFrom (0, 0, sfxTemp, 0, 0, n, gL);
        if (nch > 1)
            view.addFrom (1, 0, sfxTemp, 1, 0, n, gR);

        e->processMusic (view, (int) sr);   // centred, unpanned
    }

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
