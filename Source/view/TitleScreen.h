#pragma once

#include <array>
#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../game/GameConfig.h"
#include "../input/ControllerRouter.h"

//==============================================================================
// The lobby / title screen. Shows a 2x2 grid of Xbox controllers - one per
// quadrant - labelled P1..P4 (bright, when a controller is connected) or "AI"
// (dim, when not). Pressing buttons lights up that quadrant's controller so
// each player can find themselves. A side menu selects mode / map / skill /
// monsters / frag limit; START begins the match.
//
// Driven by MainComponent: call tick() each frame with the shared router to
// refresh the live controller display and process controller menu navigation.
//==============================================================================
class TitleScreen : public juce::Component
{
public:
    explicit TitleScreen (ControllerRouter& router);

    // Fired when the user starts the match, carrying the chosen settings.
    std::function<void (const GameConfig&)> onStart;

    // Per-frame update: repaint live controller state + handle controller nav.
    void tick();

    void paint (juce::Graphics&) override;
    bool keyPressed (const juce::KeyPress&) override;
    void mouseDown (const juce::MouseEvent&) override;

    static juce::Colour playerColour (int player);

private:
    enum Row { rowMode = 0, rowMap, rowSkill, rowMonsters, rowFrags, rowStart, numRows };

    ControllerRouter& router;
    GameConfig        config;
    int               selected = rowStart;

    // per-pad previous nav-button state, for edge detection
    std::array<juce::uint32, ControllerRouter::kNumPlayers> prevMask { 0, 0, 0, 0 };

    void moveSelection (int dir);
    void changeValue   (int dir);
    void activate();                 // A / Enter on the selected row
    void launch();

    juce::String rowLabel (int row) const;
    juce::String rowValue (int row) const;

    juce::Rectangle<int> menuArea()      const;
    juce::Rectangle<int> rowRect (int row) const;
    juce::Rectangle<int> quadRect (int player) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitleScreen)
};
