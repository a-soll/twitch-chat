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
    unsigned int hash(const char *str, unsigned int len)
{
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
            62, 62, 62, 62, 62, 62, 62,  5,  0,  3,
            0, 20,  0, 62, 62,  0, 62, 62, 62,  0,
            0, 62, 62, 62,  0,  5, 30,  5, 62, 62,
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
            62, 62, 62, 62, 62, 62
        };
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
