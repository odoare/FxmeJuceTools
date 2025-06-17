/*
--------------------------------------------------------------

ScrollingScopeComponent.h

This file is part of the FxmeJuceTools module for JUCE
O. Doaré - 2025

github.com/odoare/FxmeJuceTools

This ScrollingScope component was inspired by the JUCE tutorial :
https://juce.com/tutorials/tutorial_simple_fft/

--------------------------------------------------------------
*/

#pragma once
#define SCOPESIZE 441
#define SCOPEFPS 100

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

class ScrollingScopeFifo
{
public:

    ScrollingScopeFifo()
    {
        std::fill (fifo.begin(), fifo.end(), 0.0f);
        std::fill (data.begin(), data.end(), 0.0f);
    };

    ~ScrollingScopeFifo(){};

    void fillFifoWithBlock (const juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() > 0)
        {
            auto* channelData = buffer.getReadPointer (0,0);

            for (auto i = 0; i < buffer.getNumSamples(); ++i)
                pushNextSampleIntoFifo (channelData[i]);
        }
    };

    void pushNextSampleIntoFifo (float sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next line should now be rendered..
        if (fifoIndex == SCOPESIZE)
        {
            if (! nextBlockReady)
            {
                std::fill (data.begin(), data.end(), 0.0f);
                std::copy (fifo.begin(), fifo.end(), data.begin());
                nextBlockReady = true;
            }

            fifoIndex = 0;
        }

        fifo[(size_t) fifoIndex++] = sample;
    };

    bool nextBlockReady = false;
    static constexpr auto size = SCOPESIZE;
    std::array<float, size> fifo;
    int fifoIndex = 0;
    std::array<float, size> data;

private:
    
JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrollingScopeFifo)

};

//==============================================================================
class ScrollingScopeComponent : public juce::Component, private juce::Timer
{
public:
    ScrollingScopeComponent(ScrollingScopeFifo* fifo,
        int xsize = 32,
        int ysize = 1024,
        juce::Colour col = juce::Colours::green)
        : scrollingScopeImage (juce::Image::RGB, xsize, ysize, true)
    {
        scrollingScopeFifo = fifo;
        colour = col;
        startTimerHz (SCOPEFPS);
    };

    // ~ScrollingScopeComponent() override
    // {

    // }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        //juce::Rectangle<int> drawArea = getLocalBounds();
        //g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(getLocalBounds().reduced(5.f).toFloat(),10.f);
        g.drawImage (scrollingScopeImage, getLocalBounds().reduced(10.f).toFloat());
    };

    void timerCallback() override
    {
        if (scrollingScopeFifo->nextBlockReady)
        {
            drawNextLineOfScrollingScope();
            scrollingScopeFifo->nextBlockReady = false;
            repaint();
        }
    };

    void setContrast(float newContrast)
    {
        contrast = juce::jlimit(0.1f, 2.0f, newContrast);
    }

    void mouseDrag(const juce::MouseEvent& e)
    {
        // if is is a mouseClick event, we check if it is a double click (doesn't work)
        // if (e.mouseWasClicked())
        // {
        //     std::cout << "ScrollingScopeComponent: Mouse clicked at position: " << e.position.toString() << std::endl;
        //     if (e.getNumberOfClicks() > 1)
        //     {
        //         // reset the hue and contrast to default values
        //         hueColour = 0.5f;
        //         contrast = 1.0f;
        //         repaint();
        //         std::cout << "ScrollingScopeComponent: Reset hue and contrast to default values." << std::endl;
        //         return;
        //     }
        // }
        // Adjust the contrast based on mouse position
        float changeVal = 0.0;
        if(e.getDistanceFromDragStartY() < 0) changeVal = -0.005f; //up
        if(e.getDistanceFromDragStartY() > 0) changeVal = +0.005f; //down
        setContrast(contrast+changeVal);

    }

    void drawNextLineOfScrollingScope()
    {
        auto rightHandEdge = scrollingScopeImage.getWidth() - 1;
        auto imageHeight   = scrollingScopeImage.getHeight();

        // first, shuffle our image leftwards by 1 pixel..
        scrollingScopeImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight);         // [1]

        auto levels = juce::FloatVectorOperations::findMinAndMax(scrollingScopeFifo->data.data(), scrollingScopeFifo->size ); // [2]
        juce::Image::BitmapData bitmap { scrollingScopeImage, rightHandEdge, 0, 1, imageHeight, juce::Image::BitmapData::writeOnly }; // [4]

        for (int y = 1; y < imageHeight; ++y)                                              // [5]
        {
            float yValue = 2*float(y)/float(imageHeight) - 1;

            if (yValue > levels.getEnd())
            {
                bitmap.setPixelColour (0, y, juce::Colours::black); // If the level is out of range, set pixel to black
            }
            else if (yValue < levels.getStart())
            {
                bitmap.setPixelColour (0, y, juce::Colours::black); // If the level is out of range, set pixel to white
            }
            else
            {
                bitmap.setPixelColour (0, y, colour); // Otherwise, set pixel to colour
            }
        }
    };

    //static constexpr auto fftOrder = 10;                // [1]
    static constexpr auto size = SCOPESIZE;     // [2]
    juce::Image scrollingScopeImage ;

private:
    ScrollingScopeFifo* scrollingScopeFifo;
    // float hueColour;
    juce::Colour colour = juce::Colours::green;
    float contrast = 1.0f; // Typically between 0.1 and 2.0

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrollingScopeComponent)
};
