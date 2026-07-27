#pragma once

#include <memory>
#include <set>

#include <juce_gui_basics/juce_gui_basics.h>

#include "game/DoomHost.h"
#include "game/GameConfig.h"
#include "audio/SoundEngine.h"
#include "input/ControllerRouter.h"
#include "view/TitleScreen.h"

//==============================================================================
// Top-level component and 60 Hz driver. Starts on the title screen (lobby);
// when the user starts a match it launches the four Doom instances (DoomHost),
// wires their audio into the mixer (SoundEngine), routes controllers to the
// game each tick, and draws the four framebuffers in a 2x2 grid.
//
// The keyboard is a permanent fallback for player 0 (WASD move, arrows turn,
// space fire, E use, shift run, 1-7 weapons) so the game is playable/testable
// without a physical controller.
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
    enum class State { Title, Playing };

    void timerCallback() override;
    void startMatch (const GameConfig& config);
    void routeKeyboardToPlayer0();
    juce::File extractWad();

    static constexpr int kTickHz     = 60;
    static constexpr int kDoomWidth  = 640;   // doomgeneric framebuffer size
    static constexpr int kDoomHeight = 400;

    State state = State::Title;

    juce::ScopedLowPowerModeDisabler keepAwake;
    ControllerRouter                 controllers;
    DoomHost                         doomHost;
    SoundEngine                      soundEngine;
    std::unique_ptr<TitleScreen>     title;
    juce::Slider                     masterSlider;   // on-screen master volume
    juce::File                       wad;

    std::set<int> kbDown;   // Doom keys currently held on the keyboard (player 0)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
