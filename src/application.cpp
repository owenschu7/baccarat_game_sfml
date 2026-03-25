#include "application.h"

#include <iostream>
#include <imgui.h>
#include <imgui-SFML.h>
#include "uuid.h"

#include "Screens/StartScreen.h"
#include "Screens/MainMenuScreen.h"
#include "Screens/GameScreen.h"
#include "Screens/SettingsScreen.h"
#include "Screens/demoScreen.h"
#include "Screens/availableTablesScreens.h"

#include "network/PacketParser.h"

#include "core/Debug.h"

Application::Application()
: m_settings("data/settings.txt")
{
  initWindow();
  initImGui();
  initData();
  loadFonts();

  //load the main menu
  //currentScreen will hold the current screen we are on (main menu, game screen, settings menu etc)
  m_currentScreen = std::make_unique<StartScreen>(m_sharedData);
}

Application::~Application()
{
  // Clean up resources when the window is finally closed
  ImGui::SFML::Shutdown();
}

void Application::run()
{
  while (m_window.isOpen())
  {
    processEvents();
    update();
    render();
  }
}

void Application::processEvents()
{
  // the event loop (input)
  // these actions are queued up as "events"
  //                                 m_window.pollEvent() - this grabs the oldest event waiting in the queue
  //                                                      it returns a std::optional, which basically means
  //                                                      "here is an event, or here is nothing(if the queue is empty)"
  //while () - because the user might press a key
  //           and move at the exact same time there
  //           might be multiple events waiting,
  //           this inner loop processes all pending
  //           events before moving on

  while (const std::optional event = m_window.pollEvent())
  {
    //pass the event to imgui so it can catch mouse clicks, typing, etc
    ImGui::SFML::ProcessEvent(m_window, *event);

    // (event->is<sf::Event::Closed>()) - this checks if the current event is the user trying to close the
    //                                    window. if it is, you call window.close() which breaks the main
    //                                    game loop on the next pass and cleanly ends the program
    if (event->is<sf::Event::Closed>())
    {
      m_window.close();
    }

    if (const auto* resized = event->getIf<sf::Event::Resized>())
    {
      //update the view to match the new window size exactaly
      sf::FloatRect visibleArea({0.0f, 0.0f},
                                {static_cast<float>(resized->size.x),
                                static_cast<float>(resized->size.y)});

      //tell the window to look through this camera
      m_window.setView(sf::View(visibleArea));
    }

    // pass the event down to whatever screen is currently active
    m_currentScreen->handleEvent(*event, m_window);
  }
}

void Application::update()
{
  //update imgui before your screens update
  ImGui::SFML::Update(m_window, m_deltaClock.restart());

  // process the mail carrier
  processNetwork();

  applySettingsChanges(); // this is where the changes (if any) will be applied to the game
  handleScreenTransitions(); // changes screens if needed

  //update and draw the active screen
  m_currentScreen->update();

  drawGlobalDebugUI(); // runs after the screen updates so it draws on top of everything!
}

void Application::render()
{
  //SFML uses a technique called Double Buffering.
  // Imagine an artist painting on a canvas hidden bedind a curtain, and when they are done,
  // they swap, it with the canvas the aduience is currently looking at

  // this wipes the hidden canvas clean by default it clears to black.
  // if you don't do this your new frame will just smear on top of the old frame
  m_window.clear();
  m_currentScreen->draw(m_window);

  ImGui::SFML::Render(m_window); // Render the UI on top of whatever the current screen just drew
  m_window.display(); // this drops the curtain and swaps the hidden canvas (which you just drew on) with the visible one on your monitor

}

