#include <blipbus.h>

// https://github.com/bblanchon/ArduinoJson/wiki/Bag-of-Tricks#reuse-jsonbuffer
template<typename T>
void clear(T& instance)
{
    instance = T();
}

BlipBus::BlipBus()
{
}

void BlipBus::begin(const char * devicename, int port)
{
    _devicename = devicename;
    _port = port;
    _sock.begin(port);
}

void BlipBus::create(const char *event_name)
{
    clear(_json);
    _root = &_json.createObject();
    (*_root)["event"] = String(event_name);
    (*_root)["src"]= String(_devicename);
}

int BlipBus::get_int(const char *key)
{
    return (*_root)[key];
}

double BlipBus::get_double(const char *key)
{
    return (*_root)[key];
}

const char* BlipBus::get_str(const char *key)
{
    return (*_root).get<const char*>(key);
}

int BlipBus::send()
{
    return BlipBus::send("255.255.255.255", _port);
}

int BlipBus::send(const char * host, int port)
{
    _sock.beginPacket(host, port);
    (*_root).printTo(_sock);
    return _sock.endPacket();
}

int BlipBus::send(IPAddress ip, int port)
{
    _sock.beginPacket(ip, port);
    (*_root).printTo(_sock);
    return _sock.endPacket();
}

void BlipBus::dump()
{
    (*_root).prettyPrintTo(Serial);
}

bool BlipBus::poll()
{
    _lastPacketSize = _sock.parsePacket();
    if(_lastPacketSize)
    {   
        memset(_sockbuf, 0, sizeof(_sockbuf));
        _sock.read(_sockbuf, sizeof(_sockbuf) - 1);

        clear(_json);
        _root = &_json.parseObject(_sockbuf);
        return (*_root).success();
    }

    return false;
}

void BlipBus::handle()
{
    // Did we get a message, with valid JSON?
    while(BlipBus::poll())
    {
        const char *event = (*_root).get<const char*>("event");

        // Does this message have an event key?
        if(event == NULL)
            return;

        // Look for a handler with a matching event and call it.
        Handler *cur = _handlers;
        //while((cur != NULL) && ((cur = cur->next) != NULL))
        while(cur != NULL)
        {
            if(strcmp(cur->spec, event) == 0)
            {
                cur->func_ptr();
                return;
            }

            cur = cur->next;
        }

        // No matching handlers, check special internal handlers.
        if(strcmp("blipbus.ping", event) == 0)
            return  _ping();
    }

}

void BlipBus::on(const char *spec, void (*func_ptr)())
{
    Handler *cur = _handlers;
    if(cur == NULL)
    {
        // Creating the root node.
        cur = _handlers = new Handler;
    }
    else
    {
        // The root node has been created already, so iterate through
        // the list until we reach the end.
        while(cur->next != NULL)
        {
            Serial.println(cur->spec);
            cur = cur->next;
        }

        // Create a new node and switch to it.
        cur->next = new Handler;
        cur = cur->next;
    }

    // Allocate some space for the spec string, copy it in, and set the
    // function pointer for the callback.
    cur->spec = (char*) malloc(strlen(spec) + 1);
    strcpy(cur->spec, spec);
    cur->func_ptr = func_ptr;
}

void BlipBus::_ping()
{
    // Many devices may have received a broadcast ping, so to prevent
    // them all replying at once, delay for between 0-50ms.
    delay(random(0, 50));

    create("blipbus.pong");
    set("uptime", millis() / 1000);

    // Send the response back to the source.
    send(_sock.remoteIP(), _sock.remotePort());
}


