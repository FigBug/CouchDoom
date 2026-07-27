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

void DoomHost::start (const juce::File& wad, int numPlayers)
{
    numPlayers = juce::jlimit (1, kNumPlayers, numPlayers);

    // numPlayers > 1 => each instance is one player of a local deathmatch,
    // kept in lockstep by the fake-network arbiter. Player 0 renders music.
    for (int i = 0; i < numPlayers; ++i)
        instances[(size_t) i]->startGame (wad, /*playerIndex*/ i,
                                          /*numPlayers*/ numPlayers,
                                          /*playMusic*/ i == 0);
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
    if (juce::isPositiveAndBelow (player, kNumPlayers))
        instances[(size_t) player]->addEvent (key, down);
}
