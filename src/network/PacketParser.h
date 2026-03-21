#pragma once

#include <cstdint>
#include <cstring>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <iostream> //delete this later

#include "../core/GameEvents.h" // Make sure to include this so it knows what a GameEvent is!

class PacketParser
{
private:
  const uint8_t* m_buffer;
  size_t m_size;
  size_t m_curr_position;

public:
  // Initialize with the data you received from the network
  PacketParser(const uint8_t* buffer, size_t size) 
  : m_buffer(buffer), m_size(size), m_curr_position(0) {}

  PacketParser(const PacketParser&) = delete;
  PacketParser& operator=(const PacketParser&) = delete;

  // ==========================================
  // THE MAGIC PARSER FUNCTION
  // ==========================================
  static GameEvent parse(const std::vector<uint8_t>& rawPacket)
  {
    GameEvent event;

    // Safety check: if the packet is empty, return an empty event
    if (rawPacket.empty()) return event;

    // Create an instance of the parser using the raw vector
    PacketParser parser(rawPacket.data(), rawPacket.size());

    // 2. READ THE DATA (In the EXACT order the PacketBuilder appended it)
    event.type           = static_cast<EventType>(parser.read8());
    event.senderUUID     = parser.readString();
    event.senderUsername = parser.readString(); // <-- Updated name
    event.intPayload     = parser.read32();
    event.stringPayload  = parser.readString();

    return event;
  }


  // --- Read Functions ---

  uint8_t read8()
  {
    if (m_curr_position + 1 > m_size) return 0;

    uint8_t value = m_buffer[m_curr_position];
    m_curr_position += 1;
    return value;
  }

  // Function to read the 2-byte length header
  uint16_t read16()
  {
    if (m_curr_position + 2 > m_size) return 0;

    uint16_t net_val;
    memcpy(&net_val, &m_buffer[m_curr_position], sizeof(net_val));
    m_curr_position += 2;

    return ntohs(net_val); // ntohs = Network TO Host Short
  }

  uint32_t read32()
  {
    if (m_curr_position + 4 > m_size) return 0;

    uint32_t net_val;
    memcpy(&net_val, &m_buffer[m_curr_position], sizeof(net_val));
    m_curr_position += 4;

    return ntohl(net_val); 
  }

  std::string readString()
  {
    std::string result = "";
    while (m_curr_position < m_size && m_buffer[m_curr_position] != '\0')
    {
      result += static_cast<char>(m_buffer[m_curr_position]);
      m_curr_position++;
    }

    if (m_curr_position < m_size && m_buffer[m_curr_position] == '\0') {
      m_curr_position++;
    }

    return result;
  }

  bool isFinished() const { return m_curr_position >= m_size; }
  size_t getBytesRead() const { return m_curr_position; }
};
