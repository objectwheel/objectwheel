#include <settings.h>

Settings::Settings()
{

}

Settings* Settings::instance()
{
    static Settings instance;
    return &instance;
}
