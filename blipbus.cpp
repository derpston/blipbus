#include <blipbus.h>

BlipBus::BlipBus()
{
}

BlipBusMessage::BlipBusMessage()
{
    // _root is null... other methods should be protected from this.
    _root = NULL;
}

BlipBusMessage::BlipBusMessage(const char *event_name)
{
    _root = &_json.createObject();
    (*_root)["event"] = String(event_name);
}


void BlipBus::begin(const char * devicename, int port)
{
    _devicename = devicename;
    _port = port;
    _sock.begin(port);
}

int BlipBusMessage::get_int(const char *key)
{
    return (*_root)[key];
}

double BlipBusMessage::get_double(const char *key)
{
    return (*_root)[key];
}

const char* BlipBusMessage::get_str(const char *key)
{
    return (*_root).get<const char*>(key);
}

int BlipBus::send(BlipBusMessage bbm)
{
    return BlipBus::send(bbm, "255.255.255.255", _port);
}

int BlipBus::send(BlipBusMessage bbm, const char * host, int port)
{
    bbm.set("src", _devicename);
    _sock.beginPacket(host, port);
    (*bbm.get_root()).printTo(_sock);
    return _sock.endPacket();
}

int BlipBus::send(BlipBusMessage bbm, IPAddress ip, int port)
{
    bbm.set("src", _devicename);
    _sock.beginPacket(ip, port);
    (*bbm.get_root()).printTo(_sock);
    return _sock.endPacket();
}

void BlipBusMessage::dump()
{
    (*_root).printTo(Serial);
}

JsonObject* BlipBusMessage::get_root()
{
    return _root;
}

bool BlipBus::poll()
{
    _lastPacketSize = _sock.parsePacket();
    if(_lastPacketSize)
    {   
        memset(_sockbuf, 0, sizeof(_sockbuf));
        _sock.read(_sockbuf, sizeof(_sockbuf) - 1);

        return true;
    }

    return false;
}

BlipBusMessage BlipBus::fetch()
{
    BlipBusMessage bbm = BlipBusMessage();
    bbm.parse(_sockbuf);
    return bbm;
}

void BlipBusMessage::parse(char * serialised_json)
{
    _root = &_json.parseObject(serialised_json);
}

bool BlipBusMessage::is_valid()
{
    return (*_root).success();
}

void BlipBus::handle()
{
    // Did we get a message, with valid JSON?
    while(BlipBus::poll())
    {
        BlipBusMessage bbm = BlipBus::fetch();

        if(!bbm.is_valid())
            continue;

        const char *event = bbm.get_str("event");

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
                cur->func_ptr(bbm);
                return;
            }

            cur = cur->next;
        }

        // No matching handlers, check special internal handlers.
        if(strcmp("blipbus.ping", event) == 0)
            return  _ping(bbm);
    }
}

void BlipBus::on(const char *spec, void (*func_ptr)(BlipBusMessage))
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

void BlipBus::_ping(BlipBusMessage bbm)
{
    // Many devices may have received a broadcast ping, so to prevent
    // them all replying at once, delay for between 0-50ms.
    delay(random(0, 50));

    bbm.set("event", "blipbus.pong");
    bbm.set("uptime", millis() / 1000);

    // Send the response back to the source.
    send(bbm, _sock.remoteIP(), _sock.remotePort());
}
