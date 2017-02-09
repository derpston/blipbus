/*
Template implementations are in this file.
Reference: http://stackoverflow.com/questions/10632251/undefined-reference-to-template-function
*/

template<typename T>
BlipBusMessage &BlipBusMessage::set(const char *key, T value)
{
    (*_root)[key] = value;
    return *this;
}
