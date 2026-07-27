#include "MainComponent.h"

#include "BinaryData.h"

//==============================================================================
MainComponent::MainComponent()
{
    setWantsKeyboardFocus (true);
    setSize (kDoomWidth * 2, kDoomHeight * 2);   // 2x2 of 640x400

    auto wad = extractWad();
    doomHost.start (wad, kActivePlayers);

    std::array<gin::DoomAudioEngine*, DoomHost::kNumPlayers> engines {};
    for (int i = 0; i < DoomHost::count(); ++i)
        engines[(size_t) i] = doomHost.audioEngine (i);
    soundEngine.setEngines (engines);

    startTimerHz (kTickHz);
}

MainComponent::~MainComponent()
{
    stopTimer();
}

//==============================================================================
juce::File MainComponent::extractWad()
{
    // Bake-once, extract-to-disk: all instances load the same file.
    auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                   .getChildFile ("CouchDoom");
    dir.createDirectory();

    auto wad = dir.getChildFile ("DOOM1.WAD");

    if (! wad.existsAsFile() || wad.getSize() != (juce::int64) BinaryData::DOOM1_WADSize)
        wad.replaceWithData (BinaryData::DOOM1_WAD, (size_t) BinaryData::DOOM1_WADSize);

    return wad;
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    const auto full = getLocalBounds();
    const int  w    = full.getWidth()  / 2;
    const int  h    = full.getHeight() / 2;

    const juce::Rectangle<int> quads[DoomHost::kNumPlayers] =
    {
        { 0, 0, w,                  h },
        { w, 0, full.getWidth() - w, h },
        { 0, h, w,                  full.getHeight() - h },
        { w, h, full.getWidth() - w, full.getHeight() - h },
    };

    for (int i = 0; i < DoomHost::count(); ++i)
    {
        auto img = doomHost.getScreen (i);
        if (img.isValid())
            g.drawImage (img, quads[i].toFloat());

        g.setColour (juce::Colours::darkgrey);
        g.drawRect (quads[i]);
    }
}

void MainComponent::resized()
{
}

//==============================================================================
void MainComponent::timerCallback()
{
    // Game loop: the fake-network arbiter tick will run here; for now just
    // present the latest framebuffers.
    repaint();
}
