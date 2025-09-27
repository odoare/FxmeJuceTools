#include <JuceHeader.h>

class FxmeLookAndFeel : public juce::LookAndFeel_V4
{
public:

  void drawRotarySlider(juce::Graphics& g, 
          int x, int y, 
          int width,
          int height, 
          float sliderPos, 
          float rotaryStartAngle, 
          float rotaryEndAngle, 
          juce::Slider& slider) override
  {
    float diameter = 0.9*juce::jmin(width,height);
    float radius = diameter * 0.5;
    float centreX = x + width * 0.5;
    float centreY = y + height * 0.5;
    float rx = centreX - radius;
    float ry = centreY - radius;
    float angle = rotaryStartAngle + (sliderPos * (rotaryEndAngle-rotaryStartAngle));
    float thickness = diameter/15;

    juce::PathStrokeType path{thickness, juce::PathStrokeType::JointStyle::curved, juce::PathStrokeType::EndCapStyle::rounded};

    juce::Rectangle<float> dialArea(rx,ry,diameter,diameter);
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId).brighter(2.f));
    g.drawEllipse(dialArea.reduced(thickness).translated(0.f,-thickness*0.12f),thickness*0.36f);
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    g.fillEllipse(dialArea.reduced(thickness));
    
    g.setColour(slider.findColour(juce::Slider::thumbColourId));

    // Rectangle ?
    juce::Path dialTick;
    juce::Rectangle<float> rect(.25f*thickness,-radius+2.*thickness,.5*thickness,radius*0.2);
    dialTick.addRectangle(rect);
    g.fillPath(dialTick,juce::AffineTransform::rotation(angle).translated(centreX,centreY));

    // // Disc ?
    // juce::Rectangle<float> thumbArea(0.f,-radius+2*thickness,thickness,thickness);
    // g.fillEllipse(thumbArea.transformedBy(juce::AffineTransform::rotation(angle).translated(centreX,centreY)));

    g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
    juce::Path arc1;
    arc1.addArc(centreX-diameter/2, centreY-diameter/2, diameter, diameter, rotaryStartAngle, rotaryEndAngle, true);
    g.strokePath(arc1, path);

    g.setColour(slider.findColour(juce::Slider::trackColourId));
    juce::Path arc2;
    arc2.addArc(centreX-diameter/2, centreY-diameter/2, diameter, diameter, rotaryStartAngle, angle, true);
    g.strokePath(arc2, path);
    
  };

  void drawToggleButton(juce::Graphics &g,
                            juce::ToggleButton &b,
                            bool 	shouldDrawButtonAsHighlighted,
                            bool 	shouldDrawButtonAsDown ) override
  {
    auto bounds = b.getLocalBounds();
    float w = juce::jmin<float>(bounds.getWidth(), bounds.getHeight())*.1f;
    bounds = bounds.reduced(2*w);
    auto isDown = b.getToggleState();
    auto col = b.findColour(juce::ToggleButton::tickColourId);
    float t;
    t = w*.5f;

    if (isDown)
    {
      g.setColour(col.brighter(1.5f));
    }
      else
    {
      g.setColour(col);
    }
    g.drawRoundedRectangle(bounds.toFloat(),w*2,t);

    // if (b.isMouseOver())
    // {
    //   g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(),w*3,juce::Font::bold));
    // }
    // else
    // {
    //   g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(),w*3,juce::Font::plain));
    // }

    if (isDown)
    {
      g.setColour(col.brighter(0.7f));
      g.fillRoundedRectangle(bounds.toFloat().translated(w*.1,w*.1),w*2);
      g.setColour(col.darker(1.f));
      g.drawText(b.getButtonText(),bounds,juce::Justification::centred);
    }
    else
    {
      g.setColour(col.darker(.7f));
      g.fillRoundedRectangle(bounds.toFloat().translated(w*.1,w*.1),w*2);
      g.setColour(col.brighter(1.f));
      g.drawText(b.getButtonText(),bounds,juce::Justification::centred);
    }
  }    
  
private:
    // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxmeKnobLookAndFeel)

};
