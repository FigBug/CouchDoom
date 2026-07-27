#pragma once

#include <juce_core/juce_core.h>
#include <gin_doom/gin_doom.h>

//==============================================================================
// The match settings chosen on the title screen. Maps to gin::DoomSetup, which
// is what the four Doom instances are actually started with. DOOM1.WAD is the
// shareware IWAD, so the episode is fixed to 1 and maps run E1M1..E1M9.
//==============================================================================
struct GameConfig
{
    enum class Mode { Coop = 0, Deathmatch = 1, Altdeath = 2 };

    Mode mode      = Mode::Deathmatch;
    int  map       = 1;      // 1..9  (E1M1..E1M9)
    int  skill     = 3;      // 1..5  (UI 1-based; Doom skill_t = skill - 1)
    bool monsters  = true;
    int  fragLimit = 0;      // 0 = unlimited

    gin::DoomSetup toSetup() const
    {
        gin::DoomSetup s;
        s.deathmatch = (int) mode;
        s.skill      = juce::jlimit (0, 4, skill - 1);
        s.episode    = 1;
        s.map        = juce::jlimit (1, 9, map);
        s.monsters   = monsters;
        s.fragLimit  = juce::jmax (0, fragLimit);
        return s;
    }

    static juce::String modeName (Mode m)
    {
        switch (m)
        {
            case Mode::Coop:       return "Co-op";
            case Mode::Deathmatch: return "Deathmatch";
            case Mode::Altdeath:   return "Altdeath";
        }
        return {};
    }

    juce::String modeName()  const { return modeName (mode); }
    juce::String mapName()   const { return "E1M" + juce::String (map); }

    juce::String skillName() const
    {
        static const char* names[] = { "I'm Too Young to Die",
                                       "Hey, Not Too Rough",
                                       "Hurt Me Plenty",
                                       "Ultra-Violence",
                                       "Nightmare!" };
        return names[juce::jlimit (0, 4, skill - 1)];
    }

    juce::String fragLimitName() const
    {
        return fragLimit <= 0 ? juce::String ("Unlimited")
                              : juce::String (fragLimit) + " frags";
    }
};
