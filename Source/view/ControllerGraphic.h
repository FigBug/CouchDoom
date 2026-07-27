#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <gin_controllers/gin_controllers.h>

//==============================================================================
// Draws a stylised Xbox-style controller inside a rectangle, lighting up each
// button/stick/trigger that is currently pressed on `pad`. Used by the title
// screen so each player can press a button and see which quadrant is theirs.
//
// pad may be null or disconnected -> the controller is drawn in a neutral
// (unpressed) state. `dim` fades the whole thing (an unclaimed "AI" slot).
// `accent` is the player's colour, used for the press highlight.
//==============================================================================
namespace ControllerGraphic
{
    void draw (juce::Graphics& g,
               juce::Rectangle<float> area,
               gin::GameController* pad,
               bool dim,
               juce::Colour accent);
}
