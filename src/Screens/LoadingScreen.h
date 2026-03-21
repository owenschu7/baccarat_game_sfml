#include "Screen.h"
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include "../core/GameEvents.h"

#include "../core/Debug.h"

class LoadingScreen : public Screen
{
private:
  ScreenState nextState = ScreenState::None;
  
  sf::RectangleShape menuBackground;

  // --- UI HELPER METHODS ---

  void drawLoadingText(ImVec2 screenSize)
  {
    // FIX: change variable names
    ImFont* titleFont = ImGui::GetIO().Fonts->Fonts[1];
    ImGui::PushFont(titleFont);
    const char* titleString = "Loading...";
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImVec2 textSize = ImGui::CalcTextSize(titleString);

    float textX = (screenSize.x - textSize.x) * 0.5f;
    float textY = (screenSize.y - textSize.y) * 0.4f;

    ImGui::SetCursorPos(ImVec2(textX, textY));
    ImGui::Text("%s", titleString);
    ImGui::PopFont();
    ImGui::PopStyleColor();
  }
public:

  LoadingScreen(SharedData &sharedData) : Screen(sharedData)
  {
    //set up a dark blue background for the menu
    menuBackground.setSize(sf::Vector2f({1920.0f, 1080.0f}));
    menuBackground.setFillColor(sf::Color(20, 20, 50));
  }

  void handleEvent(const sf::Event& event, sf::RenderWindow& window) override
  {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
    }
  }

  void update() override
  {
    // 1. Setup the invisible full-screen window
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(screenSize);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | 
                                   ImGuiWindowFlags_NoResize | 
                                   ImGuiWindowFlags_NoMove | 
                                   ImGuiWindowFlags_NoBackground |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("Loading Layer", nullptr, windowFlags);

      // 2. Call our neat little helper functions!
    drawLoadingText(screenSize);

    // 3. End the window
    ImGui::End();


    //check inbound data from m_shared and switch when we get a NET_Login_Success
    // 1. Check the Inbox for the server's reply
    while (!m_shared.s_inboundEvents.empty())
    {
      DEBUG_PRINT << "LoadingScreen.h: update(): \n";
      DEBUG_PRINT << "  inbound GameEvent...\n";
      GameEvent incoming = m_shared.s_inboundEvents.front();

      if (incoming.type == EventType::SYS_Connect_Success) 
      {
        DEBUG_PRINT << "  event is SYS_Connect_Success\n";
        m_shared.s_isOnline = true;
        // We got in! Move to the Main Menu.
        nextState = ScreenState::MainMenu;
      }
      else if (incoming.type == EventType::SYS_Connect_Failed)
      {
        DEBUG_PRINT << "  event is SYS_Connect_Failed\n";
        // Connection failed! Go to mainmenu but set isOnline to false.
        // (Optional: Set an error message in SharedData to show the user) maybe create a queue and append each error to teh end
        //m_shared.s_errorMessage = "Failed to connect to server.";
        m_shared.s_isOnline = false;
        nextState = ScreenState::MainMenu;
      }

      // Remove the event so we don't read it again
      m_shared.s_inboundEvents.pop();
    }

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
