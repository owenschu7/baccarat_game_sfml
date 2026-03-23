#pragma once
#include <string>
#include <iostream>

// Define all the possible actions a user can take, or the server can trigger
enum class EventType 
{
  //system events
  SYS_Connect = 0,
  SYS_Disconnect,
  SYS_Connect_Success,
  SYS_Connect_Failed,



  //login events
  //USER_In - sends a login request to the server to check if the player exists
  //          if yes the server responds true
  //          if no the server responds false
  USER_In,                 // user login 
  USER_Out,                // user logout
  USER_CreateAccount,      // user create account
  USER_ChangeUsername,     //change the users username
  
  NET_Connect_Success,
  NET_Connect_Failed,

  NET_NewAccount_Success,


  GET_AvailableTables,     // get a list of all available tables to play at
  CREATE_Table,            // user create a table
  JOIN_Table,              // user joins
  JOIN_NET_Table,          // someone has joined the table
  LEAVE_Table,             // user leaves
  LEAVE_NET_Table,         // someone has left the table

  CHAT_Send,
  CHAT_NET_Recv,
  

  //baccarat events
  // user actions
  BC_USER_Bet,             // user placed bet
  // actions from other people
  BC_NET_Bet,              // someone made a bet
  BC_NET_Ready,            // someone is ready
  BC_NET_AllReady,         // everyone at table is ready

  //blackjack events
  BJ_USER_PlaceBet,
  BJ_USER_PlayerHit,
  BJ_USER_PlayerStand,

};

// A generic event structure that holds a game event
// - EventType type
// - std::string senderUUID
// - std::string senderUsername
// - std::string stringPayload
// - int intPayload
struct GameEvent 
{
  EventType type;

  std::string senderUUID;
  std::string senderUsername;


  // Optional data attached to the event. 
  std::string stringPayload = ""; 
  int intPayload = 0;             
};



inline const char* EventTypeToString(EventType type) 
{
    switch (type) 
    {
        // System
        case EventType::SYS_Connect:            return "SYS_Connect";
        case EventType::SYS_Disconnect:         return "SYS_Disconnect";
        case EventType::SYS_Connect_Success:    return "SYS_Connect_Success";
        case EventType::SYS_Connect_Failed:     return "SYS_Connect_Failed";

        // Auth/Login
        case EventType::USER_In:                return "USER_In";
        case EventType::USER_Out:               return "USER_Out";
        case EventType::USER_CreateAccount:     return "USER_CreateAccount";
        case EventType::USER_ChangeUsername:    return "USER_ChangeUsername";
        case EventType::NET_Connect_Success:    return "NET_Connect_Success";
        case EventType::NET_Connect_Failed:     return "NET_Connect_Failed";
        case EventType::NET_NewAccount_Success: return "NET_NewAccount_Success";

        // Tables/Lobby
        case EventType::GET_AvailableTables:    return "GET_AvailableTables";
        case EventType::CREATE_Table:           return "CREATE_Table";
        case EventType::JOIN_Table:             return "JOIN_Table";
        case EventType::JOIN_NET_Table:         return "JOIN_NET_Table";
        case EventType::LEAVE_Table:            return "LEAVE_Table";
        case EventType::LEAVE_NET_Table:        return "LEAVE_NET_Table";

        // Chat
        case EventType::CHAT_Send:              return "CHAT_Send";
        case EventType::CHAT_NET_Recv:          return "CHAT_NET_Recv";

        // Baccarat
        case EventType::BC_USER_Bet:            return "BC_USER_Bet";
        case EventType::BC_NET_Bet:             return "BC_NET_Bet";
        case EventType::BC_NET_Ready:           return "BC_NET_Ready";
        case EventType::BC_NET_AllReady:        return "BC_NET_AllReady";

        // Blackjack
        case EventType::BJ_USER_PlaceBet:       return "BJ_USER_PlaceBet";
        case EventType::BJ_USER_PlayerHit:      return "BJ_USER_PlayerHit";
        case EventType::BJ_USER_PlayerStand:    return "BJ_USER_PlayerStand";

        // Catch-all
        default:                                return "UNKNOWN_EVENT";
    }
}

// 2. Tells C++ how to print a GameEvent struct to the console
inline std::ostream& operator<<(std::ostream& os, const GameEvent& event) 
{
    os << "{" 
       << EventTypeToString(event.type) 
       << " | User: " << (event.senderUsername.empty() ? "None" : event.senderUsername)
       << " | PayloadInt: " << event.intPayload 
       << " | PayloadStr: '" << event.stringPayload << "'"
       << "}";
    return os;
}
