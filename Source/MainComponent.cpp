#include "MainComponent.h"

#include "BinaryData.h"
#include "input/DoomKeys.h"

//==============================================================================
MainComponent::MainComponent()
{
    setWantsKeyboardFocus (true);

    wad = extractWad();

    title = std::make_unique<TitleScreen> (controllers);
    addAndMakeVisible (*title);
    title->onStart = [this] (const GameConfig& c) { startMatch (c); };

    // On-screen master volume (shown once a match starts; see startMatch).
    masterSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    masterSlider.setRange (0.0, 100.0, 1.0);
    masterSlider.setValue (SoundEngine::kDefaultMasterLevel * 100.0, juce::dontSendNotification);
    masterSlider.setTextValueSuffix ("%");
    masterSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 52, 22);
    masterSlider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    masterSlider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    masterSlider.setColour (juce::Slider::trackColourId, juce::Colour (0xffce2b1a));
    masterSlider.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    masterSlider.setWantsKeyboardFocus (false);   // don't steal player-0 keys
    masterSlider.onValueChange = [this]
        { soundEngine.setMasterLevel ((float) (masterSlider.getValue() / 100.0)); };
    addChildComponent (masterSlider);

    // Size last, so the resized() it triggers lays out the (now-created) title.
    setSize (kDoomWidth * 2, kDoomHeight * 2);   // 2x2 of 640x400

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

    auto file = dir.getChildFile ("DOOM1.WAD");

    if (! file.existsAsFile() || file.getSize() != (juce::int64) BinaryData::DOOM1_WADSize)
        file.replaceWithData (BinaryData::DOOM1_WAD, (size_t) BinaryData::DOOM1_WADSize);

    return file;
}

//==============================================================================
void MainComponent::startMatch (const GameConfig& config)
{
    doomHost.start (wad, config);

    std::array<gin::DoomAudioEngine*, DoomHost::kNumPlayers> engines {};
    for (int i = 0; i < DoomHost::count(); ++i)
        engines[(size_t) i] = doomHost.audioEngine (i);
    soundEngine.setEngines (engines);

    controllers.reset();
    kbDown.clear();

    state = State::Playing;
    if (title != nullptr)
        title->setVisible (false);
    masterSlider.setVisible (true);
    if (isShowing())
        grabKeyboardFocus();
    repaint();
}

void MainComponent::routeKeyboardToPlayer0()
{
    using KP = juce::KeyPress;

    std::set<int> now;
    const auto mod = juce::ModifierKeys::getCurrentModifiers();

    if (KP::isKeyCurrentlyDown ('W') || KP::isKeyCurrentlyDown (KP::upKey))    now.insert (dk::UPARROW);
    if (KP::isKeyCurrentlyDown ('S') || KP::isKeyCurrentlyDown (KP::downKey))  now.insert (dk::DOWNARROW);
    if (KP::isKeyCurrentlyDown ('A'))                                          now.insert (dk::STRAFE_L);
    if (KP::isKeyCurrentlyDown ('D'))                                          now.insert (dk::STRAFE_R);
    if (KP::isKeyCurrentlyDown (KP::leftKey))                                  now.insert (dk::LEFTARROW);
    if (KP::isKeyCurrentlyDown (KP::rightKey))                                 now.insert (dk::RIGHTARROW);
    if (KP::isKeyCurrentlyDown (KP::spaceKey))                                 now.insert (dk::FIRE);
    if (KP::isKeyCurrentlyDown ('E'))                                          now.insert (dk::USE);
    if (mod.isShiftDown())                                                     now.insert (dk::RSHIFT);
    if (KP::isKeyCurrentlyDown (KP::escapeKey))                                now.insert (dk::ESCAPE);
    for (int w = '1'; w <= '7'; ++w)
        if (KP::isKeyCurrentlyDown (w))                                        now.insert (w);

    for (int k : now)     if (kbDown.find (k) == kbDown.end())  doomHost.postKey (0, k, true);
    for (int k : kbDown)  if (now.find (k)     == now.end())    doomHost.postKey (0, k, false);
    kbDown = std::move (now);
}

//==============================================================================
void MainComponent::returnToMenu()
{
    // Detach audio from the instances before stop() destroys them, so the audio
    // thread never touches a dangling engine.
    soundEngine.setEngines ({ nullptr, nullptr, nullptr, nullptr });
    doomHost.stop();

    kbDown.clear();
    state = State::Title;
    masterSlider.setVisible (false);
    if (title != nullptr)
    {
        title->setVisible (true);
        title->toFront (false);
        if (isShowing())
            title->grabKeyboardFocus();
    }
    repaint();
}

void MainComponent::timerCallback()
{
    if (state == State::Title)
    {
        if (title != nullptr)
            title->tick();
        return;
    }

    // A player chose Quit -> tear the match down and go back to the lobby.
    if (doomHost.quitRequested())
    {
        returnToMenu();
        return;
    }

    controllers.route (doomHost);
    routeKeyboardToPlayer0();
    repaint();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    if (state != State::Playing)
        return;   // the title child covers the window in the lobby

    const auto full = getLocalBounds();
    const int  w    = full.getWidth()  / 2;
    const int  h    = full.getHeight() / 2;

    const juce::Rectangle<int> quads[DoomHost::kNumPlayers] =
    {
        { 0, 0, w,                   h },
        { w, 0, full.getWidth() - w, h },
        { 0, h, w,                   full.getHeight() - h },
        { w, h, full.getWidth() - w, full.getHeight() - h },
    };

    for (int i = 0; i < DoomHost::count(); ++i)
    {
        auto img = doomHost.getScreen (i);
        if (img.isValid())
            // Letterbox: keep the framebuffer's aspect ratio, centred in the
            // quadrant, so resizing the window never stretches the view.
            g.drawImageWithin (img, quads[i].getX(), quads[i].getY(),
                               quads[i].getWidth(), quads[i].getHeight(),
                               juce::RectanglePlacement::centred);

        g.setColour (juce::Colours::darkgrey);
        g.drawRect (quads[i]);
    }

    // Master-volume backdrop + label, drawn behind the slider child.
    const auto sb    = masterSlider.getBounds();
    const auto panel = sb.expanded (10, 8).withLeft (sb.getX() - 82);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillRoundedRectangle (panel.toFloat(), 6.0f);
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.setFont (juce::Font (juce::FontOptions (13.0f).withStyle ("Bold")));
    g.drawText ("VOLUME", panel.getX() + 10, panel.getY(),
                sb.getX() - panel.getX() - 16, panel.getHeight(),
                juce::Justification::centredLeft);
}

void MainComponent::resized()
{
    if (title != nullptr)
        title->setBounds (getLocalBounds());

    // Compact master-volume control in the bottom-right corner.
    const int w = 240, h = 26, margin = 14;
    masterSlider.setBounds (getWidth()  - margin - w,
                            getHeight() - margin - h, w, h);
}
