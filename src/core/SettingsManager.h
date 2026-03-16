//setting manager .h file
//
#pragma once
#include "SharedData.h"
#include <string>

class SettingsManager
{
private:
  std::string m_filePath;

public:

  //constructor (defaults to settings.txt unless specified otherwise)
  SettingsManager(const std::string &filePath = "data/settings.txt");

  void Load(SharedData &data);
  void Save(const SharedData &data);
};
