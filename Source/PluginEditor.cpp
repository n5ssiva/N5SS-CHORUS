#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
N5ssChorusEditor::N5ssChorusEditor(N5ssChorusProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    // Set plugin window size (matching Figma design)
    setSize(300, 450);
    setResizable(false, false);

    // Load all image assets
    loadImages();
    
    // Setup UI components
    setupModeButtons();
    setupWetButtons();
    
    // Start with 50% wet by default so sound passes through
    proc.wetLevel = WetLevel::Mix50;
    proc.applyPreset();
    updateWetButtonImages();
}

N5ssChorusEditor::~N5ssChorusEditor() {}

//==============================================================================
void N5ssChorusEditor::loadImages()
{
    // Background
    bgImg = juce::ImageCache::getFromMemory(BinaryData::Blank_GUI_png,
                                             BinaryData::Blank_GUI_pngSize);
    
    // Mode buttons 1-4
    modeOnImg[0]  = juce::ImageCache::getFromMemory(BinaryData::_1_ON_png, BinaryData::_1_ON_pngSize);
    modeOffImg[0] = juce::ImageCache::getFromMemory(BinaryData::_1_OFF_png, BinaryData::_1_OFF_pngSize);
    modeOnImg[1]  = juce::ImageCache::getFromMemory(BinaryData::_2_ON_png, BinaryData::_2_ON_pngSize);
    modeOffImg[1] = juce::ImageCache::getFromMemory(BinaryData::_2_OFF_png, BinaryData::_2_OFF_pngSize);
    modeOnImg[2]  = juce::ImageCache::getFromMemory(BinaryData::_3_ON_png, BinaryData::_3_ON_pngSize);
    modeOffImg[2] = juce::ImageCache::getFromMemory(BinaryData::_3_OFF_png, BinaryData::_3_OFF_pngSize);
    modeOnImg[3]  = juce::ImageCache::getFromMemory(BinaryData::_4_ON_png, BinaryData::_4_ON_pngSize);
    modeOffImg[3] = juce::ImageCache::getFromMemory(BinaryData::_4_OFF_png, BinaryData::_4_OFF_pngSize);
    
    // Wet level buttons
    wet25OnImg    = juce::ImageCache::getFromMemory(BinaryData::_25_ON_png, BinaryData::_25_ON_pngSize);
    wet25OffImg   = juce::ImageCache::getFromMemory(BinaryData::_25_OFF_png, BinaryData::_25_OFF_pngSize);
    wet50OnImg    = juce::ImageCache::getFromMemory(BinaryData::_50_ON_png, BinaryData::_50_ON_pngSize);
    wet50OffImg   = juce::ImageCache::getFromMemory(BinaryData::_50_OFF_png, BinaryData::_50_OFF_pngSize);
    wet75OnImg    = juce::ImageCache::getFromMemory(BinaryData::_75_ON_png, BinaryData::_75_ON_pngSize);
    wet75OffImg   = juce::ImageCache::getFromMemory(BinaryData::_75_OFF_png, BinaryData::_75_OFF_pngSize);
    wet100OnImg   = juce::ImageCache::getFromMemory(BinaryData::_100_ON_png, BinaryData::_100_ON_pngSize);
    wet100OffImg  = juce::ImageCache::getFromMemory(BinaryData::_100_OFF_png, BinaryData::_100_OFF_pngSize);
    bypassOnImg   = juce::ImageCache::getFromMemory(BinaryData::bypass_ON_png, BinaryData::bypass_ON_pngSize);
    bypassOffImg  = juce::ImageCache::getFromMemory(BinaryData::bypass_OFF_png, BinaryData::bypass_OFF_pngSize);
}

//==============================================================================
void N5ssChorusEditor::setupModeButtons()
{
    for (int i = 0; i < 4; i++)
    {
        // Set images: normal, over, down (using OFF image for normal, ON for down)
        modeBtn[i].setImages(false, true, true,
                             modeOffImg[i], 1.0f, juce::Colours::transparentBlack,  // normal
                             modeOffImg[i], 1.0f, juce::Colours::transparentBlack,  // over
                             modeOnImg[i], 1.0f, juce::Colours::transparentBlack);  // down

        // Handle click - toggle mode, allow max 2 simultaneous
        modeBtn[i].onClick = [this, i]()
        {
            // Toggle this mode
            proc.modeActive[i] = !proc.modeActive[i];
            
            // Count active modes
            int cnt = 0;
            for (int j = 0; j < 4; j++)
                if (proc.modeActive[j]) cnt++;

            // If more than 2 active, deactivate the first other active one
            if (cnt > 2)
            {
                for (int j = 0; j < 4; j++)
                {
                    if (j != i && proc.modeActive[j])
                    {
                        proc.modeActive[j] = false;
                        break;
                    }
                }
            }

            proc.applyPreset();
            updateModeButtonImages();
        };

        addAndMakeVisible(modeBtn[i]);
    }
    
    updateModeButtonImages();
}

