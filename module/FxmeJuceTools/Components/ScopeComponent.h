/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

// JUCEライブラリのヘッダーをインクルードする。
#include <array>

#define FPS 30
#define ORDER 11

//==============================================================================
// ① A queue-based data structure class that holds sample data to be displayed
// in the scope. Define it as a template class.
template <typename SampleType>
class AudioBufferQueue
{
public:
	// バッファサイズが512の時, サンプリングレート96kHz下での440Hzの正弦波を2周+αまで描画することができる
	// サンプリングレート96kHz(⊿T = 0.01[ms])下での440Hzの正弦波(2.27[ms]) => 1バッファ当たり5.12[ms](= 0.01 * 512)の波形を格納できる
	// バッファサイズが512の時, サンプリングレート48kHz下での440Hzの正弦波を4周+αまで描画することができる
	// サンプリングレート48kHz(⊿T = 0.02[ms])下での440Hzの正弦波(2.27[ms]) => 1バッファ当たり10.24[ms](= 0.02 * 512)の波形を格納できる

	// ② Declare constants used within the class.
	//static constexpr size_t order = ORDER;						// Byte order for specifying buffer size
	static constexpr size_t bufferSize = 1U << ORDER;		// Number of samples for a single element in the container of the audio buffer for plotting(512)
	static constexpr size_t numBuffers = 5;					// Number of container elements for plotting audio buffer held in this class

	// ③A function that performs push operations using the FIFO method.
	// Add a plotting audio buffer to the last element of the container.
	void push(const SampleType* dataToPush, size_t numSamples)
	{

		jassert(numSamples <= bufferSize);

		// Declare variables to which the index value of the container element to be written
		// and the number of elements to be written are assigned.
		int start1, size1, start2, size2;

		// FIFO方式でのコンテナ要素の追加操作を行うための準備処理。
		// juce::AbstractFifoクラスのオブジェクトから、コンテナ実体に対して書き込み操作を行うインデックス値と要素数を取得する。
		// コンテナの空要素が不足している場合にはインデックス値=0,要素数=0が返る。その場合には要素の追加操作は行わない。
		abstractFifo.prepareToWrite(1, start1, size1, start2, size2);

		jassert(size1 <= 1);
		jassert(size2 == 0);

		if (size1 > 0) {
			// 浮動小数点型(floatまたはdouble)の配列要素をコピーする処理。内部ではmemcpy関数が実行される。
			juce::FloatVectorOperations::copy(buffers[(size_t)start1].data(), dataToPush, (int)juce::jmin(bufferSize, numSamples));
		}

		// FIFO方式でのコンテナ要素の追加操作を行った後の終了処理。
		// 書き込み操作を行った要素数の値だけ、最後尾のインデックス値を移動させる。
		abstractFifo.finishedWrite(size1);
	}


	// ④FIFO方式でのポップ操作を行う関数。コンテナの先頭要素からプロット用オーディオバッファを取り出す。
	void pop(SampleType* outputBuffer)
	{
		// 読み込み操作を行うコンテナ要素のインデックス値と書き込み操作を行う要素数を代入する変数を宣言する。
		int start1, size1, start2, size2;

		// FIFO方式でのコンテナ要素の取り出し操作を行うための準備処理。
		// juce::AbstractFifoクラスのオブジェクトから、コンテナ実体に対して読み込み操作を行うインデックス値と要素数を取得する。
		// 取り出し可能な要素が不足している場合にはインデックス値=0,要素数=0が返る。その場合には要素の読み込み操作は行わない。
		abstractFifo.prepareToRead(1, start1, size1, start2, size2);

		jassert(size1 <= 1);
		jassert(size2 == 0);

		if (size1 > 0) {
			// 浮動小数点型(floatまたはdouble)の配列要素をコピーする処理。内部ではmemcpy関数が実行される。
			juce::FloatVectorOperations::copy(outputBuffer, buffers[(size_t)start1].data(), (int)bufferSize);
		}

		// FIFO方式でのコンテナ要素の読み込み操作を行った後の終了処理。
		// 読み込み操作を行った要素数の値だけ、先頭のインデックス値を移動させる。
		abstractFifo.finishedRead(size1);
	}

private:
	// ⑤メンバ変数を宣言する。
	// プロット用オーディオバッファを保持するコンテナ。サンプルデータを保持する配列をさらに配列化している。
	std::array <std::array<SampleType, bufferSize>, numBuffers> buffers;

	// FIFO方式のアルゴリズムを提供するjuce::AbstractFifoクラスのインスタンス。
	// コンストラクタの引数としてFIFO方式で操作するコンテナの要素数を渡す。
	juce::AbstractFifo abstractFifo{ numBuffers };

};

