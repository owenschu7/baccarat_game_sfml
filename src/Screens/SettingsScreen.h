#include "Screen.h"
#include <SFML/Graphics.hpp>
#include <imgui.h>

class SettingsScreen : public Screen
{
private:
  ScreenState nextState = ScreenState::None;
  
  sf::RectangleShape menuBackground;


  // --- UI HELPER METHODS ---

  void drawSettingsText(ImVec2 screenSize)
  {
    ImFont* bigFont = ImGui::GetIO().Fonts->Fonts[1];
    ImGui::PushFont(bigFont);

    const char* settingsString = "Settings";

    ImVec2 statusSize = ImGui::CalcTextSize(settingsString);
    
    float statusX = (screenSize.x / 2) - (statusSize.x / 2);
    float statusY = (screenSize.y * 0.15f) - (statusSize.y / 2);

    ImGui::SetCursorPos(ImVec2(statusX, statusY));
    ImGui::Text("%s", settingsString);
    ImGui::PopFont();
  }


  void drawButtons(ImVec2 screenSize)
  {
    float backButtonHeight = 60.0f;

    float leftPadding = 60.0f; // More padding to match the image
    float startY = screenSize.y * 0.25f; 
    float labelWidth = 150.0f;  // Space reserved for the "Music", "Sound fx" text
    
    float rightSideStart = screenSize.x - 450.0f;

    ImFont* smallFont = ImGui::GetIO().Fonts->Fonts[0]; // Using Small Font
    ImGui::PushFont(smallFont);

    ImGui::SetCursorPosY(startY);

    // Styling to match the red/white sliders in your image
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.7f, 0.8f, 0.9f, 1.0f)); // Light blue knob
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.3f, 0.3f, 1.0f)); // Red filled part
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 40.0f)); // Vertical gap


    // --- 1. Video Quality (Checkboxes using m_shared) ---
    ImGui::SetCursorPosX(leftPadding);
    ImGui::Text("Video quality");
    ImGui::SameLine(rightSideStart);
    
    ImGui::Text("Low"); ImGui::SameLine();
    if (ImGui::Checkbox("##low", &m_shared.s_videoLow)) { 
      m_shared.s_videoHigh = !m_shared.s_videoLow; 
      m_shared.s_settingsChanged = true;
    }
    
    ImGui::SameLine();
    ImGui::Text("High"); ImGui::SameLine();
    if (ImGui::Checkbox("##high", &m_shared.s_videoHigh)) { 
      m_shared.s_videoLow = !m_shared.s_videoHigh; 
      m_shared.s_settingsChanged = true;
    }

    // --- 2. Music Slider (using m_shared) ---
    ImGui::SetCursorPosX(leftPadding);
    ImGui::Text("Music");
    ImGui::SameLine(rightSideStart);
    ImGui::PushItemWidth(400.0f);
    if (ImGui::SliderFloat("##music", &m_shared.s_musicVolume, 0.0f, 1.0f, ""))
    {
      m_shared.s_settingsChanged = true;
    }
    ImGui::PopItemWidth();

    // --- 3. Sound FX Slider (using m_shared) ---
    ImGui::SetCursorPosX(leftPadding);
    ImGui::Text("Sound fx");
    ImGui::SameLine(rightSideStart);
    ImGui::PushItemWidth(400.0f);
    if (ImGui::SliderFloat("##sfx", &m_shared.s_sfxVolume, 0.0f, 1.0f, ""))
    {
      m_shared.s_settingsChanged = true;
    }
    ImGui::PopItemWidth();

    // --- 4. Language Combo (using m_shared) ---
    ImGui::SetCursorPosX(leftPadding);
    ImGui::Text("Language");
    ImGui::SameLine(rightSideStart);
    const char* languages[] = { "English", "Spanish", "French" };
    ImGui::PushItemWidth(250.0f);
    if (ImGui::Combo("##lang", &m_shared.s_currentLanguage, languages, IM_ARRAYSIZE(languages)))
    {
      m_shared.s_settingsChanged = true;
    }
    ImGui::PopItemWidth();

    // --- 5. Lightmode/darkmode (using m_shared) ---
    ImGui::SetCursorPosX(leftPadding);
    ImGui::Text("Light Mode");
    ImGui::SameLine(rightSideStart);
 
    if (ImGui::Checkbox("##on", &m_shared.s_lightMode)) { 
      // This block ONLY runs on the exact frame the user clicks the box
      m_shared.s_settingsChanged = true;
    }


    // --- 5. Back Button (To return to Menu) ---
    ImGui::SetCursorPosX(leftPadding);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f)); 
    if (ImGui::Button("Back to Menu", ImVec2(0.0f, backButtonHeight))) {
        nextState = ScreenState::MainMenu;
    }

    // Clean up
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(1);
    ImGui::PopFont();
  }


public:

  SettingsScreen(SharedData &sharedData) : Screen(sharedData)
  {
    //set up a dark blue background for the menu
    menuBackground.setSize(sf::Vector2f({1920.0f, 1080.0f}));
    menuBackground.setFillColor(sf::Color(20, 20, 50));
  }

  void handleEvent(const sf::Event& event, sf::RenderWindow& window) override
  {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
      //pressing "Enter" signals we want to switch to the game state
      if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
      {
        nextState = ScreenState::MainMenu;
      }
    }

    //handle mouse clicks
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
      if(mousePressed->button == sf::Mouse::Button::Left)
      {
        //convent the raw pixel coordinates to window corrdinates
        sf::Vector2f mousePos = window.mapPixelToCoords(mousePressed->position);
        
      }
    }
  }

  void update() override
  {
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(screenSize);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | 
                                   ImGuiWindowFlags_NoResize | 
                                   ImGuiWindowFlags_NoMove | 
                                   ImGuiWindowFlags_NoBackground |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("Settings Layer", nullptr, windowFlags);
    drawSettingsText(screenSize);
    drawButtons(screenSize);
    ImGui::End();
  }

  void draw(sf::RenderWindow& window) override
  {
    // draw play button, settings button, background
    window.draw(menuBackground);
  }
  
  ScreenState getNextState() const override
  {
    return nextState;
  }
};
