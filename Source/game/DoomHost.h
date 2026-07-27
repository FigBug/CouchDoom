#pragma once

#include <array>
#include <memory>

#include <juce_core/juce_core.h>
#include <gin_doom/gin_doom.h>

#include "GameConfig.h"

//==============================================================================
// Owns the four Doom instances ("four computers"). Each is a fully independent
// simulation (its own data_t) running on its own thread, kept in lockstep by
// the fake-network arbiter (couch.c) into one deathmatch/co-op session. Player
// 0 renders music; players 1-3 are SFX-only.
//
// All four slots always run as real Doom players. Slots without a controller
// are simply idle ("AI" on the title screen) until real bot AI is added.
//==============================================================================
class DoomHost
{
public:
    static constexpr int kNumPlayers = 4;

    DoomHost();
    ~DoomHost();

    // Launch the four instances for one session with the given match settings.
    void start (const juce::File& wad, const GameConfig& config);

    // True once start() has been called (a session is running).
    bool isRunning() const { return running; }

    // Latest framebuffer for a player (640x400), for the 2x2 grid.
    juce::Image getScreen (int player);

    // The player's audio engine, for the mixer.
    gin::DoomAudioEngine* audioEngine (int player);

    // Feed a Doom KEY_* event to one player's instance (from controllers /
    // keyboard). Ignored if no session is running.
    void postKey (int player, int key, bool down);

    static constexpr int count() { return kNumPlayers; }

private:
    std::array<std::unique_ptr<gin::Doom>, kNumPlayers> instances;
    bool running = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DoomHost)
};
