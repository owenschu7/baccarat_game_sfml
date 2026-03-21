#include <cstdint>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdlib>
#include <unistd.h>


// stores a packet in a continuous memory
// - allows easy building of a packet by append8, append32, and appendString
// - stores it in stack memory for speed, and when space runs out on the
//   stack, it moves to the heap (slower but can hold bigger sized packets)
class PacketBuilder
{
private:
  static const int MAX_PACKET_SIZE = 1024;
  uint8_t m_buffer[MAX_PACKET_SIZE];

  uint8_t *m_active_buffer; // points to either the stack or heap
  size_t m_capacity;           // tracks the total space we have
  size_t m_curr_position = 2;      // keeps track of the size and where to append the next piece of data
                                   // index 0 and 1 are for the length of the packet

  void ensureCapacity(size_t extra_bytes)
  {
    //check to see if we have room
    if (m_curr_position + extra_bytes <= m_capacity)
    {
      return;
    }

    //calculate the new size (doubles it to fit the new data)
    size_t new_capacity = m_capacity * 2;
    while (m_curr_position + extra_bytes > new_capacity)
    {
      new_capacity *= 2;
    }

    //create the new bigger array (on the heap)
    uint8_t *new_heap_array = new uint8_t[new_capacity];
    
    //copy the existing data over
    memcpy(new_heap_array, m_active_buffer, m_curr_position);

    //if previous array was on the heap then we need to delete it
    // m_active_buffer and m_buffer are both same memory locations,
    // so when we check if they are not equal we know the original 
    // array has been modified (switched to heap)
    if (m_active_buffer != m_buffer)
    {
      delete[] m_active_buffer;
    }

    m_active_buffer = new_heap_array;
    m_capacity = new_capacity;
  }

public:
  PacketBuilder() {
    m_active_buffer = m_buffer; // point to the stack array
    m_capacity = MAX_PACKET_SIZE;
    m_curr_position = 2;
  }
  ~PacketBuilder() {
    //delete only if we have moved to the heap
    if (m_active_buffer != m_buffer)
    {
      delete[] m_active_buffer;
    }
  }

  // prevents copying so we dont double delete heap memory
  // tell the compiler to not generate automatic copy function
  PacketBuilder(const PacketBuilder&) = delete;
  PacketBuilder& operator=(const PacketBuilder&) = delete;

  //append to packet functions
  void append8(uint8_t value)
  {
    ensureCapacity(1);
    m_active_buffer[m_curr_position] = value;
    m_curr_position += 1;
  }

  void append32(uint32_t value)
  {
    ensureCapacity(4);
    uint32_t net_val = htonl(value);
    memcpy(&m_active_buffer[m_curr_position], &net_val, sizeof(net_val));
    m_curr_position += 4;
  }

  void appendString(const char *str) // handles null termination 
  {
    if (str == NULL)
    {
      return;
    }
    size_t length = strlen(str) + 1; // +1 for the null terminator
    ensureCapacity(length);

    memcpy(&m_active_buffer[m_curr_position], str, length);
    m_curr_position += length;
  }

  void finalize()
  {
    // Calculate how big the actual payload is (Total size minus the 2 header bytes)
    uint16_t payload_size = static_cast<uint16_t>(m_curr_position - 2);

    // Convert it to Network Byte Order (htons = Host TO Network Short)
    uint16_t net_size = htons(payload_size);

    // Copy that 2-byte integer into the very beginning of the buffer
    memcpy(m_active_buffer, &net_size, sizeof(uint16_t));
  }

  //get functions
  const uint8_t *getPtr() const { return m_active_buffer; }
  size_t getSize() const { return m_curr_position; }
};

