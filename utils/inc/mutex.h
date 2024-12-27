#pragma once

#include "common.h"

typedef struct {
  volatile uint32_t notified;
} Notif;

void notif_init(Notif *notif);
void notif_wait(Notif *notif);
void notif_signal(Notif *notif);
