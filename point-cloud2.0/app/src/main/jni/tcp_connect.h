#ifndef POINT_CLOUD_JNI_EXAMPLE_TANGO_TCP_CONNEXTION_H_
#define POINT_CLOUD_JNI_EXAMPLE_TANGO_TCP_CONNEXTION_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



  bool tcpconnection(const char *name , int port);

  void tcpsend(float * fbuffer, int lenght);

  void tcpdisconnect();

  void * tcptransaction(void *);

#endif
