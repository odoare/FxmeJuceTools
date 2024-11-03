//===========================================================
//
// FX-Mechanics gui elements
//
// ==========================================================


class FxmeButton
{

public:

    FxmeButton(juce::AudioProcessorValueTreeState& apvts
                , juce::String paramName = "dummy"
                , juce::Colour colour = juce::Colours::white)
    {
        button.setColour(juce::ToggleButton::tickColourId,colour);
        button.setButtonText(paramName);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts,paramName,button);
        setFlex();
    }

    ~FxmeButton()
    {
    }

    juce::FlexBox& flex()
    {
        return flexBox;
    }

    juce::ToggleButton button;
    juce::FlexBox flexBox;

private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

    void setFlex()
    {
        flexBox.flexDirection = juce::FlexBox::Direction::row;
        //flexBox.items.add(juce::FlexItem(textLabel).withFlex(1.0f));
        flexBox.items.add(juce::FlexItem(button).withFlex(1.f));
    }

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FxmeButton)

};