//processNetwork() - 
void Application::processNetwork()
{
  // We drain the Outbox completely every frame. The UI screens drop 
  // GameEvents in here, and this function physically sends them over TCP.
  while (!m_sharedData.s_outboundEvents.empty())
  {
    DEBUG_PRINT << "application.cpp: processNetwork():\n";
    // Grab the oldest letter from the front of the queue
    GameEvent event = m_sharedData.s_outboundEvents.front();
    m_sharedData.s_outboundEvents.pop();

    // if the packet is a sys_connect we dont send a packet, instead we call connectToServer()
    if (event.type == EventType::SYS_Connect)
    {
      DEBUG_PRINT << "sys_connect detected\n";
      m_network.connectToServer("127.0.0.1", "8080");
    }


    //CLIENT -> SERVER
    // --- GAME EVENTS (Bets, Chat, Movement) ---
    // These need to be packed into binary and shipped across the internet.
    DEBUG_PRINT << "sending event: " << event <<"\n";

    // 1. Initialize the PacketBuilder to crush the struct into binary
    PacketBuilder packet;

    // 2. Pack the data in a strict, predictable order
    packet.append8(static_cast<uint8_t>(event.type)); // OpCode (1 byte)
    packet.appendString(event.senderUUID.c_str());    // UUID (Dynamic length + Null term)
    packet.appendString(event.senderUsername.c_str()); // Username + Null term
    packet.append32(event.intPayload);                // Int (4 bytes, Network Byte Order)
    packet.appendString(event.stringPayload.c_str()); // String (Dynamic length + Null term)

    // 3. Finalize: Calculates total size and prepends the 2-byte length header
    packet.finalize();

    const uint8_t* rawData = packet.getPtr();
    size_t totalSize = packet.getSize();

    DEBUG_PRINT << "[PACKET DUMP]\n"
                << "Size: " << totalSize << " bytes\n"
                << "Data: ";
    for (size_t i = 0; i < totalSize; i++)
    {
      // Print each byte as a 2-digit Hex number (e.g., 0A, FF, 62)
      DEBUG_PRINTF("%02X ", rawData[i]); 
    }
    DEBUG_PRINT << "\n";

    // 4. Hand the raw memory block to the NetworkClient to push to the OS buffer
    m_network.sendPacket(packet);
  }


  //SERVER -> CLIENT
  // inbound events from Server to Client
  std::vector<uint8_t> rawPacket;

  // pollPacket() safely locks the thread mutex, grabs a packet if one exists, 
  // puts it into 'rawPacket', and returns true. If empty, it returns false.
  while (m_network.pollPacket(rawPacket))
  {
    // 1. THE TRANSLATOR
    GameEvent newEvent = PacketParser::parse(rawPacket);

    // 2. ROUTE THE EVENT
    // We just drop EVERYTHING into the Inbox. The active UI screen
    // will read this queue during its update() loop and change states if necessary!
    m_sharedData.s_inboundEvents.push(newEvent);
  }
}

void Application::applySettingsChanges()
{
  if (m_sharedData.s_settingsChanged)
  {
    m_sharedData.s_lightMode ? ImGui::StyleColorsLight() : ImGui::StyleColorsDark();
    m_settings.Save(m_sharedData);

    //after we apply changes we set it to false to catch for other changes in
    // the future so that we dont constantly waste compute
    m_sharedData.s_settingsChanged = false;
  }
}



void Application::initWindow()
{
  //m_window is a sf::RenderWindow: this created the actual window on your operating system.
  //  you are passing it a VideoMode and the title that appears at the top 
  //  of the window. the "u" just tells C++ that these are "unsigned" integers
  m_window.create(sf::VideoMode({1920u,1080u}), "Baccarat Online");

  //without this your program will run as fast as your cpu possiblly can
  //this caps it at 60fps
  m_window.setFramerateLimit(60);

  //create a virtual camera locked exactaly to 1920x1080
  sf::View view(sf::FloatRect({0.f, 0.f}, {1920.f, 1080.f}));

  //tell the window to look through this camera
  m_window.setView(view);
}

void Application::initImGui()
{
  if (!ImGui::SFML::Init(m_window, false))
  {
    std::cerr << "Initialization failed! We can't safely use ImGui.\n";
    exit(-1);
  }
}

