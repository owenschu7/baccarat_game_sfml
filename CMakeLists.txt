
cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

# this tells cmake to put a file called compile_commands.json 
# that acts as a map for clangd lsp
# (for neovim users using clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# instead of downloading SFML, CMake reaches out to github and
# downloads the source code for SFML and perpares it to be built
# alongside your project
include(FetchContent)
FetchContent_Declare(SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 3.0.2
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
  SYSTEM)
FetchContent_MakeAvailable(SFML)
#################################################################
#adding IMGUI and IMGUI-SFML
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.5
  GIT_SHALLOW ON
  SOURCE_SUBDIR ""
)
# IMGUI dosn't use CMake natively, so we just manually download its files
FetchContent_MakeAvailable(imgui)

# tell the IMGUI-SFML bridge where we downloaded the core IMGUI files
set(IMGUI_DIR ${imgui_SOURCE_DIR})

# Tell ImGui-SFML to use our fetched SFML instead of searching the hard drive
set(IMGUI_SFML_FIND_SFML OFF)

# fetchC IMGUI_SFML (the bridge for SFML 3)
FetchContent_Declare(imgui_sfml
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
  GIT_TAG v3.0   # Version specifically updated for SFML 3
  GIT_SHALLOW ON
  SYSTEM
)

FetchContent_MakeAvailable(imgui_sfml)

# tell Cmake the name of your final program (run) and tells it
# which source files to compile to make it (src/main.cpp)
# when you add new .cpp files to your game you will list 
# them here -------------------V
add_executable(run 
  src/main.cpp
  src/core/SettingsManager.cpp
  src/Screens/Screen.h
  src/Screens/MainMenuScreen.h
  src/Screens/GameScreen.h

  #headers
  src/logic/card.h
  src/logic/deck.h
  src/logic/player.h
  src/logic/baccarat_hand.h
  src/logic/baccarat_table.h

  #source files
  src/logic/card.cpp
  src/logic/deck.cpp
  src/logic/player.cpp
  src/logic/baccarat_hand.cpp
  src/logic/baccarat_table.cpp

  # src/Card.cpp <- example
)
target_compile_features(run PRIVATE cxx_std_20)

# this connects the SFML libraries to your executable.
# if your games noods networking, you would simlpy add sfml-network to this
target_link_libraries(run PRIVATE 
  SFML::Graphics
  ImGui-SFML::ImGui-SFML
  uuid
)



# how to build and run your project
# CMake is a two-step process: configure (generating the bulid files for your specific system)
# and build (actually compiling the C++ code)

# STEP 1: CONFIGURE
# open your terminal or command prompt, navigate to the folder containing your CMakeLists.txt (this file)
# run these commands
# cmake -B build      // -B build flags tells CMake to put all the messy generated files into a folder named build
# 
# STEP 2: BULID
# cmake --build build     // looks inside the build folder and compiles your executable.
#                         // you will run this command every time you make a change to your C++ code
# 
# STEP 3: RUN
# once the build is complete you can run your app
# Windows: .\build\Debug\run.exe (or inside build\bin\)
# macOS / Linux: ./build/run (or inside ./build/bin/)


