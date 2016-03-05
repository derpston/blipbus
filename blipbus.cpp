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

void BlipBus::begin(int port)
{
    _port = port;
    _sock.begin(port);
}

void BlipBus::create(const char *event_name)
{
    clear(_json);
    _root = &_json.createObject();
    (*_root)["name"] = String(event_name);
}

void BlipBus::set(const char *key, const char *value)
{
    (*_root)[key] = value;
}

void BlipBus::set(const char *key, int value)
{
    (*_root)[key] = value;
}

void BlipBus::set(const char *key, double value)
{
    (*_root)[key] = value;
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
    _sock.beginPacket("255.255.255.255", _port);
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
        const char *name = (*_root).get<const char*>("name");

        // Does this message have a name key?
        if(name == NULL)
            return;

        // Have any handlers been registered?
        if(_handlers == NULL)
            return;

        // Look for a handler with a matching name and call it.
        Handler *cur = _handlers;
        do
        {
            if(strcmp(cur->spec, name) == 0)
            {
                cur->func_ptr();
                return;
            }
        }
        while((cur = cur->next) != NULL);

        // No matching handlers, check special internal handlers.
        if(strcmp("blipbus.ping", name) == 0)
           return  _ping();

        // No further handlers, ignore the message.
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
    // Repeat the most recent message back to the sender, but change
    // the name so it is clear it is a response to a previous ping.
    
    // Many devices may have received a broadcast ping, so to prevent
    // them all replying at once, delay for between 0-50ms.
    delay(random(0, 50));
    _sock.beginPacket(_sock.remoteIP(), _sock.remotePort());
    (*_root)["name"] = "blipbus.pong";
    (*_root).printTo(_sock);
    _sock.endPacket();
}


