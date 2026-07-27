#include "DoomHost.h"

//==============================================================================
DoomHost::DoomHost()
{
    for (auto& d : instances)
        d = std::make_unique<gin::Doom>();
}

DoomHost::~DoomHost()
{
    // unique_ptr reset stops each Doom's thread in its destructor.
    for (auto& d : instances)
        d.reset();
}

void DoomHost::start (const juce::File& wad, const GameConfig& config)
{
    if (running)
        return;

    const auto setup = config.toSetup();

    // Every slot is one player of the same kNumPlayers-way session, kept in
    // lockstep by the arbiter. Player 0 renders music; the rest are -nomusic.
    for (int i = 0; i < kNumPlayers; ++i)
        instances[(size_t) i]->startGame (wad, /*playerIndex*/ i,
                                          /*numPlayers*/ kNumPlayers,
                                          /*playMusic*/ i == 0,
                                          setup);
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
