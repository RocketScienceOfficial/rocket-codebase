#include "radio_controller.h"
#include "serial_controller.h"
#include "gps_controller.h"
#include "oled_controller.h"
#include "state_manager.h"
#include "power_manager.h"
#include "commander.h"
#include <Arduino.h>

static unsigned long g_power_last_update;
static unsigned long g_oled_last_update;

void setup()
{
  serial_init();
  oled_init();
  state_init();
  gps_init();
  radio_init();
  power_init();

  delay(1000);
}

void loop()
{
  serial_update();
  state_update();
  gps_update();
  radio_update();
  commander_update();

  unsigned long now = millis();

  if (now - g_power_last_update >= PMU_UPDATE_INTERVAL)
  {
    g_power_last_update = now;

    power_read();
  }

  if (now - g_oled_last_update >= OLED_UPDATE_INTERVAL || state_changed())
  {
    g_oled_last_update = now;

    oled_update();
  }
}