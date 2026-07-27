#pragma once

#include <array>
#include <set>

#include <gin_controllers/gin_controllers.h>

#include "../game/DoomHost.h"

//==============================================================================
// Owns the gamepad manager (self-polls at 60 Hz on the message thread) and
// translates each controller into Doom input for the matching player. Port
// index maps 1:1 to player index: controller 0 => player 0 (top-left), etc.
//
// The title screen queries pad()/connected() for its per-quadrant labels and
// live button highlight; gameplay calls route() every tick to push edge-
// detected KEY_* events into the game.
//==============================================================================
class ControllerRouter
{
public:
    static constexpr int kNumPlayers = 4;

    ControllerRouter() = default;

    gin::GameControllerManager& getManager()       { return manager; }
    gin::GameController*         pad (int player);
    bool                        connected (int player);

    // Clear remembered held-key state (call when entering gameplay).
    void reset();

    // Poll every pad and post the resulting Doom key up/down events. Call once
    // per frame while a session is running.
    void route (DoomHost& host);

private:
    gin::GameControllerManager manager;

    std::array<std::set<int>, kNumPlayers> down;               // Doom keys held
    std::array<bool, kNumPlayers>          prevLB { false, false, false, false };
    std::array<bool, kNumPlayers>          prevRB { false, false, false, false };
    std::array<int,  kNumPlayers>          weapon { 1, 1, 1, 1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControllerRouter)
};
