#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "game/DoomHost.h"
#include "audio/SoundEngine.h"

//==============================================================================
// Top-level component: a 60 Hz driver that owns the Doom instances (DoomHost),
// mixes their audio (SoundEngine), and draws their framebuffers into a 2x2
// grid. The fake-network arbiter and controller input are layered on next.
//==============================================================================
class MainComponent : public juce::Component,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    juce::File extractWad();

    static constexpr int kTickHz     = 60;
    static constexpr int kDoomWidth  = 640;   // doomgeneric framebuffer size
    static constexpr int kDoomHeight = 400;

    // Full 2x2: four instances run and render concurrently. (Set to 1 to
    // debug a single instance.)
    static constexpr int kActivePlayers = DoomHost::count();

    juce::ScopedLowPowerModeDisabler keepAwake;
    DoomHost                         doomHost;
    SoundEngine                      soundEngine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
