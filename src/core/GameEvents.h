#pragma once
#include <string>

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
