#include "ControllerGraphic.h"

using Button = gin::GameController::Button;
using Axis   = gin::GameController::Axis;

namespace ControllerGraphic
{

void draw (juce::Graphics& g, juce::Rectangle<float> full,
           gin::GameController* pad, bool dim, juce::Colour accent)
{
    const bool live = pad != nullptr && pad->isConnected();
    auto down = [&] (Button b) { return live && pad->isButtonDown (b); };
    auto axis = [&] (Axis a)   { return live ? pad->getAxis (a) : 0.0f; };

    // Fit a ~1.4:1 controller box centred in `full`.
    const float aspect = 1.4f;
    auto area = full.reduced (full.getWidth() * 0.04f);
    if (area.getWidth() / area.getHeight() > aspect)
        area = area.withWidth (area.getHeight() * aspect);
    else
        area = area.withHeight (area.getWidth() / aspect);
    area = area.withCentre (full.getCentre());

    const float W = area.getWidth();
    const float H = area.getHeight();
    auto P = [&] (float nx, float ny)
    {
        return juce::Point<float> (area.getX() + nx * W, area.getY() + ny * H);
    };

    const float alpha = dim ? 0.28f : 1.0f;
    const juce::Colour bodyCol = juce::Colour (0xff2a2d33).withMultipliedAlpha (alpha);
    const juce::Colour partCol = juce::Colour (0xff4a4f57).withMultipliedAlpha (alpha);
    const juce::Colour edgeCol = juce::Colour (0xff15171b).withMultipliedAlpha (alpha);
    const juce::Colour hi      = accent.withMultipliedAlpha (alpha);

    // ---- body: central slab + two grip lobes ----
    juce::Path body;
    body.addRoundedRectangle (area.getX() + W * 0.14f, area.getY() + H * 0.10f,
                              W * 0.72f, H * 0.52f, H * 0.22f);
    const float gripR = H * 0.30f;
    auto lg = P (0.20f, 0.60f), rg = P (0.80f, 0.60f);
    body.addEllipse (lg.x - gripR, lg.y - gripR, gripR * 2, gripR * 2);
    body.addEllipse (rg.x - gripR, rg.y - gripR, gripR * 2, gripR * 2);
    g.setColour (bodyCol);
    g.fillPath (body);
    g.setColour (edgeCol);
    g.strokePath (body, juce::PathStrokeType (juce::jmax (1.0f, H * 0.012f)));

    // A round element; when pressed it brightens and gets an accent ring.
    auto disc = [&] (juce::Point<float> c, float r, juce::Colour base, bool pressed)
    {
        g.setColour (pressed ? base.brighter (0.7f) : base);
        g.fillEllipse (c.x - r, c.y - r, r * 2, r * 2);
        if (pressed)
        {
            g.setColour (hi);
            g.drawEllipse (c.x - r, c.y - r, r * 2, r * 2, juce::jmax (1.5f, r * 0.30f));
        }
    };

    // A pill (bumper/trigger); accent fill when pressed.
    auto bar = [&] (float x0, float y0, float x1, float y1, bool pressed)
    {
        juce::Rectangle<float> r (P (x0, y0), P (x1, y1));
        g.setColour (pressed ? hi : partCol);
        g.fillRoundedRectangle (r, r.getHeight() * 0.45f);
    };

    // ---- triggers + bumpers along the top edge ----
    bar (0.20f, 0.00f, 0.36f, 0.045f, down (Button::leftTrigger));
    bar (0.64f, 0.00f, 0.80f, 0.045f, down (Button::rightTrigger));
    bar (0.17f, 0.06f, 0.37f, 0.105f, down (Button::leftShoulder));
    bar (0.63f, 0.06f, 0.83f, 0.105f, down (Button::rightShoulder));

    // ---- sticks (knob deflects with the axis) ----
    auto stick = [&] (juce::Point<float> c, Axis ax, Axis ay, Button click)
    {
        const float r = H * 0.11f;
        g.setColour (edgeCol);
        g.fillEllipse (c.x - r, c.y - r, r * 2, r * 2);
        auto k = c.translated (axis (ax) * r * 0.6f, axis (ay) * r * 0.6f);
        const bool active = down (click) || std::abs (axis (ax)) > 0.5f || std::abs (axis (ay)) > 0.5f;
        disc (k, r * 0.78f, partCol, active);
    };
    stick (P (0.28f, 0.30f), Axis::leftX,  Axis::leftY,  Button::leftStick);
    stick (P (0.60f, 0.52f), Axis::rightX, Axis::rightY, Button::rightStick);

    // ---- dpad (plus) ----
    {
        auto c = P (0.31f, 0.55f);
        const float arm = H * 0.11f, th = H * 0.05f;
        auto seg = [&] (juce::Rectangle<float> r, bool pressed)
        {
            g.setColour (pressed ? hi : partCol);
            g.fillRoundedRectangle (r, th * 0.25f);
        };
        seg ({ c.x - th / 2, c.y - arm, th, arm }, down (Button::dpadUp));
        seg ({ c.x - th / 2, c.y,       th, arm }, down (Button::dpadDown));
        seg ({ c.x - arm,    c.y - th / 2, arm, th }, down (Button::dpadLeft));
        seg ({ c.x,          c.y - th / 2, arm, th }, down (Button::dpadRight));
    }

    // ---- face buttons (ABXY diamond, canonical colours) ----
    {
        auto c = P (0.72f, 0.34f);
        const float d = H * 0.12f, r = H * 0.055f;
        disc (c.translated (0, -d), r, juce::Colour (0xffe8b923).withMultipliedAlpha (alpha), down (Button::faceUp));    // Y
        disc (c.translated (0,  d), r, juce::Colour (0xff37a24a).withMultipliedAlpha (alpha), down (Button::faceDown));  // A
        disc (c.translated (-d, 0), r, juce::Colour (0xff2f6fd0).withMultipliedAlpha (alpha), down (Button::faceLeft));  // X
        disc (c.translated ( d, 0), r, juce::Colour (0xffca3b32).withMultipliedAlpha (alpha), down (Button::faceRight)); // B
    }

    // ---- view / menu buttons ----
    disc (P (0.45f, 0.30f), H * 0.028f, partCol, down (Button::select));
    disc (P (0.55f, 0.30f), H * 0.028f, partCol, down (Button::start));
}

}