//==============================================================================
// ① A class that collects sample data from the DSP audio buffer and adds the
// sample data to the AudioBufferQueue object.
template<typename SampleType>
class ScopeDataCollector
{
public:
	// ②引数付きコンストラクタ。引数としてAudioBufferQueueオブジェクトの参照を受け取る。
	ScopeDataCollector(AudioBufferQueue<SampleType>& queueToUse)
		: audioBufferQueue(queueToUse)
	{}

	// ③DSPのオーディオバッファからサンプルデータを回収して、AudioBufferQueueオブジェクトにサンプルデータをプッシュする関数。
	//   第一引数：DSPのオーディオバッファのポインタ,第二引数：DSPのオーディオバッファに含まれるサンプル数
	void process(const SampleType* data, size_t numSamples)
	{
		size_t index = 0;

		// ③-A."WaitingForTrigger"モード時に実行される処理。
		if (currentState == State::WaitingForTrigger)
		{
			while (index++ < numSamples)
			{
				// オーディオバッファからサンプルデータを1つ取得して一時変数に代入するとともにポインタをインクリメントする。
				auto currentSample = *data++;
				// 前後のサンプルデータによる波形がトリガーレベルと交差する場合"Collecting"モードに移行する。
				if (currentSample >= triggerLevel && prevSample < triggerLevel)
				{
					numCollected = 0;						// メンバ変数でキャッシュしているサンプルのカウントをリセットする。
					currentState = State::Collecting;	// 動作モードを"Collecting"に移行する。
					break;										// whileループを抜ける。
				}
				prevSample = currentSample;				// 今回処理したサンプルデータを一時的に保持する。

			}	// >> while (index++ < numSamples) 
		}	// >> if (currentState == State::WaitingForTrigger)

		// ③-B."Collecting"モード時に実行される処理
		if (currentState == State::Collecting)
		{
			while (index++ < numSamples)
			{
				// サンプルデータをキャッシュするとともに、サンプルのカウント数とサンプルデータのポインタをインクリメントする。
				buffer[numCollected++] = *data++;

				// キャッシュしているサンプルのカウント数がキャッシュ用のコンテナのサイズと一致する場合、
				// キャッシュしているコンテナをAudioBufferQueueオブジェクトにプッシュするとともに、動作モードを元に戻す。
				if (numCollected == buffer.size())
				{
					audioBufferQueue.push(buffer.data(), buffer.size());	// AudioBufferQueueオブジェクトにプッシュする。
					currentState = State::WaitingForTrigger;				// 動作モードを"WaitingForTrigger"に移行する。
					prevSample = SampleType(100);							// 前回のサンプルデータとして仮の値を代入する。
					break;													// whileループを抜ける。
				}
			}
		}

	}	// >> void process(const SampleType* data, size_t numSamples)

private:
	// ③-C."ScopeDataCollector::process"関数内における動作モードの名前および判定フラグを列挙型で宣言する。
	enum class State
	{
		WaitingForTrigger,
		Collecting
	};

	// ④クラス内で使用する定数を宣言する。
	static constexpr auto triggerLevel = SampleType(0.001);

	// ⑤メンバ変数を宣言する。
	std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize> buffer;	// サンプルデータをキャッシュするコンテナ
	State currentState{ State::WaitingForTrigger };								// 現在の動作モードを保持する変数
	AudioBufferQueue<SampleType>& audioBufferQueue; 							// AudioBufferQueueクラスのオブジェクトへの参照
	size_t numCollected;														// バッファにセットしたサンプルの数を監視するための変数
	SampleType prevSample = SampleType(100);									// 前回のサンプルデータの値を保持する変数
};

//==============================================================================
// ①A GUI component class that plots and draws sample data stored in the AudioBufferQueue object.
//   Inheriting classes: juce::Component class, juce::Timer class
template<typename SampleType>
class ScopeComponent : public juce::Component, private juce::Timer
{
public:

	// Write the using declaration of AudioBufferQueue class.
	using Queue = AudioBufferQueue<SampleType>;

	// ②Constructor with arguments. Receive the AudioBufferQueue class reference
	// as an argument and assign it to the member variable.
	ScopeComponent(Queue& queueuToUse
					, juce::Colour colour
					, juce::Colour background
					, float radius=10.f
					, float width=1.f)
		: audioBufferQueue(queueuToUse)
	{
		lineColour = colour;
		backgroundColour = background;
		scopeRoundRadius = radius;
		scopeLineWidth = width;
		sampleData.fill(SampleType(0));
		setFramePerSecond(FPS);
		addMouseListener(this, true); // Enable mouse drag events
	}

	// ③ A function that sets the time interval for updating the waveform plot.
	void setFramePerSecond(int framePerSecond)
	{
		jassert(framePerSecond > 0 && framePerSecond < 1000);
		startTimerHz(framePerSecond);
	}

