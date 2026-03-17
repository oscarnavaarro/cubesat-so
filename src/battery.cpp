#include "battery.h"

static int gBatteryLevel = 50;
static bool gBatteryDischarging = true;

int simulateBatteryLevel(void) {
    const int kStepPercent = 6;
    const int kMinPercent = 10;
    const int kMaxPercent = 100;

    if (gBatteryDischarging) {
        gBatteryLevel -= kStepPercent;
        if (gBatteryLevel <= kMinPercent) {
            gBatteryLevel = kMinPercent;
            gBatteryDischarging = false;
        }
    } else {
        gBatteryLevel += kStepPercent;
        if (gBatteryLevel >= kMaxPercent) {
            gBatteryLevel = kMaxPercent;
            gBatteryDischarging = true;
        }
    }

    return gBatteryLevel;
}