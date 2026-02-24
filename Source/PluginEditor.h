#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * N5ssChorusEditor
 *
 * Plugin GUI with:
 * - 4 mode selection buttons (1-4) for chorus character
 * - 5 wet level buttons (25%, 50%, 75%, 100%, Bypass)
 * - Custom PNG-based rendering using ImageButtons
 */
//==============================================================================
class N5ssChorusEditor : public juce::AudioProcessorEditor
{
public:
    N5ssChorusEditor(N5ssChorusProcessor&);
    ~N5ssChorusEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    //==========================================================================
    // Processor reference
    //==========================================================================
    N5ssChorusProcessor& proc;
    
    //==========================================================================
    // Mode buttons (1-4) - select chorus character
    //==========================================================================
    juce::ImageButton modeBtn[4];
    
    //==========================================================================
    // Wet level buttons - 25%, 50%, 75%, 100%, Bypass
    //==========================================================================
    juce::ImageButton wetBtn[5];
    
    //==========================================================================
    // Image assets
    //==========================================================================
    juce::Image bgImg;
    
    // Mode button images (1-4)
    juce::Image modeOnImg[4];
    juce::Image modeOffImg[4];
    
    // Wet level button images
    juce::Image wet25OnImg, wet25OffImg;
    juce::Image wet50OnImg, wet50OffImg;
    juce::Image wet75OnImg, wet75OffImg;
    juce::Image wet100OnImg, wet100OffImg;
    juce::Image bypassOnImg, bypassOffImg;
    
    //==========================================================================
    // Helper methods
    //==========================================================================
    void loadImages();
    void setupModeButtons();
    void setupWetButtons();
    void updateModeButtonImages();
    void updateWetButtonImages();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(N5ssChorusEditor)
};
