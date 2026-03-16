#include <SFML/Graphics.hpp>
#include <SFML/Config.hpp>
#include <iostream>
#include <imgui.h>
#include <imgui-SFML.h>
#include <memory>
#include "core/SharedData.h"
#include "core/SettingsManager.h"
#include "Screens/Screen.h"
#include "Screens/MainMenuScreen.h"
#include "Screens/GameScreen.h"
#include "Screens/SettingsScreen.h"
#include "Screens/demoScreen.h"
#include "Screens/availableTablesScreens.h"
//include other screens here
#include "uuid.h"

std::string GenerateRealUUID() 
{
  // 1. Create a secure random number engine
  std::random_device rd;
  std::mt19937 gen(rd());
  
  // 2. Pass the engine to stduuid's random generator
  uuids::uuid_random_generator uuid_gen{gen};
  uuids::uuid const id = uuid_gen();
  
  // 3. Convert to string
  return uuids::to_string(id);
}

void applyChanges(SharedData &data);

int main()
{
  //debug info
  std::cout << "SFML Version: " 
              << SFML_VERSION_MAJOR << "." 
              << SFML_VERSION_MINOR << "." 
              << SFML_VERSION_PATCH << std::endl;

  //            sf::RenderWindow: this created the actual window on your operating system.
  //                              you are passing it a VideoMode and the title that appears at the top 
  //                              of the window. the u just tells C++ that these are "unsigned" integers
  auto window = sf::RenderWindow(sf::VideoMode({1920u, 1080u}), "Baccarat Online");

  //IMGUI setup
  if (!ImGui::SFML::Init(window, false))
  {
    // Initialization failed! We can't safely use ImGui.
    // Return a non-zero value to indicate an error to the operating system.
    return -1; 
  }

  // --- LOAD CUSTOM FONT ---
  ImGuiIO& io = ImGui::GetIO();
  
  // load sharedData (for settings and account info)
  SharedData sharedData; // Lives here for the whole program

  // initialize manager to find data at specific path
  SettingsManager settings("data/settings.txt");
  settings.Load(sharedData); // loads data from settings.txt and stores it in m_shared

  // create a UUID if there is none
  if (sharedData.s_currentUUID.empty()) 
  {
    // Generate a new local UUID for the offline client
    sharedData.s_currentUUID = GenerateRealUUID();
    
    // Save the new UUID immediately so it persists next time
    settings.Save(sharedData);

    sharedData.s_AuthState = AuthState::NeedUsername;
  }
  else
  {
    sharedData.s_AuthState = AuthState::LoggedIn;
  }

  // settingsChanged update logic
  // force a change so we get changes to load to screen
  sharedData.s_settingsChanged = true;

  // Load the font at size 40.0f pixels. 
  // ImGui rasterizes fonts at a specific size, so it's best to load it at the size you want to display it!

  // Load the default font (Slot 0)
  ImFont* defaultFont = io.Fonts->AddFontFromFileTTF("assets/fonts/8bitOperatorPlus8-Regular.ttf", 20.0f);
  if (!defaultFont) 
  {
      std::cerr << "Failed to load the BIG ImGui font. Check the file path!\n";
      return -1; // Stops the g.WithinFrameScope crash!
  }
  
  // Load the big font (Slot 1)
  ImFont* bigFont = io.Fonts->AddFontFromFileTTF("assets/fonts/8bitOperatorPlus8-Regular.ttf", 70.0f);
  if (!bigFont) 
  {
      std::cerr << "Failed to load the SMALL ImGui font. Check the file path!\n";
      return -1; // Stops the g.WithinFrameScope crash!
  }
  
  // Load the big font (Slot 2)
  ImFont* titleButtonFont = io.Fonts->AddFontFromFileTTF("assets/fonts/8bitOperatorPlus8-Regular.ttf", 40.0f);
  if (!titleButtonFont) 
  {
      std::cerr << "Failed to load the titleButtonFont ImGui font. Check the file path!\n";
      return -1; // Stops the g.WithinFrameScope crash!
  }



  // CRITICAL: You must tell ImGui-SFML to rebuild the font texture after adding a new font
  if (!ImGui::SFML::UpdateFontTexture()) 
  {
      std::cerr << "Failed to update ImGui font texture!\n";
  }
  // -------------------------
  
  //without this your program will run as fast as your cpu possiblly can
  //this caps it at 60fps
  window.setFramerateLimit(60);

  //create a virtual camera locked exactaly to 1920x1080
  sf::View view(sf::FloatRect({0.f, 0.f}, {1920.f, 1080.f}));
  //tell the window to look through this camera
  window.setView(view);

  //start by loading the main menu
  //currentScreen will hold the current screen we are on (main menu, game screen, settings menu etc)
  std::unique_ptr<Screen> currentScreen = std::make_unique<MainMenuScreen>(sharedData);

  // ImGui needs a clock to track time between frames
  sf::Clock deltaClock;

  // the main game loop inside this is where two things happen, Handling events and Rendering
  while (window.isOpen())
  {
    // the event loop (input)
    // these actions are queued up as "events"
    //                                 window.pollEvent() - this grabs the oldest event waiting in the queue
    //                                                      it returns a std::optional, which basically means
    //                                                      "here is an event, or here is nothing(if the queue is empty)"
    //while () - because the user might press a key
    //           and move at the exact same time there
    //           might be multiple events waiting,
    //           this inner loop processes all pending
    //           events before moving on
    while (const std::optional event = window.pollEvent())
    {
      //pass the event to imgui so it can catch mouse clicks, typing, etc
      ImGui::SFML::ProcessEvent(window, *event);

      // (event->is<sf::Event::Closed>()) - this checks if the current event is the user trying to close the
      //                                    window. if it is, you call window.close() which breaks the main
      //                                    game loop on the next pass and cleanly ends the program
      if (event->is<sf::Event::Closed>())
      {
        window.close();
      }
      if (const auto *resized = event->getIf<sf::Event::Resized>())
      {
        //update the view to match the new window size exactaly
        sf::FloatRect visibleArea({0.f, 0.f}, 
                                  {static_cast<float>(resized->size.x), 
                                   static_cast<float>(resized->size.y)});
        window.setView(sf::View(visibleArea));
      }

      // pass the event down to whatever screen is currently active
      currentScreen->handleEvent(*event, window);
    }

    // Update ImGui BEFORE your screens update, so your screens can build UI
    ImGui::SFML::Update(window, deltaClock.restart());

    //global state watcher
    // this is where the changes (if any) will be applied to the game
    // this only needs to happen AFTER settings are changed (not every frame)
    if (sharedData.s_settingsChanged)
    {
      applyChanges(sharedData);

      //save the data to the file for next session
      settings.Save(sharedData);
      
      //after we apply changes we set it to false to catch for other changes in
      // the future so that we dont constantly waste compute
      sharedData.s_settingsChanged = false;
    }

    //state switching logic
    // on every single frame, the manager asks the active screen
    // do you want to switch to a different screen?
    // if the user pressed somithing which triggers its internal state to change screens
    // then the manager will switch to the nextState
    ScreenState nextState = currentScreen->getNextState();
    if (nextState != ScreenState::None)
    {
      switch (nextState)
      {
        case ScreenState::MainMenu:
          currentScreen = std::make_unique<MainMenuScreen>(sharedData);
          break;

        case ScreenState::Settings:
          currentScreen = std::make_unique<SettingsScreen>(sharedData);
          break;

        case ScreenState::Tables:
          currentScreen = std::make_unique<AvailableTablesScreen>(sharedData);
          break;

        case ScreenState::Game:
          currentScreen = std::make_unique<GameScreen>(sharedData);
          break;

        case ScreenState::Demo:
          currentScreen = std::make_unique<DemoScreen>(sharedData);
          break;

        case ScreenState::Quit:
          window.close();
          break;

        default:
          // This catches 'None' or any states we haven't handled yet
          break;
      }
    }

    //update and draw the active screen
    currentScreen->update();


    // global debug overlay
    // runs after the screen updates so it draws on top of everything!
    //displays on top left corner
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
    // Make it an invisible floating window with no title bar or background
    ImGuiWindowFlags overlayFlags = ImGuiWindowFlags_NoDecoration | 
                                    ImGuiWindowFlags_NoBackground | 
                                    ImGuiWindowFlags_AlwaysAutoResize | 
                                    ImGuiWindowFlags_NoMove | 
                                    ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("Global Debug", nullptr, overlayFlags);
    
    // Draw the checkbox and bind it directly to your SharedData boolean
    ImGui::Checkbox("Show Demo", &sharedData.s_demoWindow);
    
    ImGui::End();

    // 3. Draw the actual Demo Window if the checkbox is ticked!
    if (sharedData.s_demoWindow)
    {
        ImGui::ShowDemoWindow(&sharedData.s_demoWindow);
    }

    //SFML uses a technique called Double Buffering.
    // Imagine an artist painting on a canvas hidden bedind a curtain, and when they are done,
    // they swap, it with the canvas the aduience is currently looking at

    // this wipes the hidden canvas clean by default it clears to black.
    // if you don't do this your new frame will just smear on top of the old frame
    window.clear();

    currentScreen->draw(window);

    // Render the UI on top of whatever the current screen just drew
    ImGui::SFML::Render(window);

    // this drops the curtain and swaps the hidden canvas (which you just drew on) with the visible one on your monitor
    window.display();
  }

  // Clean up resources when the window is finally closed
  ImGui::SFML::Shutdown();
  return 0;
}

void applyChanges(SharedData &data)
{
  //check each data point and change it globally
  data.s_lightMode ? ImGui::StyleColorsLight() : ImGui::StyleColorsDark();


  // --- FUTURE ADDITIONS GO HERE ---
  // For example, when you add audio, you can update it instantly like this:
  // sf::Listener::setGlobalVolume(data.s_musicVolume * 100.f);
}

