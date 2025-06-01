/*
--------------------------------------------------------------

SpectrumComponent.h

This file is part of the FxmeJuceTools module for JUCE
O. Doaré - 2025

github.com/odoare/FxmeJuceTools

This spectrogram component was inspired by the JUCE tutorial :
https://juce.com/tutorials/tutorial_simple_fft/

--------------------------------------------------------------
*/

#pragma once
#define SPECTRUMFFTORDER 12
#define FPS 30

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

class SpectrumFifo
{
public:

    SpectrumFifo()
    {
        std::fill (fifo.begin(), fifo.end(), 0.0f);
        std::fill (fftData.begin(), fftData.end(), 0.0f);
    };

    ~SpectrumFifo(){};

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

        fifo[(size_t) fifoIndex++] = gain*sample;
    };

    bool nextFFTBlockReady = false;
    static constexpr auto fftSize = 1 << SPECTRUMFFTORDER;
    std::array<float, fftSize> fifo;
    int fifoIndex = 0;
    std::array<float, fftSize * 2> fftData;
    float gain = 0.05f; // Gain for the FFT data

private:

JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumFifo)

};

//==============================================================================
class SpectrumComponent : public juce::Component, private juce::Timer
{
public:
    SpectrumComponent(SpectrumFifo* fifo,
        int xsize = 128,
        int ysize = 512,
        juce::Colour col = juce::Colours::green)
        : forwardFFT (SPECTRUMFFTORDER)
    {
        spectrumFifo = fifo;
        lineColour = col;
        startTimerHz (FPS);
    };

    // ~SpectrogramComponent() override
    // {

    // }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        juce::Rectangle<int> drawArea = getLocalBounds();

		// ④-B. Fill the background of the rectangular area where the waveform will be drawn in gray.
		g.setColour(juce::Colours::black);
		g.fillRoundedRectangle(drawArea.toFloat(),scopeRoundRadius);

		// ④-B. Assign the area to plot the waveform to the Rectangle<SampleType> type.
		float drawX = (float)drawArea.getX() +5.f;
		float drawY = (float)drawArea.getY() +5.f;
		float drawH = (float)drawArea.getHeight() -10.f;
		float drawW = (float)drawArea.getWidth() -10.f;
		juce::Rectangle<float> scopeRect = juce::Rectangle<float>{ drawX, drawY, drawW, drawH };

		// ④-B. プロットする波形の色を設定する。
		g.setColour(lineColour);

		// ④-B. 波形をプロットする関数を呼び出す。
		//       第5引数の値でY方向のスケールを調整し、第6引数の値でY軸の位置を調整している。
		plot(spectrumFifo->fftData.data(), spectrumFifo->fftData.size()/4, g, scopeRect, 1.f, 0.f);

    };

    void timerCallback() override
    {
        if (spectrumFifo->nextFFTBlockReady)
        {
            drawNextLineOfSpectrum();
            spectrumFifo->nextFFTBlockReady = false;
            repaint();
        }
    };

	void setDisplayGain(float gain)
	{
		displayGain = juce::jlimit(0.05f, 10.0f, gain);
        spectrumFifo->gain = displayGain;
	}

	void mouseDrag(const juce::MouseEvent& e)
    {
        float changeVal = 0.0;
        if(e.getDistanceFromDragStartY() < 0) changeVal = +0.1f; //up
        if(e.getDistanceFromDragStartY() > 0) changeVal = -0.1f; //down
        setDisplayGain(displayGain+changeVal);
    }

    void drawNextLineOfSpectrum()
    {

        // then render our FFT data..
        forwardFFT.performFrequencyOnlyForwardTransform (spectrumFifo->fftData.data());                 // [2]

    };

    //static constexpr auto fftOrder = 10;                // [1]
    static constexpr auto fftSize  = 1 << SPECTRUMFFTORDER;     // [2]

private:

	void plot(const float* data
		, size_t numSamples
		, juce::Graphics& g
		, juce::Rectangle<float> rect
		, float scaler = 1.f
		, float offset = 0.f)
	{
		auto w = rect.getWidth();								// 波形プロットを描画する矩形領域の幅サイズを取得する。
		auto h = rect.getHeight();								// 波形プロットを描画する矩形領域の高さサイズを取得する。
		auto right = rect.getRight();							// 波形プロットを描画する矩形領域の右辺の座標を取得する。
		auto alignedCentre = rect.getBottom() - offset;			// 波形プロットのY軸を配置する座標を保持する。
		auto gain = h * scaler;									// 波形プロットのY方向のゲイン量を保持する。

		// 隣り合うサンプルデータ間の直線を描画する。
		// 変数x1,y1,x2,y2に波形プロットを描画する矩形領域内の座標を代入し、変数tには直線の太さパラメータを代入する。
		for (size_t i = 1; i < numSamples; ++i)
		{
			// juce::jmap関数によってサンプルデータ配列の要素数と矩形領域内のX座標とをマッピングし、要素ごとのX座標を算出する。
			const float x1 = juce::jmap(float(i - 1), float(0), float(numSamples - 1), float(right - w), float(right));
			const float y1 = alignedCentre - gain * juce::jmax(float(-1.0), juce::jmin(float(1.0), data[i - 1] ));
			const float x2 = juce::jmap(float(i), float(0), float(numSamples - 1), float(right - w), float(right));
			const float y2 = alignedCentre - gain * juce::jmax(float(-1.0), juce::jmin(float(1.0), data[i]));
			//const float t = scopeLineWidth;
			g.drawLine(x1, y1, x2, y2, 1.f);
		}
	}

    SpectrumFifo* spectrumFifo;
    juce::dsp::FFT forwardFFT;
    // float hueColour;
	juce::Colour lineColour, backgroundColour;
	float scopeRoundRadius, scopeLineWidth;
    float displayGain = 0.05f; // Typically between 0.1 and 2.0

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumComponent)
};
