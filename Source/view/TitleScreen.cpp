#include "TitleScreen.h"

#include "ControllerGraphic.h"

namespace
{
    constexpr int titleH = 84;

    juce::Font f (float h, bool bold = false)
    {
        return juce::Font (juce::FontOptions (h).withStyle (bold ? "Bold" : "Regular"));
    }

    // Width reserved on the right for the menu panel.
    int menuWidth (juce::Rectangle<int> content)
    {
        return juce::jmax (280, content.getWidth() * 34 / 100);
    }
}

//==============================================================================
TitleScreen::TitleScreen (ControllerRouter& r) : router (r)
{
    setWantsKeyboardFocus (true);
}

juce::Colour TitleScreen::playerColour (int player)
{
    static const juce::Colour cols[] = { juce::Colour (0xff4caf50),   // P1 green
                                         juce::Colour (0xff2196f3),   // P2 blue
                                         juce::Colour (0xffff9800),   // P3 orange
                                         juce::Colour (0xffe040fb) }; // P4 purple
    return cols[juce::jlimit (0, 3, player)];
}

//==============================================================================
void TitleScreen::tick()
{
    using B = gin::GameController::Button;
    using A = gin::GameController::Axis;

    for (int p = 0; p < ControllerRouter::kNumPlayers; ++p)
    {
        auto* pad = router.pad (p);
        juce::uint32 mask = 0;

        if (pad != nullptr && pad->isConnected())
        {
            auto set = [&] (bool v, int bit) { if (v) mask |= (1u << bit); };
            set (pad->isButtonDown (B::dpadUp)    || pad->getAxis (A::leftY) < -0.6f, 0);
            set (pad->isButtonDown (B::dpadDown)  || pad->getAxis (A::leftY) >  0.6f, 1);
            set (pad->isButtonDown (B::dpadLeft)  || pad->getAxis (A::leftX) < -0.6f, 2);
            set (pad->isButtonDown (B::dpadRight) || pad->getAxis (A::leftX) >  0.6f, 3);
            set (pad->isButtonDown (B::faceDown), 4);   // A
            set (pad->isButtonDown (B::start),    5);
        }

        const juce::uint32 rising = mask & ~prevMask[(size_t) p];
        prevMask[(size_t) p] = mask;

        if (rising & (1u << 0)) moveSelection (-1);
        if (rising & (1u << 1)) moveSelection ( 1);
        if (rising & (1u << 2)) changeValue  (-1);
        if (rising & (1u << 3)) changeValue  ( 1);
        if (rising & (1u << 4)) activate();
        if (rising & (1u << 5)) launch();
    }

    repaint();
}

void TitleScreen::moveSelection (int dir)
{
    selected = (selected + dir + numRows) % numRows;
}

void TitleScreen::changeValue (int dir)
{
    switch (selected)
    {
        case rowMode:
            config.mode = (GameConfig::Mode) (((int) config.mode + dir + 3) % 3);
            break;
        case rowMap:
            config.map = (config.map - 1 + dir + 9) % 9 + 1;
            break;
        case rowSkill:
            config.skill = juce::jlimit (1, 5, config.skill + dir);
            break;
        case rowMonsters:
            config.monsters = ! config.monsters;
            break;
        case rowFrags:
            config.fragLimit = juce::jlimit (0, 50, config.fragLimit + dir * 5);
            break;
        case rowVolume:
            volumePct = juce::jlimit (0, 100, volumePct + dir * 5);
            if (onMasterLevel)
                onMasterLevel ((float) volumePct / 100.0f);
            break;
        default:
            break;
    }
}

void TitleScreen::setMasterLevel (float level)
{
    volumePct = juce::jlimit (0, 100, juce::roundToInt (level * 100.0f));
    repaint();
}

void TitleScreen::activate()
{
    if (selected == rowStart)
        launch();
    else
        changeValue (1);
}

void TitleScreen::launch()
{
    if (onStart)
        onStart (config);
}

//==============================================================================
bool TitleScreen::keyPressed (const juce::KeyPress& k)
{
    if (k == juce::KeyPress::upKey)    { moveSelection (-1); return true; }
    if (k == juce::KeyPress::downKey)  { moveSelection ( 1); return true; }
    if (k == juce::KeyPress::leftKey)  { changeValue  (-1); return true; }
    if (k == juce::KeyPress::rightKey) { changeValue  ( 1); return true; }
    if (k == juce::KeyPress::returnKey || k == juce::KeyPress::spaceKey) { activate(); return true; }
    return false;
}

void TitleScreen::mouseDown (const juce::MouseEvent& e)
{
    for (int r = 0; r < numRows; ++r)
    {
        if (rowRect (r).contains (e.getPosition()))
        {
            selected = r;
            if (r == rowStart) launch();
            else               changeValue (1);
            repaint();
            return;
        }
    }
}

//==============================================================================
juce::Rectangle<int> TitleScreen::menuArea() const
{
    auto b = getLocalBounds();
    b.removeFromTop (titleH);
    return b.removeFromRight (menuWidth (b)).reduced (16);
}

