#ifndef EMOTE_H
#define EMOTE_H

#include "client.h"
#include "irc_struct.h"
#include "hashmap.h"

typedef struct Emote {
    const char *name;
    const char *url_1x;
    const char *url_2x;
    const char *url_3x;
    const char *id;
    int start;
    int end;
} Emote;

void populate_emote(Emote *emote);
void get_channel_emotes(Client *client, const char *channel_id, struct hashmap_s *emote_map);
void get_global_emotes(Client *client, struct hashmap_s *emote_map);
bool init_emote_map(struct hashmap_s *emote_map, const unsigned initial_size);
Emote *get_emote(const char *word, struct hashmap_s *emote_map);
void parse_emotes(Message *message, struct hashmap_s *emote_map);

#endif /* EMOTE_H */
