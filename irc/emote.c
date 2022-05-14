#include "emote.h"
#include "hashmap.h"
#include "irc_struct.h"
#include "util.h"
#include <string.h>

void get_global_emotes(Client *client, struct hashmap_s *emote_map) {
    char *endpoint = "/chat/emotes/global";
    size_t size = strlen(client->base_url) + strlen(endpoint);
    char url[URL_LEN];

    fmt_string(url, size + 1, "%s%s", client->base_url, endpoint);
    Response response = curl_request(client, url, curl_GET);
    response.data = json_object_object_get(response.response, "data");
    if (response.response != NULL) {
        response.data_len = json_object_array_length(response.data);
    }

    for (int i = 0; i < response.data_len; i++) {
        Emote *e = malloc(sizeof(Emote));
        struct json_object *images;
        response.data_array_obj = json_object_array_get_idx(response.data, i);
        e->name = get_key(response.data_array_obj, "name");
        e->id = get_key(response.data_array_obj, "id");
        images = json_object_object_get(response.data_array_obj, "images");
        e->url_1x = get_key(images, "url_1x");
        e->url_2x = get_key(images, "url_2x");
        e->url_3x = get_key(images, "url_3x");
        if (0 != hashmap_put(emote_map, e->name, strlen(e->name), e)) {
            printf("Could not add %s\n", e->name);
        }
        json_object_put(images);
    }
    clean_response(&response);
}

void get_channel_emotes(Client *client, const char *channel_id, struct hashmap_s *emote_map) {
    char url[URL_LEN];
    char *endpoint = "/chat/emotes?broadcaster_id=";
    size_t size = strlen(client->base_url) + strlen(endpoint) + strlen(channel_id);

    fmt_string(url, size + 1, "%s%s%s", client->base_url, endpoint, channel_id);
    Response response = curl_request(client, url, curl_GET);
    response.data = json_object_object_get(response.response, "data");
    if (response.response != NULL) {
        response.data_len = json_object_array_length(response.data);
    }

    for (int i = 0; i < response.data_len; i++) {
        Emote e;
        response.data_array_obj = json_object_array_get_idx(response.data, i);
        e.name = get_key(response.data_array_obj, "name");
        printf("%s\n", e.name);
        e.id = get_key(response.data_array_obj, "id");
        if (0 != hashmap_put(emote_map, e.name, strlen(e.name), &e)) {
            printf("Could not add %s\n", e.name);
        }
    }
    clean_response(&response);
}

bool init_emote_map(struct hashmap_s *emote_map, const unsigned initial_size) {
    if (0 != hashmap_create(initial_size, emote_map)) {
        return false;
    }
    return true;
}

Emote *get_emote(const char *word, struct hashmap_s *emote_map) {
    Emote *emote = hashmap_get(emote_map, word, strlen(word));
    return emote;
}

void parse_emotes(Message *message, struct hashmap_s *emote_map) {
    Emote *e;
    int i = 0;
    int j = 0;
    char word[MESSAGE_LEN];

    while (message->message[i] != '\0') {
        if (message->message[i] != ' ') {
            word[j] = message->message[i];
            j++;
        }
        if (message->message[i] == ' ' || message->message[i + 1] == '\0') {
            word[j] = '\0';
            e = get_emote(word, emote_map);
            if (e) {
                printf("%s\n", e->name);
                printf("%s\n", e->id);
                printf("%s\n", e->url_1x);
            }
            j = 0;
        }
        i++;
    }
}

void get_emote_maybe(const char *word, struct hashmap_s *emote_map) {
}
