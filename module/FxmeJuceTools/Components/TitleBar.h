/*
  ==============================================================================

    FxmeSlider.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class TitleBar : public juce::Component
{
public:
    TitleBar()
    {
    }

    TitleBar(juce::String tit, juce::Colour fontCol, juce::Colour barCol)
    {
        title = tit;
        fontColour = fontCol;
        barColour = barCol;
    }

    ~TitleBar() override
    {
    }
    void paint(juce::Graphics& g) override
    {
        g.fillAll(barColour.darker());
        g.setColour(fontColour);
        g.setFont(18.0f);
        g.drawText(title, getLocalBounds(), juce::Justification::centred);
    }
    void resized() override
    {
        repaint();
    }
    void setTitle(juce::String tit)
    {
        title = tit;
    }
    void setFontColour(juce::Colour col)
    {
        fontColour = col;
    }
    void setBarColour(juce::Colour col)
    {
        barColour = col;
    }
    
private:

    juce::Colour fontColour{juce::Colours::white}, barColour{juce::Colours::grey};
    juce::String title{""};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TitleBar)    
};
