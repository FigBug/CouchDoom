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

    // Game input is read by polling (isKeyCurrentlyDown), so consume key events
    // here to stop the OS "unhandled key" beep. As the ancestor of TitleScreen
    // this also swallows keys the lobby menu doesn't use.
    bool keyPressed (const juce::KeyPress&) override    { return true; }
    bool keyStateChanged (bool /*isKeyDown*/) override  { return true; }

private:
    enum class State { Title, Playing };

    void timerCallback() override;
    void startMatch (const GameConfig& config);
    void returnToMenu();
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
    juce::File                       wad;

    std::set<int> kbDown;   // Doom keys currently held on the keyboard (player 0)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
