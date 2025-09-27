//===========================================================
//
// FX-Mechanics gui elements
//
// ==========================================================

#include <JuceHeader.h>

class FxmeKnob : public juce::Component
{
public:

    FxmeKnob(juce::AudioProcessorValueTreeState& apvts, juce::String paramName, juce::Colour knobColor)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,80,15);
        slider.setTextBoxIsEditable(true);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::thumbColourId, knobColor);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::black);
        slider.setColour(juce::Slider::trackColourId, knobColor);
        slider.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, knobColor.darker(2.f));
        addAndMakeVisible(slider);

        textLabel.setJustificationType(juce::Justification::centred);
        if (auto* param = apvts.getParameter(paramName))
            textLabel.setText(param->getName(100), juce::NotificationType::sendNotification);
        else
            textLabel.setText(paramName, juce::NotificationType::sendNotification);
        addAndMakeVisible(textLabel);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts,paramName,slider);
    }

    FxmeKnob(juce::AudioProcessorValueTreeState& apvts, juce::String paramName, juce::String labelText, juce::Colour knobColor)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow,true,80,15);
        slider.setTextBoxIsEditable(true);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::thumbColourId, knobColor);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::black);
        slider.setColour(juce::Slider::trackColourId, knobColor);
        slider.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, knobColor.darker(2.f));
        addAndMakeVisible(slider);

        textLabel.setJustificationType(juce::Justification::centred);
        textLabel.setText(labelText, juce::NotificationType::sendNotification);
        addAndMakeVisible(textLabel);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts,paramName,slider);
    }

    ~FxmeKnob() override
    {
    }

    void resized() override
    {
        juce::FlexBox flexBox;
        flexBox.flexDirection = juce::FlexBox::Direction::column;
        flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
        flexBox.items.add(juce::FlexItem(textLabel).withFlex(0.2f));
        flexBox.items.add(juce::FlexItem(slider).withFlex(1.f));
        flexBox.performLayout(getLocalBounds());
    }

    juce::Slider slider;
    juce::Label valueLabel, textLabel;

private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FxmeKnob)

};
