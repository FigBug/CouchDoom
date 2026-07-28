#include "DoomHost.h"

// Lockstep arbiter (couch.c). Reset before a session, shut down (to break the
// barrier) before tearing instances down, and polled for the quit request.
extern "C"
{
    void Couch_Shutdown (void);
    void Couch_Reset (void);
    int  Couch_QuitRequested (void);   // Doom 'boolean' is an int-sized enum
}

//==============================================================================
DoomHost::DoomHost()
{
    for (auto& d : instances)
        d = std::make_unique<gin::Doom>();
}

DoomHost::~DoomHost()
{
    // Break the barrier first so no instance blocks waiting for a peer that is
    // being torn down, then stop each thread (unique_ptr reset -> ~Doom).
    Couch_Shutdown();
    for (auto& d : instances)
        d.reset();
}

void DoomHost::stop()
{
    if (! running)
        return;

    Couch_Shutdown();               // wake any instance waiting at the barrier
    for (auto& d : instances)
        d.reset();                  // join each game thread
    for (auto& d : instances)
        d = std::make_unique<gin::Doom>();   // fresh instances for the next match

    running = false;
}

bool DoomHost::quitRequested() const
{
    return Couch_QuitRequested() != 0;
}

void DoomHost::start (const juce::File& wad, const GameConfig& config,
                      const std::array<bool, kNumPlayers>& isBot)
{
    if (running)
        return;

    Couch_Reset();                  // clear arbiter state for a fresh session

    const auto setup = config.toSetup();

    // Every slot is one player of the same kNumPlayers-way session, kept in
    // lockstep by the arbiter. Player 0 renders music; the rest are -nomusic.
    for (int i = 0; i < kNumPlayers; ++i)
        instances[(size_t) i]->startGame (wad, /*playerIndex*/ i,
                                          /*numPlayers*/ kNumPlayers,
                                          /*playMusic*/ i == 0,
                                          setup,
                                          /*isBot*/ isBot[(size_t) i]);
    running = true;
}

juce::Image DoomHost::getScreen (int player)
{
    jassert (juce::isPositiveAndBelow (player, kNumPlayers));
    return instances[(size_t) player]->getScreen();
}

gin::DoomAudioEngine* DoomHost::audioEngine (int player)
{
    jassert (juce::isPositiveAndBelow (player, kNumPlayers));
    return &instances[(size_t) player]->getAudioEngine();
}

void DoomHost::postKey (int player, int key, bool down)
{
    if (running && juce::isPositiveAndBelow (player, kNumPlayers))
        instances[(size_t) player]->addEvent (key, down);
}

int DoomHost::cycleWeaponKey (int player, bool forward)
{
    if (running && juce::isPositiveAndBelow (player, kNumPlayers))
        return instances[(size_t) player]->cycleWeaponKey (forward);
    return 0;
}
