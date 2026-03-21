#include "Screen.h"
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <iostream> // FIX: debug info

class StartScreen : public Screen
{
private:
  ScreenState nextState = ScreenState::None;
  
  sf::RectangleShape menuBackground;

  // --- UI HELPER METHODS ---

  void drawTitle(ImVec2 screenSize)
  {
    ImFont* titleFont = ImGui::GetIO().Fonts->Fonts[1];
    ImGui::PushFont(titleFont);
    const char* titleString = "Baccarat Online";
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImVec2 textSize = ImGui::CalcTextSize(titleString);

    float textX = (screenSize.x - textSize.x) * 0.5f;
    float textY = (screenSize.y - textSize.y) * 0.4f;

    ImGui::SetCursorPos(ImVec2(textX, textY));
    ImGui::Text("%s", titleString);
    ImGui::PopFont();
    ImGui::PopStyleColor();
  }

  void drawButtons(ImVec2 screenSize)
  {
    float buttonWidth = 600.0f;
    float buttonHeight = 80.0f;
    float leftPadding = 40.0f;
    float centerY = screenSize.y * 0.50f; 

    ImFont* titleButtonFont = ImGui::GetIO().Fonts->Fonts[2];
    ImGui::PushFont(titleButtonFont);

    // Move cursor for the first button
    ImGui::SetCursorPosX(leftPadding);
    ImGui::SetCursorPosY(centerY);

    // Push Styles
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(40.0f/255.0f, 150.0f/255.0f, 60.0f/255.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(60.0f/255.0f, 170.0f/255.0f, 80.0f/255.0f, 1.0f)); 
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(20.0f/255.0f, 130.0f/255.0f, 40.0f/255.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 30.0f)); // adds a space vertically
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    // 2. Pass ImVec2(0.0f, 0.0f) to make the button auto-size to the text width and height
    // (If you still want them to be chunky 100px tall buttons, use ImVec2(0.0f, 100.0f) instead!)
    if (ImGui::Button("Start", ImVec2(0.0f, buttonHeight))) 
    {
      //switch to a loading screen
      //connect to the server
      // switch to MainMenu
      nextState = ScreenState::Loading;
    }

    ImGui::SetCursorPosX(leftPadding);
    if (ImGui::Button("Quit", ImVec2(0.0f, buttonHeight))) 
    {
      nextState = ScreenState::Quit;
    }

    // Pop Styles
    ImGui::PopStyleColor(4); 
    ImGui::PopStyleVar(2);  
    ImGui::PopFont();
  }

  void DrawUsername(ImVec2 screenSize)
  {
    static char usernameBuf[32] = "";
    //first we check which AuthState we are in to determine where to draw/update stuff goes

    switch (m_shared.s_AuthState)
    {
      case AuthState::NeedUsername:
        {
          //center the modal to the center of the screen

          float centerX = (screenSize.x / 2.0f);
          float centerY = (screenSize.y / 2.0f);

          //draw the popup to get username info
          // center the popup on the screen
          // to do this we use SetNextWindowPos(), it sets the next location to draw a window at 
          // (in our case its the popup and we want it in the center of the screen)
          ImGui::SetNextWindowPos(ImVec2(centerX, centerY), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

          // 1. Tell ImGui to actually open the modal!
          // The string here MUST match the string in BeginPopupModal exactly.
          ImGui::OpenPopup("WELCOME...");

          // 3. Define the flags to make it static (no moving, no resizing, no collapsing)
          ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoCollapse;

          // 4. Begin the modal
          if (ImGui::BeginPopupModal("WELCOME...", NULL, flags))
          {
            ImGui::Text("Please Enter a Username:");
            ImGui::Separator();

            ImGui::InputText("##username", usernameBuf, IM_ARRAYSIZE(usernameBuf));

            if (ImGui::Button("Enter"))
            {
              m_shared.s_currentUsername = usernameBuf;
              m_shared.s_settingsChanged = true;

              // Move to the next screen!
              m_shared.s_AuthState = AuthState::LoggedIn; 

              ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
          }
          break;
        }
      case AuthState::LoggedIn:
        {
        ImFont* defaultFont = ImGui::GetIO().Fonts->Fonts[0];
        ImGui::PushFont(defaultFont);

        // Combine the label with the actual username 
        // (Assumes m_shared.s_currentUsername is a std::string or const char*)
        std::string displayText = "Username: " + m_shared.s_currentUsername;

        // Calculate size based on the complete text
        ImVec2 textSize = ImGui::CalcTextSize(displayText.c_str());
        
        float bottomLeftPadding = 20.0f; 
        
        // Calculate X and Y positions
        float textX = bottomLeftPadding; 
        float textY = screenSize.y - textSize.y - bottomLeftPadding; 

        ImGui::SetCursorPos(ImVec2(textX, textY));
        
        // Push White text color (R: 1.0, G: 1.0, B: 1.0, Alpha: 1.0)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); 
        
        // Print the final string
        ImGui::Text("%s", displayText.c_str());
        
        ImGui::PopStyleColor();
        ImGui::PopFont();
        break;
      }
      default:
        break;
    }
  }


public:

  StartScreen(SharedData &sharedData) : Screen(sharedData)
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
      if (keyPressed->scancode == sf::Keyboard::Scancode::Enter)
      {
        nextState = ScreenState::MainMenu;
      }
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

    ImGui::Begin("Menu Layer", nullptr, windowFlags);

      // 2. Call our neat little helper functions!
    drawTitle(screenSize);
    drawButtons(screenSize);

    DrawUsername(screenSize); 

    // 3. End the window
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
