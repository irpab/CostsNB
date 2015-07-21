
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include <iostream>
#include <sstream>

#include "costsnb_tcp_transport.h"
#include "costsnb_clisrv_structs.h"
#include "costsnb_clisrv_encoder.h"

extern App_message *Parse_message(char *buf);

CostsNBTcpTransport::CostsNBTcpTransport(std::string host, unsigned int port)
    : _host(host), _port(port) {
  Tcp_connect();
}
CostsNBTcpTransport::~CostsNBTcpTransport() {
  close(_sock);
}

void CostsNBTcpTransport::Tcp_connect() {
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  std::stringstream ss;
  ss << _port;
  int getaddr_res = getaddrinfo(_host.c_str(), ss.str().c_str(), &hints, &result);
  if (getaddr_res != 0) {
    throw CostsNBTcpTransportException("getaddrinfo failed");
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    _sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (_sock == -1)
      continue;

    if (connect(_sock, rp->ai_addr, rp->ai_addrlen) != -1)
      break;

    close(_sock);
  }

  if (result != NULL) {
    freeaddrinfo(result);
  }

  if (rp == NULL) {
    close(_sock);
    throw CostsNBTcpTransportException("all socket/connect failed");
  }
}

void CostsNBTcpTransport::Send_tcp_packet(const char *buf, uint32_t len) {
  int numSent;
  uint32_t len2 = htonl(len);
  numSent = send(_sock, (char*)(&len2), 4, 0);
  if (numSent <= 0)
    throw CostsNBTcpTransportException("send packet length failed");
  numSent = send(_sock, buf, len, 0);
  if (numSent <= 0)
    throw CostsNBTcpTransportException("send packet failed");
}

std::tuple<std::unique_ptr<char[]>, uint32_t> CostsNBTcpTransport::Read_tcp_packet() {
  uint32_t packet_size = 0;
  int n = read(_sock, &packet_size, sizeof(packet_size));
  if (n <= 0)
    throw CostsNBTcpTransportException("read packet length failed");

  packet_size = ntohl(packet_size);
  std::unique_ptr<char[]> buf(new char[packet_size]);
  // while (packet_size != 0) {
    n = read(_sock, buf.get(), packet_size); // smart pointer!
    if (n <= 0)
      throw CostsNBTcpTransportException("read packet failed");
    // packet_size -= n;
  // }
  return std::make_tuple(std::move(buf), packet_size);
}

void Handshake(CostsNBTcpTransport &tcpTransport) {
  std::unique_ptr<char[]> buf;
  uint32_t packet_size;
  std::tie(buf, packet_size) = tcpTransport.Read_tcp_packet();
  std::shared_ptr<App_message> appMsg(Parse_message(buf.get()));
  if (!appMsg)
    throw CostsNBTcpTransportException("got unknown msg");

  std::shared_ptr<Handshake_srv> handshake_srv = std::static_pointer_cast<Handshake_srv>(appMsg);
  if (!handshake_srv)
    throw CostsNBTcpTransportException("got not Handshake_srv msg");
  std::cout << "Server vsn = " << handshake_srv->vsn << std::endl;

  Handshake_cli_ack msg;
  buf.reset(new char[sizeof(Handshake_cli_ack)]);
  Encode_message(buf.get(), &msg);
  tcpTransport.Send_tcp_packet(buf.get(), sizeof(Handshake_cli_ack));
}

void Send_string(CostsNBTcpTransport &tcpTransport, const std::string str) {
  const char *buf = str.c_str();
  size_t blockSize = 1024;
  size_t strSize = str.size();
  size_t numSeqs = strSize / blockSize;
  size_t lastBlockSize = strSize % blockSize;
  if (lastBlockSize > 0)
    numSeqs += 1;
  Str_data str_data;
  std::unique_ptr<char[]> buf2(new char[sizeof(Str_data)]);
  for (auto seq = 0; seq < numSeqs; seq++) {
    auto curBlockShift = blockSize*seq;
    auto curBlockSize = (seq != (numSeqs-1)) ? blockSize : lastBlockSize;
    str_data.seq_num = seq;
    str_data.data_size = curBlockSize;
    str_data.last = (seq != (numSeqs-1)) ? 0 : 1;
    std::memcpy(str_data.data, buf+curBlockShift, curBlockSize);
    Encode_message(buf2.get(), &str_data);
    tcpTransport.Send_tcp_packet(buf2.get(), sizeof(Str_data));
  }
}
