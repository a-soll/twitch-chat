#ifndef EMOTE_H
#define EMOTE_H

#include "client.h"
#include "hashmap.h"
#include <stdbool.h>

typedef struct Emote {
    const char *name;
    const char *url;
    const char *id;
    int start;
    int end;
} Emote;

void populate_emote(Emote *emote);
void get_channel_emotes(Client *client, const char *channel_id, struct hashmap_s *emote_map);
void get_global_emotes(Client *client, struct hashmap_s *emote_map);
bool init_emote_map(struct hashmap_s *emote_map, const unsigned initial_size);
void get_emote(const char *emote_id, struct hashmap_s *emote_map);

#endif /* EMOTE_H */
