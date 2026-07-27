#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
// Top-level component for CouchDoom.
//
// This is the shell of the app: a 60 Hz driver that will own the four Doom
// instances (via game::DoomHost), the fake-network arbiter that keeps them in
// lockstep, the audio mixer that sums their four output streams, and the
// gamepad manager that feeds per-instance input. Those subsystems are wired in
// as they are built (see README.md for the phased plan); for now this is a
// placeholder window so the project configures, builds and runs.
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

    static constexpr int kTickHz       = 60;
    static constexpr int kDoomWidth    = 640;   // doomgeneric framebuffer size
    static constexpr int kDoomHeight   = 400;
    static constexpr int kNumInstances = 4;     // 2x2 split-screen deathmatch

    juce::ScopedLowPowerModeDisabler keepAwake;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
