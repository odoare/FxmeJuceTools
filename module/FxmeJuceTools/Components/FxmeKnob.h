//===========================================================
//
// FX-Mechanics gui elements
//
// ==========================================================

#include <JuceHeader.h>

// A helper class that inherits from Slider to override its mouse behaviour.
class KnobSlider : public juce::Slider
{
public:
    KnobSlider() = default;

    void mouseDown (const juce::MouseEvent& event) override
    {
        if (event.mods.isRightButtonDown())
        {
            // The AlertWindow must be heap-allocated for async callbacks.
            // JUCE will manage its deletion.
            auto* w = new juce::AlertWindow ("Set Value", "Enter a new value", juce::AlertWindow::QuestionIcon);

            w->addTextEditor ("text", getTextFromValue (getValue()), "Value:");
            w->addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
            w->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));

            // Use a SafePointer to the AlertWindow to avoid dangling pointers in the async callback.
            juce::Component::SafePointer<juce::AlertWindow> safeW (w);

            // Use the static `create` method to build the callback.
            // It returns a raw pointer, and JUCE will manage its deletion.
            auto* callback = juce::ModalCallbackFunction::create ([this, safeW] (int result)
            {
                if (safeW == nullptr)
                    return;

                if (result != 0) // OK button
                {
                    auto newText = safeW->getTextEditorContents ("text");
                    setValue (getValueFromText (newText), juce::sendNotificationSync);
                }
            });

            w->enterModalState (true, callback, true);
        }
        else
        {
            juce::Slider::mouseDown (event);
        }
    }
};

class FxmeKnob : public juce::Component // Note: Consider namespacing your components, e.g., `namespace fxme { ... }`
{
public:
    FxmeKnob(juce::AudioProcessorValueTreeState& apvts, 
             juce::String paramName, 
             juce::String labelText = "", 
             juce::Colour knobColor = juce::Colours::white)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); 
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
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


public:
    KnobSlider slider;
    juce::Label valueLabel, textLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FxmeKnob)

};
