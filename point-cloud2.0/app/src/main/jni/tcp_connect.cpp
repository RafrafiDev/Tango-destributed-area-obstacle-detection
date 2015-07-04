

#include "tcp_connect.h"
#include "tango-gl/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include <netdb.h>
#include <conv.h>

#include "decision.h"

int sockfd, portno, n;
struct sockaddr_in serv_addr;
struct hostent *server;
bool isconnected=false;


//Tcp tcp;

//Tcp("141.26.71.166",60588);

bool tcpconnection (const char *name , int port) {

LOGI("soket start");

 portno = port;
 sockfd = socket(AF_INET, SOCK_STREAM, 0);
int reuse_opt = 1;

 if (sockfd < 0 && setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(int))!=-1){
    LOGE("soket opening socket Faild");
 }else{
 server = gethostbyname(name);
    LOGI("soket gethostbyname");
    if (server == NULL) {
    LOGE("soket no such host Faild");
    }else{
    bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *)server->h_addr,
                 (char *)&serv_addr.sin_addr.s_addr,
                 server->h_length);
        serv_addr.sin_port = htons(portno);

        LOGI("soket try connect");
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
            LOGE("Debuging connecting Faild");
        }else{

       // Decision::tcpresult=(char*)"ok";


          isconnected = true;
        __android_log_print(ANDROID_LOG_INFO, "soket connected", "%d", isconnected);
        return isconnected;

 /*char buffer[256];
    bzero(buffer,256);
    buffer[0]='(';buffer[1]='.';
    if (write(sockfd,buffer,2) < 0){
    LOGE("ERROR writing to soket");
    return;
    }


    bzero(buffer,256);

LOGI("receve data over soket");
    if (read(sockfd,buffer,18) < 0){
    LOGE("ERROR reading from soket");
    return;
   }
   __android_log_print(ANDROID_LOG_INFO, "soket connected", "%s", buffer);


*/
        }
    }
  }
return isconnected;
}

void tcpdisconnect(){
   LOGI("user disconnect socket");
   isconnected=false;
 if (write(sockfd,"#)#)#)",6) < 0){
    LOGE("ERROR writing to socket");
    return;
    }
close(sockfd);
}



void tcpsend(float * fbuffer, int length) {
//__android_log_print(ANDROID_LOG_INFO, "soket try", "%d %d", isconnected,length);

if(isconnected && length > 0){

//LOGI("send data over soket");
    schar buffert=floatToBytesArray(fbuffer,length);

    if (write(sockfd,buffert,length*sizeof(float)+6) < 0){
    LOGE("ERROR writing to soket");
    return;
    }



        pthread_t tcptransaction_thread;
        pthread_create(&tcptransaction_thread, NULL, tcptransaction,NULL);

    //tcprecv();
    }
}


void *tcptransaction(void *){

    char buffer[3];
    buffer[2]='\0';

  //  LOGI("receve data over soket");
        if (read(sockfd,buffer,2) < 0){
            LOGE("ERROR reading from soket");
            return NULL;
       }

   setTcpresult (buffer);
   // LOGE(buffer);
   //__android_log_print(ANDROID_LOG_INFO, "soket", "%s", Decision::tcpresult);

    return NULL;

}






