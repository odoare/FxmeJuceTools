#define FFTORDER 11
#pragma once

class SpectrogramFifo
{
public:

    SpectrogramFifo()
    {
        std::fill (fifo.begin(), fifo.end(), 0.0f);
        std::fill (fftData.begin(), fftData.end(), 0.0f);
    }
    ~SpectrogramFifo()
    {
    }

    void fillFifoWithBlock (const juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() > 0)
        {
            auto* channelData = buffer.getReadPointer (0,0);

            for (auto i = 0; i < buffer.getNumSamples(); ++i)
                pushNextSampleIntoFifo (channelData[i]);
        }
    }

    void pushNextSampleIntoFifo (float sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next line should now be rendered..
        if (fifoIndex == fftSize)
        {
            if (! nextFFTBlockReady)
            {
                std::fill (fftData.begin(), fftData.end(), 0.0f);
                std::copy (fifo.begin(), fifo.end(), fftData.begin());
                nextFFTBlockReady = true;
            }

            fifoIndex = 0;
        }

        fifo[(size_t) fifoIndex++] = sample;
    }

    bool nextFFTBlockReady = false;
    static constexpr auto fftSize = 1 << FFTORDER; // 1024
    std::array<float, fftSize> fifo;
    int fifoIndex = 0;
    std::array<float, fftSize * 2> fftData;


private:

JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrogramFifo)

};

//==============================================================================
class SpectrogramComponent : public juce::Component, private juce::Timer
{
public:
    SpectrogramComponent(SpectrogramFifo* fifo)
        : forwardFFT (FFTORDER),
          spectrogramImage (juce::Image::RGB, 128, 512, true)
    {
        spectrogramFifo = fifo;
        startTimerHz (60);
    }

    // ~SpectrogramComponent() override
    // {

    // }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
		//juce::Rectangle<int> drawArea = getLocalBounds();
		//g.setColour(juce::Colours::black);
		g.fillRoundedRectangle(getLocalBounds().toFloat(),10.f);
        g.drawImage (spectrogramImage, getLocalBounds().reduced(5.f).toFloat());
    }

    void timerCallback() override
    {
        if (spectrogramFifo->nextFFTBlockReady)
        {
            drawNextLineOfSpectrogram();
            spectrogramFifo->nextFFTBlockReady = false;
            repaint();
        }
    }

    void drawNextLineOfSpectrogram()
    {
        auto rightHandEdge = spectrogramImage.getWidth() - 1;
        auto imageHeight   = spectrogramImage.getHeight();

        // first, shuffle our image leftwards by 1 pixel..
        spectrogramImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight);         // [1]

        // then render our FFT data..
        forwardFFT.performFrequencyOnlyForwardTransform (spectrogramFifo->fftData.data());                   // [2]

        // find the range of values produced, so we can scale our rendering to
        // show up the detail clearly
        auto maxLevel = juce::FloatVectorOperations::findMinAndMax (spectrogramFifo->fftData.data(), fftSize / 2); // [3]

        juce::Image::BitmapData bitmap { spectrogramImage, rightHandEdge, 0, 1, imageHeight, juce::Image::BitmapData::writeOnly }; // [4]

        for (auto y = 1; y < imageHeight; ++y)                                              // [5]
        {
            auto skewedProportionY = 1.0f - std::exp (std::log ((float) y / (float) imageHeight) * 0.2f);
            auto fftDataIndex = (size_t) juce::jlimit (0, fftSize / 2, (int) (skewedProportionY * fftSize / 2));
            //auto level = juce::jmap (spectrogramFifo->fftData[fftDataIndex], 0.0f, juce::jmax (maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
            auto level = juce::jmap (spectrogramFifo->fftData[fftDataIndex], 0.f, 1.f, 0.f, 1.0f);
            
            //bitmap.setPixelColour (0, y, juce::Colour::fromHSV (level, 1.0f, level, 1.0f)); // [6]
            bitmap.setPixelColour (0, y, juce::Colour::fromHSV (.3f, 0.5f, level, 1.f)); // [6]
        }
    }

    //static constexpr auto fftOrder = 10;                // [1]
    static constexpr auto fftSize  = 1 << FFTORDER;     // [2]
    juce::Image spectrogramImage;

private:
    SpectrogramFifo* spectrogramFifo;
    juce::dsp::FFT forwardFFT;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrogramComponent)
};
