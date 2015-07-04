#include "decision.h"
#include "tango_data.h"


char tcpresult[3]="co";

void setTcpresult(char* value){
tcpresult[2]='\0';
memcpy(tcpresult,value,2);

}


int decide(){
//setTcpresult((char*) "co");
//__android_log_print(ANDROID_LOG_INFO, "xxxxxxxxx", "x %s", tcpresult);

if(strcmp(tcpresult,"co")==0)
    return 1;
    else if (strcmp(tcpresult,"er")==0)
        return 0;
            else return -1;

}