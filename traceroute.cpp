
#include "traceroute.h"



#define DATAGRAM_SIZE 64
#define ICMP_HEADER_SIZE 8
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
void fill_in_IP_header(struct iphdr *ip_header, const char *destIP);
void fill_in_ICMP_header(struct icmphdr *icmp_header);
void buildDatagram (char *pkt, const char *destIP) {
  memset(pkt, 0, DATAGRAM_SIZE);
  struct iphdr *ip_header = (struct iphdr *)pkt;
  fill_in_IP_header(ip_header, destIP);

  struct icmphdr *icmp_header = (struct icmphdr *)(pkt + sizeof(struct iphdr));
  fill_in_ICMP_header(icmp_header);
  const char *pattern = "Call me Ishmael. Some years ago- never mind how long precisely- ";
  char *payload = pkt + sizeof(struct iphdr) + ICMP_HEADER_SIZE;
  size_t p_l = DATAGRAM_SIZE - sizeof(struct iphdr) - ICMP_HEADER_SIZE;
  strncpy(payload, pattern, p_l - 1);
  printf("Payload length: %zu\n", p_l);
  // int length = sizeof(ip-header) + sizeof(icmp-header) + sizeof (payload);
  // char *packet = new char[length]();
  // fill_in_IP_header(packet[0]);
  // fill_in_ICMP_header(packet + sizeof(ip-header));
  // memset(packet + sizeof(ip-header) + sizeof(icmp-header),’A’,sizeof(payload);
}
//kernel will fill in the ID, Source IP and Checksum fields
void fill_in_ICMP_header(struct icmphdr *icmp_header){
  icmp_header->checksum = 0;
  icmp_header->code = 0;
  icmp_header->type = ICMP_ECHO;
  icmp_header->un.echo.id = 0;
  icmp_header->un.echo.sequence =1;
}

