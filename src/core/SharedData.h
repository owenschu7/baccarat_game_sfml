#pragma once
#include <string>

enum class AuthState
{
  CheckingFiles,   // Looking for player_data.json
  NeedUsername,    // First time playing
  LoggedIn,        // Logged in (playing the game)
  LinkingAccount   // User clicked the "Secure Account" button
};

struct SharedData {

  bool s_settingsChanged = false; //flag to update screen when changes occur (this should turn true when any change has been made, and false when we update the changes)

  float s_musicVolume = 0.5f;
  float s_sfxVolume = 0.3f;
  bool s_videoQuality = 0; // 0 for low 1 for high
  bool s_videoHigh = false; // FIX: remove these later
  bool s_videoLow = true;
  int s_currentLanguage = 0;

  bool s_lightMode = false; // true for lightmode false for dark mode

  // authentication data
  AuthState s_AuthState = AuthState::CheckingFiles;
  std::string s_currentUUID = "";
  std::string s_currentUsername = "";

  bool s_demoWindow = false; // displays the demo window for debugging

};