	// ④Function to draw the state of the SCOPE panel.
	// Executes the process of filling the panel area and plotting the waveform.
	void paint(juce::Graphics& g) override
	{
		// ④-B. Identify the rectangular area to draw the waveform.
		juce::Rectangle<int> drawArea = getLocalBounds();

		// ④-B. Fill the background of the rectangular area where the waveform will be drawn in gray.
		g.setColour(backgroundColour);
		g.fillRoundedRectangle(drawArea.toFloat(),scopeRoundRadius);

		// ④-B. Assign the area to plot the waveform to the Rectangle<SampleType> type.
		SampleType drawX = (SampleType)drawArea.getX();
		SampleType drawY = (SampleType)drawArea.getY();
		SampleType drawH = (SampleType)drawArea.getHeight();
		SampleType drawW = (SampleType)drawArea.getWidth();
		juce::Rectangle<SampleType> scopeRect = juce::Rectangle<SampleType>{ drawX, drawY, drawW, drawH };

		// ④-B. プロットする波形の色を設定する。
		g.setColour(lineColour);

		// ④-B. 波形をプロットする関数を呼び出す。
		//       第5引数の値でY方向のスケールを調整し、第6引数の値でY軸の位置を調整している。
		plot(sampleData.data(), sampleData.size(), g, scopeRect, SampleType(0.4), scopeRect.getHeight() / 2);

	}

	void resized() override {}

	void mouseDrag(const juce::MouseEvent& e)
    {
        float changeVal = 0.0;
        if(e.getDistanceFromDragStartY() < 0) changeVal = +0.1f; //up
        if(e.getDistanceFromDragStartY() > 0) changeVal = -0.1f; //down
        setDisplayGain(displayGain+changeVal);

    }

private:
	// ⑤juce::Timerクラスのスレッドから呼び出されるコールバック関数。
	void timerCallback() override
	{
		audioBufferQueue.pop(sampleData.data());			// AudioBufferQueueオブジェクトからサンプルデータの配列を取り出す
		repaint();											// paint関数を実行するフラグを立てる
	}

	// ⑥サンプルデータの配列コンテナから折れ線グラフとして波形をプロットする。
	// 第1引数...プロットするサンプルデータ配列、第2引数...データ配列の要素数
	// 第3引数...グラフィックコンテキストの参照、第4引数...波形プロット結果を描画する矩形領域
	// 第5引数...Y方向での波形大きさを調整する値、第6引数...矩形領域の底部を基準としてY方向に波形をずらすオフセット値
	void plot(const SampleType* data
		, size_t numSamples
		, juce::Graphics& g
		, juce::Rectangle<SampleType> rect
		, SampleType scaler = SampleType(1)
		, SampleType offset = SampleType(0))
	{
		auto w = rect.getWidth();								// 波形プロットを描画する矩形領域の幅サイズを取得する。
		auto h = rect.getHeight();								// 波形プロットを描画する矩形領域の高さサイズを取得する。
		auto right = rect.getRight();							// 波形プロットを描画する矩形領域の右辺の座標を取得する。
		auto alignedCentre = rect.getBottom() - offset;			// 波形プロットのY軸を配置する座標を保持する。
		auto gain = displayGain * h * scaler;									// 波形プロットのY方向のゲイン量を保持する。


		// 隣り合うサンプルデータ間の直線を描画する。
		// 変数x1,y1,x2,y2に波形プロットを描画する矩形領域内の座標を代入し、変数tには直線の太さパラメータを代入する。
		for (size_t i = 1; i < numSamples; ++i)
		{
			// juce::jmap関数によってサンプルデータ配列の要素数と矩形領域内のX座標とをマッピングし、要素ごとのX座標を算出する。
			const float x1 = juce::jmap(SampleType(i - 1), SampleType(0), SampleType(numSamples - 1), SampleType(right - w), SampleType(right));
			const float y1 = alignedCentre - gain * juce::jmax(SampleType(-1.0), juce::jmin(SampleType(1.0), data[i - 1] ));
			const float x2 = juce::jmap(SampleType(i), SampleType(0), SampleType(numSamples - 1), SampleType(right - w), SampleType(right));
			const float y2 = alignedCentre - gain * juce::jmax(SampleType(-1.0), juce::jmin(SampleType(1.0), data[i]));
			//const float t = scopeLineWidth;
			g.drawLine(x1, y1, x2, y2, 1.f);
		}
	}

	void setDisplayGain(float gain)
	{
		displayGain = juce::jlimit(1.f, 10.0f, gain);
	}

	// ⑦メンバ変数を宣言する。
	Queue& audioBufferQueue;										// AudioBufferQueueクラスの参照を保持する変数
	std::array<SampleType, Queue::bufferSize> sampleData;			// プロットするサンプルデータを格納する配列コンテナ

	juce::Colour lineColour, backgroundColour;
	float scopeRoundRadius, scopeLineWidth;
	SampleType displayGain = 1.0;	// Gain value for displaying the waveform, typically between 1 and 10

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopeComponent)
};
