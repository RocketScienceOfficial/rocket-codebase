#include "OLEDModule.h"

void OLEDModule::initDisplay()
{
    u8g2_SetupBuffer_SDL_128x64(&m_Display, U8G2_R0);
}

bool OLEDModule::shouldChangeState()
{
    return u8g_sdl_get_key() == ' ';
}