// Este archivo define el tipo enumerado SatMode_t, que representa los diferentes modos del satélite.

#ifndef MODES_H
#define MODES_H

typedef enum {
    MODE_INICIALIZATION,
    MODE_DEPLOYMENT,
    MODE_NOMINAL,
    MODE_LOW_POWER,
    MODE_SAFE
} SatMode_t;

#endif // MODES_H