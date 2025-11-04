
#include "traceroute.h"



#define DATAGRAM_SIZE 64
// ****************************************************************************
// * Compute the Internet Checksum over an arbitrary buffer.
// * (written with the help of ChatGPT 3.5)
// ****************************************************************************
uint16_t checksum(unsigned short *buffer, int size) {
    unsigned long sum = 0;
    while (size > 1) {
        sum += *buffer++;
        size -= 2;
    }
    if (size == 1) {
        sum += *(unsigned char *) buffer;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short) (~sum);
}

void buildDatagram () {
  int length = sizeof(ip-header) + sizeof(icmp-header) + sizeof (payload);
  char *packet = new char[length]();
  fill_in_IP_header(packet[0]);
  fill_in_ICMP_header(packet + sizeof(ip-header));
  memset(packet + sizeof(ip-header) + sizeof(icmp-header),’A’,sizeof(payload);
}
//kernel will fill in the ID, Source IP and Checksum fields
void fill_in_ICMP_header(char *packet){

}

void fill_in_IP_header(char *packet) {
  struct iphdr *ip = (struct iphdr *)packet; // Cast the pointer
  ip->version = 4;
  ip->ihl = 5; // Header length / 4
  ip->tos = 0;
  // Fill in the rest of the header fields.
}


int main (int argc, char *argv[]) {
  std::string destIP;

  // ********************************************************************
  // * Process the command line arguments
  // ********************************************************************
  int opt = 0;
  while ((opt = getopt(argc,argv,"t:d:")) != -1) {

    switch (opt) {
    case 't':
      destIP = optarg;
      break;
    case 'd':
      LOG_LEVEL = atoi(optarg);;
      break;
    case ':':
    case '?':
    default:
      std::cout << "useage: " << argv[0] << " -t [target ip] -d [Debug Level]" << std::endl;
      exit(-1);
    }
  }

//

  //1. Allocate two 64 byte buffers. One for sending and one for receiving.
  //char sendBuff[] = ;
  //char recBuff[] = ;
  // 2. Fill the whole buffer with a pattern of characters of your choice.
  //"Call me Ishmael. Some years ago- never mind how long precisely- "
  // 3. Fill in all the fields of the IP header at the front of the buffer.
    // a. You don’t need to fill in source IP or checksum
  // 4. Fill in all the fields of the ICMP header right behind the IP header.
  // 5. Create the send and receive sockets.
  // 6. while (CURRENT_TTL <= 31) and (reply-not-received)
    // a. Set the TTL in the IP header in the buffer to CURRENT_TTL
    // b. Set the checksum in the ICMP header
    // c. Send the buffer using sendfrom()
    // d. While (now < START_TIME + 15) and (not-done-reading)
      // i. Use select() to sleep for up to 5 seconds, wake up if data arrives.
      // ii. If data has arrived, read it with recevfrom()
        // 1. If received data is Echo Reply from the destination
          // a. Print message
          // b. Set reply-not-received to false
          // c. Set not-done-reading to false
        // 2. If received data is TTL Time Exceeded; TTL
          // a. print message
          // b. Set not-done-reading to false
    // e. Increment TTL by 1.
    

}
