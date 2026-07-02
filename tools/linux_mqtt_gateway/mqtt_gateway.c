#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>

#define DEFAULT_BROKER_HOST "127.0.0.1"
#define DEFAULT_BROKER_PORT 1883
#define DEFAULT_TOPIC   "aiot/glasses/cmd"

#define CMD_BUF_SIZE    32

static int extract_json_field(const char *json,
                               const char *key,
                            char *out,
                        size_t out_size)
{
    if(json==NULL||key==NULL||out==NULL||out_size==0)
    {
        return -1;
    }

    char pattern[64];
    snprintf(pattern,sizeof(pattern),"\"%s\":\"",key);

    const char *start=strstr(json,pattern);
    if(start==NULL)
    {
        return -1;
    }

    start+=strlen(pattern);

    const char *end=strchr(start,'"');
    if(end==NULL)
    {
        return -1;
    }

    size_t len=end-start;
    if(len>=out_size)
    {
        len=out_size-1;
    }

    memcpy(out,start,len);
    out[len]='\0';

    return 0;
}
static const char *cmd_to_stm32_protocol(const char *cmd)
{
    if(cmd==NULL)
    {
        return "HAND_NONE";
    }
    if(strcmp(cmd,"OPEN")==0)
    {
        return "HAND_OPEN";
    }
      if(strcmp(cmd,"GRAB")==0)
    {
        return "HAND_GRAB";
    }
      if(strcmp(cmd,"RELEASE")==0)
    {
        return "HAND_RELEASE";
    }
      if(strcmp(cmd,"STOP")==0)
    {
        return "HAND_STOP";
    }
    return "HAND_NONE";
}
static void handle_payload(const char*payload)
{
    char cmd[CMD_BUF_SIZE];

    printf("\nMQTT PAYLOAD=%s\n",payload);

    if(extract_json_field(payload,"cmd",cmd,sizeof(cmd))!=0)
    {
        printf("PARSE ERROR=cmd field not found\n");
        printf("STM32 CMD  =HAND_NONE\n");
        return;
    }
    printf("CMD =%s\n",cmd);
    printf("STM32   CMD  =%s\n",cmd_to_stm32_protocol(cmd));

}
static void on_connect(struct mosquitto*mosq,
                        void *userdata,
                        int rc)
{
    const char *topic=(const char*)userdata;

    if(rc==0)
    {
        printf("MQTT connected\n");

        int ret=mosquitto_subscribe(mosq,NULL,topic,1);
        if(ret!=MOSQ_ERR_SUCCESS)
        {
            printf("subscribe failed:%s\n",mosquitto_strerror(ret));
        }
        else
        {
            printf("Subscribed topic:%s\n",topic);
        }
    }
    else{
        printf("MQTT connect failed,rc=%d\n",rc);
    }
}
static void on_message(struct mosquitto*mosq,
                        void *userdata,
                        const struct mosquitto_message*msg)
{
    (void)mosq;
    (void)userdata;

    if(msg==NULL||msg->payload==NULL)
    {
        return;
    }

    char *payload=calloc((size_t)msg->payloadlen+1,1);
    if(payload==NULL)
    {
        printf("calloc payload failed\n");
        return;
    }
    memcpy(payload,msg->payload,(size_t)msg->payloadlen);
    payload[msg->payloadlen]='\0';

    printf("-----------------------------\n");
    printf("TOPIC   =%s\n",msg->topic);

    handle_payload(payload);

    free(payload);
}
int main(int argc,char * argv[])
{
    const char *host=DEFAULT_BROKER_HOST;
    int port=DEFAULT_BROKER_PORT;
    const char *topic=DEFAULT_TOPIC;

    if (argc>=2)
    {
        host=argv[1];
        /* code */
    }

    if(argc>=3)
    {
        port=atoi(argv[2]);
    }

    if(argc>=4)
    {
        topic=argv[3];
    }

    printf("Linux MQTT Gateway started\n");
    printf("Broker host:%s\n",host);
    printf("Broker port:%d\n",port);
    printf("Topic   :%s",topic);
    printf("-------------------------");

    mosquitto_lib_init();

    struct mosquitto *mosq=mosquitto_new("linux_gateway_01",true,(void*)topic);
    if (mosq==NULL)
    {
        printf("mosquitto_new failed\n");
        mosquitto_lib_cleanup();
        return 1;
        /* code */
    }
    
    mosquitto_connect_callback_set(mosq,on_connect);
    mosquitto_message_callback_set(mosq,on_message);

    int ret=mosquitto_connect(mosq,host,port,60);
    if(ret!=MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto_connect failed:%s\n",mosquitto_strerror(ret));
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return  1;
        /* data */
    }

    mosquitto_loop_forever(mosq,-1,1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    
    return 0;
}                       
    

