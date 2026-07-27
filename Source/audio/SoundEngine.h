#pragma once

#include <array>

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <gin_doom/gin_doom.h>

//==============================================================================
// Owns the audio device and mixes the four Doom instances' output into one
// stereo stream. Each gin::DoomAudioEngine::processBlock() *adds* its SFX (and,
// for player 0, music) into the buffer, so summing is just calling all four on
// the same (cleared) buffer.
//==============================================================================
class SoundEngine
{
public:
    static constexpr int kNumPlayers = 4;

    SoundEngine();
    ~SoundEngine();

    // Point the mixer at the four instances' engines (owned by DoomHost).
    void setEngines (std::array<gin::DoomAudioEngine*, kNumPlayers> engines);

private:
    struct Mixer : public juce::AudioSource
    {
        void prepareToPlay (int, double sampleRate) override { sr = sampleRate; }
        void releaseResources() override {}
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& info) override;

        juce::CriticalSection lock;
        std::array<gin::DoomAudioEngine*, kNumPlayers> engines { nullptr, nullptr, nullptr, nullptr };
        double sr        = 44100.0;
        float  masterGain = 0.8f;
    };

    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer  player;
    Mixer                    mixer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundEngine)
};
