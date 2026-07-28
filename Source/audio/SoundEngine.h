#pragma once

#include <array>
#include <atomic>

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

    // Master output level (0..1), driven by the on-screen volume control.
    void  setMasterLevel (float level);
    float getMasterLevel() const { return mixer.masterGain.load(); }

    static constexpr float kDefaultMasterLevel = 0.8f;

private:
    struct Mixer : public juce::AudioSource
    {
        void prepareToPlay (int, double sampleRate) override { sr = sampleRate; }
        void releaseResources() override {}
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& info) override;

        juce::CriticalSection lock;
        std::array<gin::DoomAudioEngine*, kNumPlayers> engines { nullptr, nullptr, nullptr, nullptr };
        double sr        = 44100.0;
        std::atomic<float> masterGain { kDefaultMasterLevel };
        juce::AudioBuffer<float> sfxTemp;   // per-player SFX, before panning
    };

    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer  player;
    Mixer                    mixer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundEngine)
};
