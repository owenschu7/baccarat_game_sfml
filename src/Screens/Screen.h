#pragma once
#include <SFML/Graphics.hpp>
#include "../core/SharedData.h"

//an enum to help us switch between screens easily
enum class ScreenState
{
  None,
  Start,
  Loading,
  MainMenu,
  Settings,
  Tables,
  Game,
  Quit,
  Demo
};

class Screen
{
protected:
  SharedData& m_shared; // Every screen has a reference to the same data
  
  ScreenState m_nextState = ScreenState::None;
  
public:
  virtual ~Screen() = default;

  // member initializer list
  Screen(SharedData& shared) : m_shared(shared) {}

  // every screen must implement these three functions
  virtual void handleEvent(const sf::Event& event, sf::RenderWindow& window) = 0;
  virtual void update() = 0;
  virtual void draw(sf::RenderWindow& window) = 0;

  void processNetworkEvents()
  {
    while (!m_shared.s_inboundEvents.empty())
    {
      GameEvent incoming = m_shared.s_inboundEvents.front();
      m_shared.s_inboundEvents.pop();

      // GLOBAL EVENTS: You can handle things every screen cares about here!
      if (incoming.type == EventType::SYS_Disconnect) {
          m_shared.s_isOnline = false;
          m_nextState = ScreenState::Start; // Kick them to the start screen
          continue;
      }
      if (incoming.type == EventType::SYS_Connect) {
          m_shared.s_isOnline = true;
          m_nextState = ScreenState::MainMenu;
        std::cout << "sys_connect\n";
          continue;
      }

      // Route the event to the specific active screen
      onNetworkEvent(incoming);
    }
  }
  virtual void onNetworkEvent(const GameEvent& event) {}

  // this allows a screen to tell the main loop it's time to switch
  virtual ScreenState getNextState() const = 0;
};
