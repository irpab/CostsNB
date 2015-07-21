#ifndef COSTSNB_TCP_TRANSPORT_H
#define COSTSNB_TCP_TRANSPORT_H

#include <exception>
#include <string>
#include <tuple>
#include <memory>

class CostsNBTcpTransportException: public std::exception
{
  std::string _whatStr;
public:
  CostsNBTcpTransportException(std::string whatStr) : _whatStr(whatStr) {}
  virtual const char* what() const throw()
  {
    return _whatStr.c_str();
  }
};

class CostsNBTcpTransport {
  std::string _host;
  unsigned int _port;
  int _sock;

  void Tcp_connect();
public:
  CostsNBTcpTransport(std::string host, unsigned int port);
  ~CostsNBTcpTransport();

  void Send_tcp_packet(const char *buf, uint32_t len);
  std::tuple<std::unique_ptr<char[]>, uint32_t> Read_tcp_packet();
};

void Handshake(CostsNBTcpTransport &tcpTransport);
void Send_string(CostsNBTcpTransport &tcpTransport, const std::string str);

#endif