//==============================================================================
void N5ssChorusEditor::setupWetButtons()
{
    // Set initial images for all wet buttons
    wetBtn[0].setImages(false, true, true,
                        wet25OffImg, 1.0f, juce::Colours::transparentBlack,
                        wet25OffImg, 1.0f, juce::Colours::transparentBlack,
                        wet25OnImg, 1.0f, juce::Colours::transparentBlack);
    
    wetBtn[1].setImages(false, true, true,
                        wet50OffImg, 1.0f, juce::Colours::transparentBlack,
                        wet50OffImg, 1.0f, juce::Colours::transparentBlack,
                        wet50OnImg, 1.0f, juce::Colours::transparentBlack);
    
    wetBtn[2].setImages(false, true, true,
                        wet75OffImg, 1.0f, juce::Colours::transparentBlack,
                        wet75OffImg, 1.0f, juce::Colours::transparentBlack,
                        wet75OnImg, 1.0f, juce::Colours::transparentBlack);
    
    wetBtn[3].setImages(false, true, true,
                        wet100OffImg, 1.0f, juce::Colours::transparentBlack,
                        wet100OffImg, 1.0f, juce::Colours::transparentBlack,
                        wet100OnImg, 1.0f, juce::Colours::transparentBlack);
    
    wetBtn[4].setImages(false, true, true,
                        bypassOffImg, 1.0f, juce::Colours::transparentBlack,
                        bypassOffImg, 1.0f, juce::Colours::transparentBlack,
                        bypassOnImg, 1.0f, juce::Colours::transparentBlack);

    // Wet level values corresponding to each button
    const WetLevel levels[5] = {
        WetLevel::Mix25,
        WetLevel::Mix50,
        WetLevel::Mix75,
        WetLevel::Mix100,
        WetLevel::Bypass
    };
    
    for (int i = 0; i < 5; i++)
    {
        // Handle click - mutually exclusive selection
        wetBtn[i].onClick = [this, i, levels]()
        {
            proc.wetLevel = levels[i];
            proc.applyPreset();
            updateWetButtonImages();
        };

        addAndMakeVisible(wetBtn[i]);
    }
    
    updateWetButtonImages();
}

//==============================================================================
void N5ssChorusEditor::updateModeButtonImages()
{
    for (int i = 0; i < 4; i++)
    {
        if (proc.modeActive[i])
        {
            // Show ON image when active
            modeBtn[i].setImages(false, true, true,
                                 modeOnImg[i], 1.0f, juce::Colours::transparentBlack,
                                 modeOnImg[i], 1.0f, juce::Colours::transparentBlack,
                                 modeOnImg[i], 1.0f, juce::Colours::transparentBlack);
        }
        else
        {
            // Show OFF image when inactive
            modeBtn[i].setImages(false, true, true,
                                 modeOffImg[i], 1.0f, juce::Colours::transparentBlack,
                                 modeOffImg[i], 1.0f, juce::Colours::transparentBlack,
                                 modeOnImg[i], 1.0f, juce::Colours::transparentBlack);
        }
    }
    repaint();
}

