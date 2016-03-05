BlipBus
==
Arduino library for simple shared bus messaging, using JSON, UDP, and WiFi.

Features
==
* Send short messages between devices on the same WiFi network.
* Uses callback functions to handle events.
* Uses JSON for serialising, so it is easy to debug and build clients in other languages.
* Simple interface.

Dependencies
==
* ArduinoJson
* Arduino WiFi hardware (ESP8266 or a WiFi shield)

Example
==
```cpp
#include <blipbus.h>
BlipBus bb = BlipBus();

void handle_read()
{
    // Got a digitalio.read event!
    int pin = bb.get_int("pin");
    
    // Send a new event to all devices on this WiFi network.
    bb.create("test.example");
    bb.set("value", digitalRead(pin));
    bb.send();
}

void setup() {
    // XXX Your Arduino WiFi connection code here. (and any headers it needs above)

    // Register a handler for any events matching "digitalio.read"
    bb.on("digitalio.read", handle_read);
    
    // Use UDP port 3333 for sending events to the bus and listening for events.
    bb.begin(3333);
}

void loop() {
    // Check for new events and call any matching handers.
    bb.handle();
}
```

TODO
==
* ~~Publish in the platform.io library list~~
* Add installation instructions for the modern Arduino IDE with the library lookup feature.
* Add reply() for use instead of create() when you'd like to respond to only the sender of the last message.
* Make the size of the packet and JSON buffer configurable!
* Consider renaming create() to broadcast()?
* Consider authentication. (as this is intended for inconsequential devices on a trusted network, this is a low priority for now)
* Consider allowing deeper access to the underlying JSON object for complicated structures.
* Consider using fnmatch(3) for shell-like glob matching on event names.
* Tests would be nice. :)

Contributing
==
Contributions welcome!

License
==
See LICENSE file.
