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
#define FIFOSIZE 16384
#define SCOPEFPS 40

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

class CircularFifo
{
public:

    CircularFifo()
    {
        fifoBuffer.clear();
    };

    ~CircularFifo(){};

    void fillFifoWithBuffer (const juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() > 0)
        {
            auto* channelData = buffer.getReadPointer (0,0);

            if (FIFOSIZE < buffer.getNumSamples() + numSamplesReady)
            {
                std::cout << "Buffer full" << std::endl; // Not enough samples in the fifo to fill the buffer
            }
            if (writeHeadPos + buffer.getNumSamples() < FIFOSIZE)
            {
                fifoBuffer.copyFrom(0, writeHeadPos, channelData, buffer.getNumSamples());
                numSamplesReady += buffer.getNumSamples();
                writeHeadPos += buffer.getNumSamples();
            }
            else
            {
                auto remaining = FIFOSIZE - writeHeadPos;
                fifoBuffer.copyFrom(0, writeHeadPos, channelData, remaining);
                fifoBuffer.copyFrom(0, 0, channelData+remaining, buffer.getNumSamples() - remaining);
                numSamplesReady += buffer.getNumSamples();
                writeHeadPos = buffer.getNumSamples() - remaining;
            }
        }
    };

    void fillBufferWithFifo (juce::AudioBuffer<float>& bufferToFill)
    {
        if (numSamplesReady > bufferToFill.getNumSamples())
        {
            auto* channelData = bufferToFill.getWritePointer(0, 0);
            if (readHeadPos + bufferToFill.getNumSamples() < FIFOSIZE)
            {
                bufferToFill.copyFrom(0, 0, fifoBuffer.getReadPointer(0, readHeadPos), bufferToFill.getNumSamples());
                numSamplesReady -= bufferToFill.getNumSamples();
                readHeadPos += bufferToFill.getNumSamples();
            }
            else
            {
                auto remaining = FIFOSIZE - readHeadPos;
                bufferToFill.copyFrom(0, 0, fifoBuffer.getReadPointer(0, readHeadPos), remaining);
                bufferToFill.copyFrom(0, remaining, fifoBuffer.getReadPointer(0, 0), bufferToFill.getNumSamples() - remaining);
                numSamplesReady -= bufferToFill.getNumSamples();
                readHeadPos = (bufferToFill.getNumSamples() - remaining);
            }
        }
    };

    void resetFifo()
    {
        fifoBuffer.clear();
        numSamplesReady = 0;
        int readHeadPos = 0;
        int writeHeadPos = 0;
    };

    //==============================================================================

    juce::AudioBuffer<float> fifoBuffer{1, FIFOSIZE};
    int numSamplesReady = 0;
    int readHeadPos = 0; // position of the reading head of the fifo
    int writeHeadPos = 0; // position of the writing head of the fifo

private:
    
JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CircularFifo)

};

//==============================================================================
class ScrollingScopeComponent : public juce::Component, private juce::Timer
{
public:
    ScrollingScopeComponent(CircularFifo* fifo,
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

    // ~ScrollingScopeComponent() override
    // {

    // }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        //juce::Rectangle<int> drawArea = getLocalBounds();
        //g.setColour(juce::Colours::black);k
        g.fillRoundedRectangle(getLocalBounds().reduced(5.f).toFloat(),10.f);
        g.drawImage (scrollingScopeImage, getLocalBounds().reduced(10.f).toFloat());
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

    void setContrast(float newContrast)
    {
        contrast = juce::jlimit(0.1f, 2.0f, newContrast);
    }

    void mouseDrag(const juce::MouseEvent& e)
    {

        // Adjust the contrast based on mouse position
        float changeVal = 0.0;
        if(e.getDistanceFromDragStartY() < 0) changeVal = -0.005f; //up
        if(e.getDistanceFromDragStartY() > 0) changeVal = +0.005f; //down
        setContrast(contrast+changeVal);

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
        std::cout << "NSamples:" << scrollingScopeFifo->numSamplesReady
                    << "  SpPerPix:" << samplesPerPixel
                    << "  NbBlocks:" << numBlocks << std::endl;

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

            for (int y = 1; y < ySize; ++y)
            {
                float yValue = 2*float(y)/float(ySize) - 1;

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

        }

        // Update the x position for the next line
        // We add the number of blocks we just drew to the x position
        xPos += numBlocks;

    };

    CircularFifo* getFifo()
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
    CircularFifo* circularFifo;
    int xSize, ySize;
    int xPos;
    int samplesPerPixel;
    juce::Colour colour;
    float contrast = 1.0f; // Typically between 0.1 and 2.0

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrollingScopeComponent)
};
