#include <Arduino.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define BUFSIZE 256

class BlipBusMessage
{
    public:
        BlipBusMessage();
        BlipBusMessage(const char *);
        template<typename T> BlipBusMessage &set(const char *, T);
        int get_int(const char *);
        double get_double(const char *);
        const char* get_str(const char *);
        void dump();
        JsonObject * get_root();
        void parse(char *);
        bool is_valid();
    private:
        JsonObject *_root;
        StaticJsonBuffer<BUFSIZE> _json;
};

struct Handler {
    char *spec;
    void (*func_ptr)(BlipBusMessage);
    Handler *next = NULL;
};


class BlipBus
{
    public:
        BlipBus();
        void begin(const char *, int);
        bool poll();
        void handle();
        void on(const char *, void (*)(BlipBusMessage));
        int send(BlipBusMessage);
        int send(BlipBusMessage, const char *, int);
        int send(BlipBusMessage, IPAddress, int);
        BlipBusMessage fetch();
    private:
        WiFiUDP _sock;
        const char * _devicename;
        int _port;
        int _lastPacketSize = 0;
        char _sockbuf[BUFSIZE] = {0};
        Handler *_handlers;
        void _ping(BlipBusMessage);
};

#include "blipbus.tpp"
