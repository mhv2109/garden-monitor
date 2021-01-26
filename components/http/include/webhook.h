#ifndef WEBHOOK_H
#define WEBHOOK_H

#include "freertos/portmacro.h"

typedef struct wehbook {
  char *url;
  uint16_t wait;
  BaseType_t task_id;
} webhook;

typedef struct webhooks {
  webhook webhook;
  struct webhooks *next;
} webhooks;

void add_webhook(webhooks *ws, webhook *w);
void del_webhook(webhooks *ws, webhook *w);
webhook *get_webhook(webhooks *ws, BaseType_t task_id);
webhooks *load_webhooks(void);

#endif