//==============================================================================
void N5ssChorusEditor::updateWetButtonImages()
{
    // 25%
    if (proc.wetLevel == WetLevel::Mix25)
        wetBtn[0].setImages(false, true, true,
                            wet25OnImg, 1.0f, juce::Colours::transparentBlack,
                            wet25OnImg, 1.0f, juce::Colours::transparentBlack,
                            wet25OnImg, 1.0f, juce::Colours::transparentBlack);
    else
        wetBtn[0].setImages(false, true, true,
                            wet25OffImg, 1.0f, juce::Colours::transparentBlack,
                            wet25OffImg, 1.0f, juce::Colours::transparentBlack,
                            wet25OnImg, 1.0f, juce::Colours::transparentBlack);
    
    // 50%
    if (proc.wetLevel == WetLevel::Mix50)
        wetBtn[1].setImages(false, true, true,
                            wet50OnImg, 1.0f, juce::Colours::transparentBlack,
                            wet50OnImg, 1.0f, juce::Colours::transparentBlack,
                            wet50OnImg, 1.0f, juce::Colours::transparentBlack);
    else
        wetBtn[1].setImages(false, true, true,
                            wet50OffImg, 1.0f, juce::Colours::transparentBlack,
                            wet50OffImg, 1.0f, juce::Colours::transparentBlack,
                            wet50OnImg, 1.0f, juce::Colours::transparentBlack);
    
    // 75%
    if (proc.wetLevel == WetLevel::Mix75)
        wetBtn[2].setImages(false, true, true,
                            wet75OnImg, 1.0f, juce::Colours::transparentBlack,
                            wet75OnImg, 1.0f, juce::Colours::transparentBlack,
                            wet75OnImg, 1.0f, juce::Colours::transparentBlack);
    else
        wetBtn[2].setImages(false, true, true,
                            wet75OffImg, 1.0f, juce::Colours::transparentBlack,
                            wet75OffImg, 1.0f, juce::Colours::transparentBlack,
                            wet75OnImg, 1.0f, juce::Colours::transparentBlack);
    
    // 100%
    if (proc.wetLevel == WetLevel::Mix100)
        wetBtn[3].setImages(false, true, true,
                            wet100OnImg, 1.0f, juce::Colours::transparentBlack,
                            wet100OnImg, 1.0f, juce::Colours::transparentBlack,
                            wet100OnImg, 1.0f, juce::Colours::transparentBlack);
    else
        wetBtn[3].setImages(false, true, true,
                            wet100OffImg, 1.0f, juce::Colours::transparentBlack,
                            wet100OffImg, 1.0f, juce::Colours::transparentBlack,
                            wet100OnImg, 1.0f, juce::Colours::transparentBlack);
    
    // Bypass
    if (proc.wetLevel == WetLevel::Bypass)
        wetBtn[4].setImages(false, true, true,
                            bypassOnImg, 1.0f, juce::Colours::transparentBlack,
                            bypassOnImg, 1.0f, juce::Colours::transparentBlack,
                            bypassOnImg, 1.0f, juce::Colours::transparentBlack);
    else
        wetBtn[4].setImages(false, true, true,
                            bypassOffImg, 1.0f, juce::Colours::transparentBlack,
                            bypassOffImg, 1.0f, juce::Colours::transparentBlack,
                            bypassOnImg, 1.0f, juce::Colours::transparentBlack);
    
    repaint();
}

//==============================================================================
void N5ssChorusEditor::paint(juce::Graphics& g)
{
    // Always fill white background first
    g.fillAll(juce::Colours::white);
    
    // Draw background image on top
    if (bgImg.isValid())
        g.drawImage(bgImg, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
}

//==============================================================================
void N5ssChorusEditor::resized()
{
    // Layout matching Figma mockup exactly
    // Window size: 300 x 450
    
    //==========================================================================
    // Mode buttons (1-4) - horizontal row
    // Size: 40x55, Spacing: 19px, Y position: 115px
    //==========================================================================
    const int modeButtonWidth = 40;
    const int modeButtonHeight = 55;
    const int modeSpacing = 19;
    const int modeTotalWidth = modeButtonWidth * 4 + modeSpacing * 3;
    const int modeStartX = (getWidth() - modeTotalWidth) / 2;
    const int modeStartY = 115;
    
    for (int i = 0; i < 4; i++)
    {
        modeBtn[i].setBounds(
            modeStartX + i * (modeButtonWidth + modeSpacing),
            modeStartY,
            modeButtonWidth,
            modeButtonHeight
        );
    }
    
    //==========================================================================
    // Wet level buttons - 2x2 grid
    // Size: 104x39, Spacing: 8px horizontal, 11px vertical
    // Row 1 Y: 252px, Row 2 Y: 302px
    //==========================================================================
    const int wetButtonWidth = 104;
    const int wetButtonHeight = 39;
    const int wetSpacingX = 8;
    const int wetTotalWidth = wetButtonWidth * 2 + wetSpacingX;
    const int wetStartX = (getWidth() - wetTotalWidth) / 2;
    
    // Row 1: 25% and 50% at Y=252
    wetBtn[0].setBounds(wetStartX, 252, wetButtonWidth, wetButtonHeight);
    wetBtn[1].setBounds(wetStartX + wetButtonWidth + wetSpacingX, 252, wetButtonWidth, wetButtonHeight);
    
    // Row 2: 75% and 100% at Y=302
    wetBtn[2].setBounds(wetStartX, 302, wetButtonWidth, wetButtonHeight);
    wetBtn[3].setBounds(wetStartX + wetButtonWidth + wetSpacingX, 302, wetButtonWidth, wetButtonHeight);
    
    //==========================================================================
    // Bypass button - centered
    // Size: 216x59, Y position: 352px
    //==========================================================================
    const int bypassWidth = 216;
    const int bypassHeight = 59;
    const int bypassX = (getWidth() - bypassWidth) / 2;
    wetBtn[4].setBounds(bypassX, 352, bypassWidth, bypassHeight);
}
