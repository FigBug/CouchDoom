#pragma once

#include <array>
#include <memory>

#include <juce_core/juce_core.h>
#include <gin_doom/gin_doom.h>

//==============================================================================
// Owns the four Doom instances ("four computers"). Each is a fully independent
// simulation (its own data_t) running on its own thread. Player 0 renders
// music; players 1-3 are SFX-only.
//
// For now the four run as independent games; the fake-network arbiter (next
// phase) synchronises them into a single deathmatch.
//==============================================================================
class DoomHost
{
public:
    static constexpr int kNumPlayers = 4;

    DoomHost();
    ~DoomHost();

    // Launch numPlayers instances on the given IWAD file (player 0 with music).
    // NOTE: numPlayers > 1 is blocked until the WAD subsystem (w_wad.c
    // lumpinfo/numlumps) is moved into data_t — today it is a global that all
    // instances would clobber. Start 1 for now; flip to 4 once that lands.
    void start (const juce::File& wad, int numPlayers = kNumPlayers);

    // Latest framebuffer for a player (640x400), for the 2x2 grid.
    juce::Image getScreen (int player);

    // The player's audio engine, for the mixer.
    gin::DoomAudioEngine* audioEngine (int player);

    // Feed a key event to one player's instance (temporary keyboard path;
    // replaced by controllers).
    void postKey (int player, int key, bool down);

    static constexpr int count() { return kNumPlayers; }

private:
    std::array<std::unique_ptr<gin::Doom>, kNumPlayers> instances;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DoomHost)
};
