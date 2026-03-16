//Settings manager .cpp file
//
#include "SettingsManager.h"
#include <fstream>
#include <iostream>

SettingsManager::SettingsManager(const std::string &filePath) : m_filePath(filePath) {}

void SettingsManager::Save(const SharedData &data)
{
  std::ofstream file(m_filePath);

  if (file.is_open())
  {
    file << "UUID=" << data.s_currentUUID << "\n";
    file << "username=" << data.s_currentUsername << "\n";
    file << "lightMode=" << data.s_lightMode << "\n";
    file << "musicVolume=" << data.s_musicVolume << "\n";
    file << "sfxVolume=" << data.s_sfxVolume << "\n";
    file << "videoQuality=" << data.s_videoQuality << "\n";
    file << "language=" << data.s_currentLanguage << "\n";

    file.close();
  } else {
    std::cerr << "Failed to open " << m_filePath << " for writing.\n";
  }
}

void SettingsManager::Load(SharedData &data)
{
  std::ifstream file(m_filePath);
  std::string line;

  if (file.is_open())
  {
    while (std::getline(file, line))
    {
      if (line.find("UUID=") == 0)
      {
        data.s_currentUUID =line.substr(5);
      }
      else if (line.find("username=") == 0)
      {
        data.s_currentUsername = line.substr(9);
      }
      else if (line.find("lightMode=") == 0)
      {
        std::string val = line.substr(10);
        if (!val.empty()) data.s_lightMode = std::stoi(val);
      }
      else if (line.find("musicVolume=") == 0)
      {
        std::string val = line.substr(12);
        if (!val.empty()) data.s_musicVolume = std::stof(val);
      }
      else if (line.find("sfxVolume=") == 0)
      {
        std::string val = line.substr(10);
        if (!val.empty()) data.s_sfxVolume = std::stof(val);
      }
      else if (line.find("videoQuality=") == 0)
      {
        std::string val = line.substr(13);
        if (!val.empty()) data.s_videoQuality = std::stoi(val);
      }
      else if (line.find("language=") == 0)
      {
        std::string val = line.substr(9);
        if (!val.empty()) data.s_currentLanguage = std::stoi(val);
      }
    }
    file.close();
  } else {
    std::cerr << "No settings file found at " << m_filePath << " using defaults.\n";
  }
}

