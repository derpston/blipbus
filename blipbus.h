#include <Arduino.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define BUFSIZE 256

struct Handler {
    char *spec;
    void (*func_ptr)();
    Handler *next = NULL;
};

class BlipBus
{
    public:
        BlipBus();
        void begin(const char *, int);
        BlipBus &create(const char *);
        template<typename T> BlipBus &set(const char *, T);
        int get_int(const char *);
        double get_double(const char *);
        const char* get_str(const char *);
        int send();
        int send(const char *, int);
        int send(IPAddress, int);
        void dump();
        bool poll();
        void handle();
        void on(const char *, void (*)());
    private:
        WiFiUDP _sock;
        const char * _devicename;
        int _port;
        int _lastPacketSize = 0;
        char _sockbuf[BUFSIZE] = {0};
        StaticJsonBuffer<BUFSIZE> _json;
        JsonObject *_root;
        Handler *_handlers;
        void _ping();
};

#include "blipbus.tpp"
