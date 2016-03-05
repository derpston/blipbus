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
        void begin(int);
        void create(const char *);
        void set(const char *, const char *);
        void set(const char *, int);
        void set(const char *, double);
        int get_int(const char *);
        double get_double(const char *);
        const char* get_str(const char *);
        int send();
        void dump();
        bool poll();
        void handle();
        void on(const char *, void (*)());
    private:
        WiFiUDP _sock;
        int _port;
        int _lastPacketSize = 0;
        char _sockbuf[BUFSIZE] = {0};
        StaticJsonBuffer<BUFSIZE> _json;
        JsonObject *_root;
        Handler *_handlers;
        void _ping();
};