juce::Rectangle<int> TitleScreen::rowRect (int row) const
{
    auto m = menuArea();
    m.removeFromTop (48);                 // panel header
    const int rh = 54;
    return { m.getX(), m.getY() + row * rh, m.getWidth(), rh - 8 };
}

juce::Rectangle<int> TitleScreen::quadRect (int player) const
{
    auto b = getLocalBounds();
    b.removeFromTop (titleH);
    b.removeFromRight (menuWidth (b));
    auto left = b.reduced (12);
    const int hw = left.getWidth() / 2;
    const int hh = left.getHeight() / 2;
    const int col = player % 2, row = player / 2;
    return { left.getX() + col * hw, left.getY() + row * hh, hw, hh };
}

//==============================================================================
juce::String TitleScreen::rowLabel (int row) const
{
    switch (row)
    {
        case rowMode:     return "Mode";
        case rowMap:      return "Map";
        case rowSkill:    return "Skill";
        case rowMonsters: return "Monsters";
        case rowFrags:    return "Frag Limit";
        case rowVolume:   return "Volume";
        default:          return {};
    }
}

juce::String TitleScreen::rowValue (int row) const
{
    switch (row)
    {
        case rowMode:     return config.modeName();
        case rowMap:      return config.mapName();
        case rowSkill:    return config.skillName();
        case rowMonsters: return config.monsters ? "On" : "Off";
        case rowFrags:    return config.fragLimitName();
        case rowVolume:   return juce::String (volumePct) + "%";
        default:          return {};
    }
}

//==============================================================================
void TitleScreen::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0e0f12));

    // ---- title ----
    auto top = getLocalBounds().removeFromTop (titleH);
    g.setColour (juce::Colour (0xffce2b1a));
    g.setFont (f (52.0f, true));
    g.drawText ("CouchDoom", top, juce::Justification::centred);

    // ---- controllers grid ----
    for (int p = 0; p < ControllerRouter::kNumPlayers; ++p)
    {
        auto quad = quadRect (p);
        const bool connected = router.connected (p);

        auto padArea = quad.reduced (10).withTrimmedBottom (34).toFloat();
        ControllerGraphic::draw (g, padArea, router.pad (p), ! connected, playerColour (p));

        auto label = quad.removeFromBottom (34);
        g.setFont (f (22.0f, true));
        g.setColour (connected ? playerColour (p) : juce::Colour (0xff54585f));
        g.drawText (connected ? ("P" + juce::String (p + 1)) : juce::String ("AI"),
                    label, juce::Justification::centred);
    }

    // ---- menu panel ----
    auto m = menuArea();
    g.setColour (juce::Colour (0xff17191e));
    g.fillRoundedRectangle (m.expanded (10).toFloat(), 10.0f);

    g.setColour (juce::Colour (0xff9aa0aa));
    g.setFont (f (18.0f, true));
    g.drawText ("MATCH SETUP", m.removeFromTop (48).withTrimmedTop (8),
                juce::Justification::centredTop);

    for (int r = 0; r < numRows; ++r)
    {
        auto rr = rowRect (r);
        const bool sel = (r == selected);

        if (r == rowStart)
        {
            g.setColour (sel ? juce::Colour (0xff37a24a) : juce::Colour (0xff2b2f36));
            g.fillRoundedRectangle (rr.toFloat(), 8.0f);
            g.setColour (juce::Colours::white);
            g.setFont (f (24.0f, true));
            g.drawText ("START", rr, juce::Justification::centred);
            continue;
        }

        if (sel)
        {
            g.setColour (juce::Colour (0xff262a31));
            g.fillRoundedRectangle (rr.toFloat(), 8.0f);
            g.setColour (juce::Colour (0xffce2b1a));
            g.drawRoundedRectangle (rr.toFloat().reduced (0.5f), 8.0f, 1.5f);
        }

        auto rowInner = rr.reduced (14, 0);
        g.setColour (juce::Colour (0xffb8bec8));
        g.setFont (f (19.0f));
        g.drawText (rowLabel (r), rowInner, juce::Justification::centredLeft);
        g.setColour (sel ? juce::Colours::white : juce::Colour (0xff8b909a));
        g.setFont (f (19.0f, true));

        // little left/right arrows around the value when selected
        juce::String val = (sel ? juce::String (juce::CharPointer_UTF8 ("\xe2\x97\x80 ")) : juce::String())
                         + rowValue (r)
                         + (sel ? juce::String (juce::CharPointer_UTF8 (" \xe2\x96\xb6")) : juce::String());
        g.drawText (val, rowInner, juce::Justification::centredRight);
    }

    // ---- hint ----
    auto hint = menuArea();
    hint = hint.removeFromBottom (54);
    g.setColour (juce::Colour (0xff6b7079));
    g.setFont (f (14.0f));
    g.drawFittedText ("D-pad / arrows: navigate    A / Enter: change    START button: begin",
                      hint, juce::Justification::centred, 2);
}
