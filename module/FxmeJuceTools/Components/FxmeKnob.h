//===========================================================
//
// FX-Mechanics gui elements
//
// ==========================================================

#include <JuceHeader.h>

class FxmeKnob : public juce::Component // Note: Consider namespacing your components, e.g., `namespace fxme { ... }`
{
public:
    FxmeKnob(juce::AudioProcessorValueTreeState& apvts, 
             juce::String paramName, 
             juce::String labelText = "", 
             juce::Colour knobColor = juce::Colours::white)
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

        auto textForLabel = labelText;
        if (textForLabel.isEmpty())
            if (auto* param = apvts.getParameter(paramName))
                textForLabel = param->getName(100);

        if (!labelText.isEmpty())
        {
            textLabel.setJustificationType(juce::Justification::centred);
            textLabel.setText(labelText, juce::NotificationType::sendNotification);
            addAndMakeVisible(textLabel);
        }
        
        // Only create an attachment if the parameter exists. Master knobs won't have one.
        if (apvts.getParameter(paramName) != nullptr)
            attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts,paramName,slider);
    }

    ~FxmeKnob() override
    {
    }

    void resized() override
    {
        // For rotary knobs, use the flexbox to position the label and slider.
        juce::FlexBox flexBox;
        flexBox.flexDirection = juce::FlexBox::Direction::column;
        flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;

        if (textLabel.isVisible())
            flexBox.items.add(juce::FlexItem(textLabel).withFlex(0.2f));
        flexBox.items.add(juce::FlexItem(slider).withFlex(1.0f));
        flexBox.performLayout(getLocalBounds());
    }

    void setLookAndFeel(juce::LookAndFeel* newLookAndFeel)
    {
        slider.setLookAndFeel(newLookAndFeel);
    }

    juce::Slider slider;
    juce::Label valueLabel, textLabel;

private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FxmeKnob)

};
