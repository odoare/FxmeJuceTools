#pragma once

#include <JuceHeader.h>

#define FIFOSIZE 16384

class CircularFifo
{
public:

    /* The buffer is initialized by default as FIFOSIZE, but it can be specified as
        an argument in the constructor */
    CircularFifo(int size = FIFOSIZE)
        : fifoBuffer(1, size), numSamplesReady(0), readHeadPos(0), writeHeadPos(0)
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

    int getFreeSpace() const
        {
            return fifoBuffer.getNumSamples() - numSamplesReady;
        }

    //==============================================================================

    juce::AudioBuffer<float> fifoBuffer;
    int numSamplesReady = 0;
    int readHeadPos = 0; // position of the reading head of the fifo
    int writeHeadPos = 0; // position of the writing head of the fifo

private:

JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CircularFifo)

};