void Application::initData()
{
  m_settings.Load(m_sharedData); // loads data from settings.txt and stores it in m_sharedData

  // create a UUID if there is none
  if (m_sharedData.s_currentUUID.empty())
  {
    // Generate a new local UUID for the offline client
    m_sharedData.s_currentUUID = generateRealUUID();
    // Save the new UUID immediately so it persists next time
    m_settings.Save(m_sharedData);
    m_sharedData.s_AuthState = AuthState::NeedUsername;
  }
  else
{
    m_sharedData.s_AuthState = AuthState::LoggedIn;
  }

  //force an update to load session settings
  m_sharedData.s_settingsChanged = true;
}

void Application::loadFonts() 
{
  ImGuiIO& io = ImGui::GetIO();

  // Load the default font (Slot 0)
  ImFont* defaultFont = io.Fonts->AddFontFromFileTTF("assets/fonts/8bitOperatorPlus8-Regular.ttf", 20.0f);
  if (!defaultFont) { std::cerr << "Failed to load default font!\n"; exit(-1); }

  // Load the big font (Slot 1)
  ImFont* bigFont = io.Fonts->AddFontFromFileTTF("assets/fonts/8bitOperatorPlus8-Regular.ttf", 70.0f);
  if (!bigFont) { std::cerr << "Failed to load big font!\n"; exit(-1); }

  // Load the title font (Slot 2)
  ImFont* titleButtonFont = io.Fonts->AddFontFromFileTTF("assets/fonts/8bitOperatorPlus8-Regular.ttf", 40.0f);
  if (!titleButtonFont) { std::cerr << "Failed to load title button font!\n"; exit(-1); }

  // CRITICAL: You must tell ImGui-SFML to rebuild the font texture after adding a new font
  if (!ImGui::SFML::UpdateFontTexture()) 
  {
    std::cerr << "Failed to update ImGui font texture!\n";
  }
}

void Application::handleScreenTransitions() 
{
  //state switching logic
  // on every single frame, the manager asks the active screen
  // do you want to switch to a different screen?
  // if the user pressed somithing which triggers its internal state to change screens
  // then the manager will switch to the nextState
  ScreenState nextState = m_currentScreen->getNextState();
  if (nextState != ScreenState::None) 
  {
    switch (nextState) 
    {
      case ScreenState::Start:
        m_currentScreen = std::make_unique<StartScreen>(m_sharedData); break;
      case ScreenState::MainMenu:
        m_currentScreen = std::make_unique<MainMenuScreen>(m_sharedData); break;
      case ScreenState::Settings:
        m_currentScreen = std::make_unique<SettingsScreen>(m_sharedData); break;
      case ScreenState::Tables:
        m_currentScreen = std::make_unique<AvailableTablesScreen>(m_sharedData); break;
      case ScreenState::Game:
        m_currentScreen = std::make_unique<GameScreen>(m_sharedData); break;
      case ScreenState::Demo:
        m_currentScreen = std::make_unique<DemoScreen>(m_sharedData); break;
      case ScreenState::Quit:
        m_window.close(); break;
      default: break;
    }
  }
}

void Application::drawGlobalDebugUI()
{
  //displays on top left corner
  ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));

  // Make it an invisible floating window with no title bar or background
  ImGuiWindowFlags overlayFlags = ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoBackground | 
    ImGuiWindowFlags_AlwaysAutoResize | 
    ImGuiWindowFlags_NoMove | 
    ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("Global Debug", nullptr, overlayFlags);

  // Draw the checkbox and bind it directly to your SharedData bool
  ImGui::Checkbox("Show Demo", &m_sharedData.s_demoWindow);
  ImGui::End();

  // draw the actual demo window if the checkbox is ticked
  if (m_sharedData.s_demoWindow)
  {
    ImGui::ShowDemoWindow(&m_sharedData.s_demoWindow);
  }
}

std::string Application::generateRealUUID() 
{
  std::random_device rd;
  std::mt19937 gen(rd());
  uuids::uuid_random_generator uuid_gen{gen};
  uuids::uuid const id = uuid_gen();
  return uuids::to_string(id);
}

