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

    // TEMP: run a single instance until the WAD subsystem is per-instance
    // (see DoomHost::start). Flip to DoomHost::count() for the full 2x2.
    static constexpr int kActivePlayers = 1;

    juce::ScopedLowPowerModeDisabler keepAwake;
    DoomHost                         doomHost;
    SoundEngine                      soundEngine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
