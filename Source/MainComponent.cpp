#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setWantsKeyboardFocus (true);

    // 2x2 grid of 640x400 Doom framebuffers.
    setSize (kDoomWidth * 2, kDoomHeight * 2);

    startTimerHz (kTickHz);
}

MainComponent::~MainComponent()
{
    stopTimer();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);

    // Placeholder quadrant guides (2x2) until the Doom views are wired in.
    g.drawLine ((float) getWidth() / 2.0f, 0.0f,
                (float) getWidth() / 2.0f, (float) getHeight());
    g.drawLine (0.0f, (float) getHeight() / 2.0f,
                (float) getWidth(), (float) getHeight() / 2.0f);

    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions (20.0f)));
    g.drawText ("CouchDoom \xe2\x80\x94 4-instance deathmatch (scaffold)",
                getLocalBounds(), juce::Justification::centred);
}

void MainComponent::resized()
{
    // Child Doom views will be laid out here into the 2x2 grid.
}

//==============================================================================
void MainComponent::timerCallback()
{
    // The game loop lives here: pump the fake-network arbiter, then repaint.
    repaint();
}
