/*
--------------------------------------------------------------

ScrollingScopeComponent.h

This file is part of the FxmeJuceTools module for JUCE
O. Doaré - 2025

github.com/odoare/FxmeJuceTools

--------------------------------------------------------------
*/

#pragma once
// #define FIFOSIZE 16384
#define SCOPEFPS 50

#include <JuceHeader.h>
#include <FxmeJuceTools/FxmeJuceTools.h>
#include <FxmeJuceTools/Dsp/CircularFifo.h>
// #include <juce_dsp/juce_dsp.h>
#include <array>


//==============================================================================
class ScrollingScopeComponent : public juce::Component, public juce::Timer
{
public:
    ScrollingScopeComponent(fxme::CircularFifo* fifo,
        int xsize = 512,
        int ysize = 1024,
        int smpPerPix = 128,
        juce::Colour col = juce::Colours::green)
        : scrollingScopeImage (juce::Image::RGB, xsize, ysize, true)
    {
        xSize = xsize;
        ySize = ysize;
        xPos = 0; // initial position of the scope
        samplesPerPixel = smpPerPix;
        circularFifo = fifo;
        colour = col;
        startTimerHz (SCOPEFPS);
    };

    ~ScrollingScopeComponent()
    {
        stopTimer();
    };

    // ~ScrollingScopeComponent() override
    // {

    // }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        g.fillRoundedRectangle(getLocalBounds().reduced(5.f).toFloat(),10.f);
        g.drawImage (scrollingScopeImage,
            getLocalBounds().reduced(10.f).toFloat()
            // ,juce::RectanglePlacement::doNotResize
        );
    };

    void timerCallback() override
    {
        // std::cout << circularFifo->numSamplesReady << " samples ready in the fifo" << std::endl;
        if (circularFifo->numSamplesReady < samplesPerPixel)
            return; // not enough samples to draw a new line
        else
        {
            drawNextLineOfScrollingScope(); // if we have enough samples, we draw a new line
            repaint(); // repaint the component to show the new line
        }
    };

    void setGain(float newGain)
    {
        scopeGain = juce::jlimit(0.1f, 2.0f, newGain);
    }

    void mouseDrag(const juce::MouseEvent& e)
    {
        // Adjust the contrast based on mouse position
        float changeVal = 0.0;
        if(e.getDistanceFromDragStartY() < 0) changeVal = -0.005f; //up
        if(e.getDistanceFromDragStartY() > 0) changeVal = +0.005f; //down
        setGain(scopeGain+changeVal);
    }

    void setScrolling(bool shouldScroll)
    {
        if (shouldScroll)
        {
            startTimerHz(SCOPEFPS);
        }
        else
        {
            stopTimer();
        }
    }

    void setSamplesPerPixel(int newValue)
    {
        if (newValue > 0)
        {
            samplesPerPixel = newValue;
            std::cout << "New samples per pixels value : " << newValue << std::endl;
        }
        else
        {
            std::cout << "Samples per pixel must be greater than 0" << std::endl;
        }
    }

    int getSamplesPerPixel() const
    {
        return samplesPerPixel;
    }

    void drawNextLineOfScrollingScope()
    {

        if (circularFifo->numSamplesReady < samplesPerPixel)
            return; // not enough samples to draw a new line

        auto scrollingScopeFifo = circularFifo;

        // Number of blocks of samples to read from the fifo
        auto numBlocks = scrollingScopeFifo->numSamplesReady / samplesPerPixel;
        // std::cout << "NSamples:" << scrollingScopeFifo->numSamplesReady
        //             << "  SpPerPix:" << samplesPerPixel
        //             << "  NbBlocks:" << numBlocks << std::endl;

        // auto rightHandEdge = scrollingScopeImage.getWidth() - numBlocks;
        // auto imageHeight   = scrollingScopeImage.getHeight();

        // Si on atteint le bord droit de l'image, on doit la scroller
        if (xPos+numBlocks >= xSize)
        {
            int numBlocksToShift = xPos + numBlocks - xSize + 1;
            xPos -= numBlocksToShift;
            scrollingScopeImage.moveImageSection (0, 0, numBlocksToShift, 0,  xSize-numBlocksToShift, ySize);
        }

        // Now we can draw the new samples on the right hand edge of the image
        // We will draw a vertical line of pixels, one for each block of samples
        // We will use the colour set in the constructor, or the default colour if none was set

        for (int i=0; i<numBlocks; ++i)
        {
            // // Get the next block of samples from the fifo
            juce::AudioBuffer<float> blockBuffer(1, samplesPerPixel);
            scrollingScopeFifo->fillBufferWithFifo(blockBuffer);
            auto levels = juce::FloatVectorOperations::findMinAndMax(blockBuffer.getReadPointer(0), samplesPerPixel);
            juce::Image::BitmapData bitmap { scrollingScopeImage, xPos+i, 0, 1, ySize, juce::Image::BitmapData::writeOnly };

            // On veut eviter les cassures dans le graphique lorsque les variations sont rapides
            auto yMin = scopeGain*juce::jmin<float>(prevMax,levels.getStart());
            auto yMax = scopeGain*juce::jmax<float>(prevMin,levels.getEnd());

            for (int y = 1; y < ySize; ++y)
            {
                float yValue = 2*float(y)/float(ySize) - 1;

                if (yValue > yMax)
                {
                    bitmap.setPixelColour (0, y, juce::Colours::black); // If the level is out of range, set pixel to black
                }
                else if (yValue < yMin)
                {
                    bitmap.setPixelColour (0, y, juce::Colours::black); // If the level is out of range, set pixel to white
                }
                else
                {
                    bitmap.setPixelColour (0, y, colour); // Otherwise, set pixel to colour
                }
            }

            // On met à jour les valeurs précédentes
            prevMin = levels.getStart();
            prevMax = levels.getEnd();

        }

        juce::Image::BitmapData bitmap1 { scrollingScopeImage, 0, 0, xSize, 1, juce::Image::BitmapData::writeOnly };
        juce::Image::BitmapData bitmap2 { scrollingScopeImage, 0, ySize-1, xSize, 1, juce::Image::BitmapData::writeOnly };
        for (int ix=0; ix<xSize; ix++)
        {
            bitmap1.setPixelColour (ix, 0, colour); // Otherwise, set pixel to colour
            bitmap2.setPixelColour (ix, 0, colour); // Otherwise, set pixel to colour
        }

        // Update the x position for the next line
        // We add the number of blocks we just drew to the x position
        xPos += numBlocks;

    };

    fxme::CircularFifo* getFifo()
    {
        return circularFifo;
    }

    void resetScope()
    {
        circularFifo->resetFifo();
        scrollingScopeImage.clear(juce::Rectangle<int>(scrollingScopeImage.getWidth(),
                                    scrollingScopeImage.getHeight()),
                                    juce::Colours::black);
        xPos = 0;
        repaint();
    }

    juce::Image scrollingScopeImage ;

private:
    fxme::CircularFifo* circularFifo;
    int xSize, ySize;
    int xPos;
    int samplesPerPixel;
    juce::Colour colour;
    float scopeGain = 1.0f; // Typically between 0.1 and 2.0
    float prevMin, prevMax;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrollingScopeComponent)
};
