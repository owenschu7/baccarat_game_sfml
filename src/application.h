#pragma once

#include <SFML/Graphics.hpp>
#include <memory> // unique_ptr
#include <string>

#include "core/SharedData.h"
#include "core/SettingsManager.h"
#include "Screens/Screen.h"
#include "network/client/NetworkClient.h"


class Application
{
public:
  Application();
  ~Application();

  void run();

private:
  //init functions
  void initWindow();
  void initImGui();
  void initData();
  void loadFonts();

  //main loop phases
  void processEvents();
  void update();
  void render();

  //network
  void processNetwork();

  //game state and UI helpers
  void handleScreenTransitions();
  void applySettingsChanges();
  void drawGlobalDebugUI();

  //utils
  std::string generateRealUUID();

  //core variables
  sf::RenderWindow m_window;
  sf::Clock m_deltaClock; // ImGui needs a clock to track time between frames

  //data and settings
  SharedData m_sharedData;
  SettingsManager m_settings;
  NetworkClient m_network;

  //state machine
  std::unique_ptr<Screen> m_currentScreen;
};