void fill_in_IP_header(struct iphdr *ip_header, const char *destIP) {
  ip_header->daddr= inet_addr(destIP);
  ip_header->frag_off = 0;
  ip_header->id=htons(0);
  ip_header->ihl = 5;
  ip_header->protocol=IPPROTO_ICMP;
  ip_header->saddr = 0;
  ip_header->check = 0;
  ip_header->tos = 0;
  ip_header->tot_len = htons(DATAGRAM_SIZE);
  ip_header->ttl=64; ///////What should this be
  ip_header->version = 4;
  
  //struct iphdr *ip = (struct iphdr *)packet; // Cast the pointer
  //ip->version = 4;
  //ip->ihl = 5; // Header length / 4
  //ip->tos = 0;
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
  char *sendBuff = new char[DATAGRAM_SIZE];
  char *recBuff = new char[DATAGRAM_SIZE];
  // 2. Fill the whole buffer with a pattern of characters of your choice.
  // Convert from a dotted decimal string to network representation:
  buildDatagram(sendBuff, destIP.c_str());
  //DEBUG << "Built Datagram" << ENDL;
  //memset(sendBuff, 'T', DATAGRAM_SIZE);
  //"Call me Ishmael. Some years ago- never mind how long precisely- "
  // 3. Fill in all the fields of the IP header at the front of the buffer.
      //fill_in_ip_header()
    // a. You don’t need to fill in source IP or checksum
  // 4. Fill in all the fields of the ICMP header right behind the IP header.
      //fill_in_icmp_header()
  // 5. Create the send and receive sockets.
  int sendSockFD;
  int recSockFD;
  if ((sendSockFD = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) {
    perror("send socket");
    exit(-1);
  }
  if ((recSockFD = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
    perror("receive socket");
    exit(-1);
  }
  // Place a dotted decimal string into a address structure
  struct sockaddr_in dest_addr;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_addr.s_addr = inet_addr(destIP.c_str());
  bool lastHop = false;
  // 6. while (CURRENT_TTL <= 31) and (reply-not-received)
  // TTL LOOP START ##############################################
  for(int current_ttl = 2; current_ttl <=31 && !lastHop; current_ttl++){
    DEBUG << "In 31 for loop" << ENDL;
    // a. Set the TTL in the IP header in the buffer to CURRENT_TTL
    struct iphdr *ip_header = (struct iphdr *)sendBuff;
    struct icmphdr *icmp_header = (struct icmphdr *)(sendBuff+sizeof(struct iphdr));
    ip_header->ttl = current_ttl;
    // b. Set the checksum in the ICMP header
    //initialize?
    icmp_header->checksum = 0;
    icmp_header->checksum= checksum((unsigned short *)icmp_header, DATAGRAM_SIZE - sizeof(struct iphdr));
    // c. Send the buffer using sendfrom()
    //printf("sizeof(iphdr): %zu, sizeof(icmp): %zu\n",
      //  sizeof(struct iphdr), sizeof(struct icmp));
    DEBUG << "Calling sendto" << ENDL;
    if (sendto(sendSockFD, sendBuff, DATAGRAM_SIZE, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr)) == -1){
      perror("sendto");
      return -1;
      //cont?
    }
    printf("Sent Datagram TTL: %d\n", current_ttl);

    // d. While (now < START_TIME + 15) and (not-done-reading)
    struct timeval timeout;
    fd_set mySet;
    bool doneReading = false;
    bool gotResponse = false;
    // POLLING LOOP START #####################################
    for(int i = 0; i<3 && !doneReading; i++){
      DEBUG << "In wait loop run" << i << ENDL;
      FD_ZERO(&mySet);
      FD_SET(recSockFD, &mySet);
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;
      // i. Use select() to sleep for up to 5 seconds, wake up if data arrives.
      int poll = select(recSockFD+1, &mySet, NULL, NULL, &timeout);
      if(poll == 0){
        //if(i == 2){
        //  printf("%d *** no response from poll after 15 seconds ***\n", current_ttl );
        //}
        continue;
      }else if(poll < 0){
        perror("select");
        doneReading = true;
        break;
      }else{
        printf("Data Available!\n");
      }

      if(FD_ISSET(recSockFD, &mySet)){
        struct sockaddr_in rec_addr;
        socklen_t r_length = sizeof(rec_addr);
        ssize_t bytes_read = recvfrom(recSockFD, recBuff, DATAGRAM_SIZE, 0, (struct sockaddr *)&rec_addr, &r_length);
        if(bytes_read < 0){
            perror("recvfrom");
            continue;
        }
        char respAddress[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &rec_addr.sin_addr, respAddress, INET_ADDRSTRLEN);
          // 1. If received data is Echo Reply from the destination
        
        struct iphdr *recv_ip = (struct iphdr *)recBuff;
        int ip_hdr_len = recv_ip->ihl * 4;

        

        struct iphdr *rec_ip = (struct iphdr *)recBuff;
        int ih_len = rec_ip->ihl*4;
        struct icmphdr *rec_icmp = (struct icmphdr *)(recBuff+ih_len);
        
      
        printf("ICMP Type: %d, Code: %d\n", rec_icmp->type, rec_icmp->code);
        auto current = rec_addr.sin_addr.s_addr;
        auto desired = dest_addr.sin_addr.s_addr;
        bool correctDest = (current == desired);
        if(rec_icmp->type == ICMP_ECHOREPLY && correctDest){
          // a. Print message
          // b. Set reply-not-received to false
          // c. Set not-done-reading to false
          auto type = rec_icmp->type;
          DEBUG << "Type : " << type << ENDL;
          printf("%d %s completed\n", current_ttl, respAddress);
          gotResponse = true;
          doneReading = true;
          lastHop = true;
        }else if(rec_icmp->type == ICMP_TIME_EXCEEDED && rec_icmp->code == ICMP_EXC_TTL){
          // 2. If received data is TTL Time Exceeded; TTL
          // a. print message
          // b. Set not-done-reading to false
          DEBUG << "GOT icmp time exceeded" << ENDL;
          printf("%d %s TTL exceeded\n", current_ttl, respAddress);
          //responseGot = true;
          gotResponse = true;
          doneReading = true;
        }else{
          printf("TTL: %2d, Response Address: %s, Other ICMP type: %d code: %d \n", current_ttl, respAddress, rec_icmp->type, rec_icmp->code);
          //responseGot = true;
        }
        
        
        //auto current = rec_addr.sin_addr.s_addr;
        //auto desired = dest_addr.sin_addr.s_addr;
        //if(current == desired){
        //  printf("yay!\n");
        //  break;
        //}
      }
    }//POLLLING LOOP END ####################################
    //if(!responseGot){
    //  printf("%d *** no response from poll after 15 seconds ***\n", current_ttl );
    //}
    if(!gotResponse){
      printf("%d no response *********", current_ttl);
    }
      // ii. If data has arrived, read it with recevfrom()
    
        
    // e. Increment TTL by 1.
  }//TTL LOOP END ###############
 
 
  //Convert an address structure to a dotted decimal string.
  //inet_ntop(AF_INET, &recv_addr.sin_addr, respondent_ip, INET_ADDRSTRLEN);

  close(sendSockFD);
  close(recSockFD);
  delete[] sendBuff;
  delete[] recBuff;
  return 0;
    

}
