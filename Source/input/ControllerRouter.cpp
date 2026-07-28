#include "ControllerRouter.h"

#include "DoomKeys.h"

using Button = gin::GameController::Button;
using Axis   = gin::GameController::Axis;

//==============================================================================
gin::GameController* ControllerRouter::pad (int player)
{
    if (! juce::isPositiveAndBelow (player, kNumPlayers))
        return nullptr;
    return manager.getController (player);
}

bool ControllerRouter::connected (int player)
{
    auto* p = pad (player);
    return p != nullptr && p->isConnected();
}

void ControllerRouter::reset()
{
    for (auto& s : down)   s.clear();
    for (auto& b : prevLB) b = false;
    for (auto& b : prevRB) b = false;
    for (auto& w : weaponKey)  w = 0;
    for (auto& w : weaponHold) w = 0;
}

void ControllerRouter::route (DoomHost& host)
{
    constexpr float dead = 0.5f;

    for (int p = 0; p < kNumPlayers; ++p)
    {
        const auto i = (size_t) p;
        auto* c = pad (p);
        std::set<int> now;

        // Release a held weapon key once its hold expires. The game reads
        // weapon keys by polling gamekeydown[] when it builds the ticcmd, and
        // it drains all queued events first - so a same-frame down+up cancels
        // out. Holding the key down a few frames guarantees it spans a tic.
        if (weaponHold[i] > 0 && --weaponHold[i] == 0 && weaponKey[i] != 0)
        {
            host.postKey (p, weaponKey[i], false);
            weaponKey[i] = 0;
        }

        if (c != nullptr && c->isConnected())
        {
            // gin's convention: forward/up = negative Y (see backends).
            const float lx = c->getAxis (Axis::leftX);
            const float ly = c->getAxis (Axis::leftY);
            const float rx = c->getAxis (Axis::rightX);

            auto b = [c] (Button btn) { return c->isButtonDown (btn); };

            // Twin-stick: left stick moves/strafes, right stick turns.
            if (ly < -dead || b (Button::dpadUp))    now.insert (dk::UPARROW);
            if (ly >  dead || b (Button::dpadDown))  now.insert (dk::DOWNARROW);
            if (lx < -dead)                          now.insert (dk::STRAFE_L);
            if (lx >  dead)                          now.insert (dk::STRAFE_R);
            if (rx < -dead || b (Button::dpadLeft))  now.insert (dk::LEFTARROW);
            if (rx >  dead || b (Button::dpadRight)) now.insert (dk::RIGHTARROW);

            if (b (Button::rightTrigger))            now.insert (dk::FIRE);
            if (b (Button::faceDown))                now.insert (dk::USE);    // A
            if (b (Button::leftTrigger))             now.insert (dk::RSHIFT); // run
            if (b (Button::start))                   now.insert (dk::ESCAPE);

            // Weapon cycle on bumper rising edge: RB = next owned weapon, LB =
            // previous. cycleWeaponKey skips weapons the player doesn't have.
            const bool lb = b (Button::leftShoulder);
            const bool rb = b (Button::rightShoulder);
            const bool bump = (rb && ! prevRB[i]) || (lb && ! prevLB[i]);
            if (bump)
            {
                // Release any still-held weapon key, then press the new one and
                // hold it for a few frames (see the release logic above).
                if (weaponKey[i]) { host.postKey (p, weaponKey[i], false); weaponKey[i] = 0; }

                int k = host.cycleWeaponKey (p, rb && ! prevRB[i]);
                if (k)
                {
                    host.postKey (p, k, true);
                    weaponKey[i]  = k;
                    weaponHold[i] = 6;   // ~100ms at 60Hz, comfortably spans a game tic
                }
            }
            prevLB[i] = lb;
            prevRB[i] = rb;
        }

        // Diff current held-key set against last frame -> up/down events.
        for (int k : now)
            if (down[i].find (k) == down[i].end())
                host.postKey (p, k, true);

        for (int k : down[i])
            if (now.find (k) == now.end())
                host.postKey (p, k, false);

        down[i] = std::move (now);
    }
}
