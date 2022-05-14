#include "parse.h"
#include <stddef.h>

#define TOTAL_KEYWORDS 25
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 25
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 61
/* maximum key range = 60, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
unsigned int hash(const char *str, unsigned int len) {
    static unsigned char asso_values[] =
        {
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62,  0, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 5,  0,  3,
            0,  20, 0,  62, 62, 0,  62, 62, 62, 0,
            0,  62, 62, 62, 0,  5,  30, 5,  62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
            62, 62, 62, 62, 62, 62};
    register unsigned int hval = len;

    switch (hval) {
    default:
        hval += asso_values[(unsigned char)str[5]];
    /*FALLTHROUGH*/
    case 5:
    case 4:
    case 3:
    case 2:
    case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
    return hval;
}

// header key string, type of parse function to call, and offset of the
// specific Header member to dynamically obtain
header_keys *in_word_set(const char *str, unsigned int len) {
    static header_keys wordlist[] = {
        {""}, {""},
        {"id", &parse_token, NULL, offsetof(Header, id)},
        {"mod", NULL, &parse_bool, offsetof(Header, mod)},
        {"bits", &parse_token, NULL, offsetof(Header, bits)},
        {"flags", &parse_token, NULL, offsetof(Header, flags)},
        {"msg-id", &parse_token, NULL, offsetof(Header, msg_id)},
        {"room-id", &parse_token, NULL, offsetof(Header, room_id)},
        {"color", &parse_token, NULL, offsetof(Header, color)},
        {"first-msg", NULL, &parse_bool, offsetof(Header, first_msg)},
        {"badge-info", &parse_token, NULL, offsetof(Header, badge_info)},
        {"badges", &parse_token, NULL, offsetof(Header, badges)},
        {"user-id", &parse_token, NULL, offsetof(Header, user_id)},
        {""},
        {"user-name", &parse_token, NULL, offsetof(Header, user_name)},
        {"subscriber", NULL, &parse_bool, offsetof(Header, subscriber)},
        {""},
        {"display-name", &parse_token, NULL, offsetof(Header, display_name)},
        {""},
        {"reply-parent-msg-id", &parse_token, NULL, offsetof(Header, reply_parent_msg_id)},
        {"reply-parent-user-id", &parse_token, NULL, offsetof(Header, reply_parent_user_id)},
        {"reply-parent-msg-body", &parse_token, NULL, offsetof(Header, reply_parent_msg_body)},
        {""},
        {"reply-parent-user-login", &parse_token, NULL, offsetof(Header, reply_parent_user_login)},
        {""},
        {"reply-parent-display-name", &parse_token, NULL, offsetof(Header, reply_parent_display_name)},
        {""}, {""}, {""}, {""},
        {"emote-only", NULL, &parse_bool, offsetof(Header, emote_only)},
        {"emotes", &parse_token, NULL, offsetof(Header, emotes)},
        {""}, {""}, {""},
        {"turbo", NULL, &parse_bool, offsetof(Header, turbo)},
        {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
        {"user-type", &parse_token, NULL, offsetof(Header, user_type)},
        {"client-nonce", &parse_token, NULL, offsetof(Header, client_nonce)},
        {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
        {""}, {""}, {""}, {""}, {""}, {""},
        {"tmi-sent-ts", &parse_token, NULL, offsetof(Header, tmi_sent_ts)}
    };

    if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
        unsigned int key = hash(str, len);

        if (key <= MAX_HASH_VALUE) {
            register const char *s = wordlist[key].name;

            if (*str == *s && !strcmp(str + 1, s + 1))
                return &wordlist[key];
        }
    }
    return 0;
}

int parse_header_line(Irc *irc, TwitchChat *chat, int i) {
    Iterator *iter = &irc->iterator;
    if (!iter->header_str[0]) {
        init_header(&irc->header);
    }
    int count = 0;

    while (irc->buf[i] != '\0') {
        if (irc->buf[i] == ' ') { // header ends on space
            iter->header_str[iter->hind] = '\0';
            pong_check(irc, chat, i);
            if (iter->header_str[0] == '@') {
                parse_header(&irc->header, iter->header_str, iter->hind);
                iter->processing_header = false;
                iter->processing_msg = true;
                count++;
            }
            iter->hind = 0;
            break;
        }
        if (irc->buf[i] != '\r' && irc->buf[i] != '\n') { // weed out welcome text
            iter->header_str[iter->hind] = irc->buf[i];
            iter->hind++;
        } else {
            iter->hind = 0;
            iter->header_str[0] = '\0';
        }
        count++;
        i++;
    }
    iter->header_str[iter->hind] = '\0';
    return count;
}

int parse_msg_line(Irc *irc, int i) {
    Iterator *iter = &irc->iterator;
    int count = 0;

    while (irc->buf[i] != '\0') {
        if (irc->buf[i] == '\r') { // \r is the official end of a message
            iter->processing_msg = false;
            iter->processing_header = true;
            iter->message_string[iter->bind] = '\0';
            parse_message(&irc->message, iter->message_string);
            iter->bind = 0;
            break;
        }
        iter->message_string[iter->bind] = irc->buf[i];
        iter->bind++;
        i++;
        count++;
    }
    iter->message_string[iter->bind] = '\0';
    return count;
}

char *lookahead(char *str, int position, int end) {
    char *to = malloc(sizeof(char) * end);
    int j = 0;

    for (int i = position; i < position + end; i++) {
        if (str[i] == '\0') {
            free(to);
            return NULL;
        } else {
            to[j] = str[i];
            j++;
        }
    }
    return to;
}

int parse_user(char field[], char *msg_str, int cur_ind) {
    int count = 0;
    int i = 0;
    while (msg_str[cur_ind] != '\0' && msg_str[cur_ind] != '!') {
        if (msg_str[cur_ind] == ':') {
            cur_ind++;
        } else {
            field[i] = msg_str[cur_ind];
            i++;
            cur_ind++;
        }
        count++;
    }
    field[i] = '\0';
    return count;
}

// get message text
void parse_msg(Message *message, char *msg_str, int cur_ind) {
    message->message[0] = '\0';
    bool add_to = false;
    int len = strlen(msg_str);

    while (msg_str[cur_ind] != '\0' && cur_ind < len) {
        if (msg_str[cur_ind] == ':' && !add_to) {
            add_to = true;
            cur_ind++;
        }
        if (add_to) {
            message->message[message->size] = msg_str[cur_ind];
            message->size++;
        }
        cur_ind++;
    }
    message->message[message->size] = '\0';
}

void parse_message(Message *message, char *msg_str) {
    int i = 0;

    while (msg_str[i] != '\0') {
        i += parse_user(message->user, msg_str, i);
        parse_msg(message, msg_str, i);
        break;
    }
}

int parse_token(char *field, char header_str[], int cur_ind) {
    field[0] = '\0';
    int count = 0;
    int i = 0;
    if (header_str[cur_ind] == '=') {
        count++;
        cur_ind++;
    }
    while (header_str[cur_ind] != ';' && header_str[cur_ind] != '\0') {
        field[i] = header_str[cur_ind];
        i++;
        count++;
        cur_ind++;
    }
    field[i] = '\0';
    return count;
}

int parse_bool(bool *field, char *header_str, int cur_ind) {
    int count = 0;
    if (header_str[cur_ind] == '=') {
        count++;
        cur_ind++;
    }
    while (header_str[cur_ind] != ';' && header_str[cur_ind] != '\0' && header_str[cur_ind] != ' ') {
        *field = header_str[cur_ind] - '0';
        cur_ind++;
        count++;
    }
    return count;
}

int get_header_key(char *header_str, char *key, int cur_ind) {
    int count = 0;
    int i = 0;
    while (header_str[cur_ind] != '=') {
        key[i] = header_str[cur_ind];
        i++;
        count++;
        cur_ind++;
    }
    key[i] = '\0';
    return count;
}

void parse_header(Header *header, char *header_str, int len) {
    int i = 1;
    char key[KEY_LEN];

    while (i <= len) {
        header_keys *hkey;
        i += get_header_key(header_str, key, i);
        hkey = in_word_set(key, strlen(key));
        if (hkey->parse_token != NULL) {
            char *field = (char *)((char *)header + hkey->offset);
            i += hkey->parse_token(field, header_str, i);
        }
        if (hkey->parse_bool != NULL) {
            bool field = *((bool *)(char *)header + hkey->offset);
            i += hkey->parse_bool(&field, header_str, i);
        }
        i++;
    }
}

void init_header(Header *header) {
    header->badge_info[0] = '\0';
    header->badges[0] = '\0';
    header->client_nonce[0] = '\0';
    header->color[0] = '\0';
    header->reply_parent_display_name[0] = '\0';
    header->reply_parent_msg_body[0] = '\0';
    header->reply_parent_user_id[0] = '\0';
    header->reply_parent_msg_id[0] = '\0';
    header->reply_parent_user_login[0] = '\0';
    header->room_id[0] = '\0';
}

void init_iterator(Iterator *iterator) {
    iterator->hind = 0;
    iterator->proc_ind = 0;
    iterator->bind = 0;
    iterator->to_process = NULL;
    iterator->processing_header = true;
    iterator->processing_msg = false;
    iterator->body[0] = '\0';
    iterator->header_str[0] = '\0';
    iterator->message_string[0] = '\0';
    iterator->process_len = 0;
    iterator->done_initial = false;
}
