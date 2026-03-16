#pragma once
#include <SFML/Graphics.hpp>
#include "../core/SharedData.h"

//an enum to help us switch between screens easily
enum class ScreenState
{
  None,
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
  
public:
  virtual ~Screen() = default;

  // member initializer list
  Screen(SharedData& shared) : m_shared(shared) {}

  // every screen must implement these three functions
  virtual void handleEvent(const sf::Event& event, sf::RenderWindow& window) = 0;
  virtual void update() = 0;
  virtual void draw(sf::RenderWindow& window) = 0;

  // this allows a screen to tell the main loop it's time to switch
  virtual ScreenState getNextState() const = 0;
};
