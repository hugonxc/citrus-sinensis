/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CitrussinensisAudioProcessorEditor::CitrussinensisAudioProcessorEditor (CitrussinensisAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    forwardFFT(10),
    window (1 << 10, juce::dsp::WindowingFunction<float>::hann)
{
    
    setSize (1000, 650);
    startTimerHz (30);
    
    
    //========================= GAIN DIAL =============================
    gainDial.setColour(juce::Slider::thumbColourId, juce::Colour::fromRGB(255, 149, 0)); //#ff9500
    gainDial.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromRGB(70, 80, 88)); //#4e5962
    gainDial.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGB(255, 183, 82)); //#
    gainDial.setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromRGB(255, 149, 0));
    gainDial.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);
    gainDial.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    gainDial.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 50);
    gainDial.setTextValueSuffix(" dB");
    gainDial.setRange(-24.0, 24.0);
    gainDial.setValue(0.01);
    gainDial.addListener(this);
        
    addAndMakeVisible(gainDial);
}

CitrussinensisAudioProcessorEditor::~CitrussinensisAudioProcessorEditor()
{
}

//==============================================================================
void CitrussinensisAudioProcessorEditor::paint (juce::Graphics& g)
{
    //========================= BACKGROUND =============================
    juce::Rectangle<int> left = juce::Rectangle<int>(0, 0, 500, 650);
    juce::Rectangle<int> rigth = juce::Rectangle<int>(500, 0, 500, 650);
    
    g.setGradientFill(juce::ColourGradient(juce::Colour::fromRGB(31, 35, 38), 500, 0, juce::Colour::fromRGB(44, 50, 55), 0, 0, false));
    g.drawRect(left);
    g.fillRect(left);

    g.setGradientFill(juce::ColourGradient(juce::Colour::fromRGB(31, 35, 38), 500, 0, juce::Colour::fromRGB(44, 50, 55), 1000, 0, false));

    g.drawRect(rigth);
    g.fillRect(rigth);
    
    //========================= TITLE =============================
    juce::Font fontTitle ("Futura", "Regular", 72.0f);
    g.setFont(fontTitle);
    g.setColour(juce::Colour::fromRGB(252, 134, 19));
    g.drawText("Citrus", titleLeftRect, juce::Justification::right);

    g.setFont(fontTitle);
    g.setColour(juce::Colour::fromRGB(252, 134, 19));
    g.drawText("Sinensis", titleRightRect, juce::Justification::left);

    juce::Image logo = juce::ImageCache::getFromMemory(BinaryData::orange_png, BinaryData::orange_pngSize).rescaled(100, 100);
    
    g.drawImageAt(logo, titleLogoRect.getX(), titleLogoRect.getY());
    
    
    //========================= SPECTRUM =============================
    
    for (int i = 0; i < audioProcessor.scopeSize; ++i)
    {
        float height = spectrumRect.getHeight();
        
        auto spectrumX  = spectrumRect.getX();
        float spectrumY = spectrumRect.getY();

        float value = juce::jmap(audioProcessor.scopeData[i], 0.0f, 1.0f, (float) 0, height);
        
        juce::Rectangle<int> dataInfo = juce::Rectangle<int>(spectrumX + (i * 15), spectrumY + height, 10, -value);

        if (i <= 12) {
            g.setColour(juce::Colour::fromRGB(206, 120, 0));
        }
        
        if (i > 12 && i <= 24) {
            g.setColour(juce::Colour::fromRGB(229, 135, 3));
        }
        
        if (i > 24 && i <= 36) {
            g.setColour(juce::Colour::fromRGB(255, 149, 0));
        }
        
        if (i > 36 && i <= 48) {
            g.setColour(juce::Colour::fromRGB(255, 183, 82));
        }
        
        if (i > 48 && i <= 60) {
            g.setColour(juce::Colour::fromRGB(255, 196, 113));
        }
        
        
        g.drawRect(dataInfo);
        g.fillRect(dataInfo);
    }
}

void CitrussinensisAudioProcessorEditor::resized()
{
    titleLeftRect = getBounds().removeFromTop(110).removeFromBottom(100).removeFromLeft(450).removeFromRight(300);
    
    titleLogoRect = getBounds().removeFromTop(110).removeFromBottom(100).removeFromLeft(550).removeFromRight(100);

    titleRightRect = getBounds().removeFromTop(110).removeFromBottom(100).removeFromRight(450).removeFromLeft(300);
    
    spectrumRect = getBounds().removeFromTop(420).removeFromBottom(310).removeFromLeft(950).removeFromRight(900);
    
    gainRect = getBounds().removeFromBottom(180).removeFromTop(170).removeFromLeft(600).removeFromRight(200);
    
    gainDial.setBounds(gainRect);
}

//==============================================================================
void CitrussinensisAudioProcessorEditor::timerCallback()
{
    drawNextFrameOfSpectrum();
    audioProcessor.nextFFTBlockReady = false;
    repaint();
}

void CitrussinensisAudioProcessorEditor::drawNextFrameOfSpectrum ()
{
    window.multiplyWithWindowingTable(audioProcessor.fftData, audioProcessor.fftSize);
    
    forwardFFT.performFrequencyOnlyForwardTransform(audioProcessor.fftData);
    
    auto mindB = -100.0f;
    auto maxdB = 0.0f;
    
    for (int i = 0; i < audioProcessor.scopeSize; ++i)
    {
        auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) audioProcessor.scopeSize) * 0.2f);
        auto fftDataIndex = juce::jlimit (0, audioProcessor.fftSize / 2, (int) (skewedProportionX * (float) audioProcessor.fftSize * 0.5f));
        auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (audioProcessor.fftData[fftDataIndex]) - juce::Decibels::gainToDecibels ((float) audioProcessor.fftSize)),mindB, maxdB, 0.0f, 1.0f);
     
        audioProcessor.scopeData[i] = level;
    }
}

//==============================================================================
void CitrussinensisAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
        if (slider == &gainDial)
        {
            audioProcessor.rawVolume = pow(10, gainDial.getValue() / 20);
        }
}
