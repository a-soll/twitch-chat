#include "parse.h"

void chat_init(TwitchChat *chat) {
    memset(&chat->hints, 0, sizeof(chat->hints));
    chat->hints.ai_family = AF_UNSPEC;
    chat->hints.ai_socktype = SOCK_STREAM;
    chat->hints.ai_flags = AI_PASSIVE;

    if ((chat->status = getaddrinfo("irc.chat.twitch.tv", MYPORT, &chat->hints, &chat->res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(chat->status));
        exit(1);
    }
    chat->sockfd = socket(chat->res->ai_family, chat->res->ai_socktype, chat->res->ai_protocol);
    chat->con = connect(chat->sockfd, chat->res->ai_addr, chat->res->ai_addrlen);
}

int chat_send(TwitchChat *chat, const char *msg) {
    int len = send(chat->sockfd, msg, strlen(msg), 0);
    return len;
}

int chat_recv(TwitchChat *chat, char buf[BUFF_SIZE]) {
    int rec = recv(chat->sockfd, buf, BUFF_SIZE, 0);
    if (rec != -1) {
        buf[rec] = '\0';
    }
    return rec;
}

void chat_deinit(TwitchChat *chat) {
    freeaddrinfo(chat->res);
}

void reset_message(Message *message) {
    message->message[0] = '\0';
    message->user[0] = '\0';
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
void parse_msg(char *field, char *msg_str, int cur_ind) {
    field[0] = '\0';
    bool add_to = false;
    int count = 0;
    while (msg_str[cur_ind] != '\0') {
        if (msg_str[cur_ind] == ':' && !add_to) {
            add_to = true;
            cur_ind++;
        }
        if (add_to) {
            field[count] = msg_str[cur_ind];
            count++;
        }
        cur_ind++;
    }
    field[count] = '\0';
}

void parse_message(Message *message, char *msg_str) {
    int i = 0;

    while (msg_str[i] != '\0') {
        i += parse_user(message->user, msg_str, i);
        parse_msg(message->message, msg_str, i);
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

int get_key(char *header_str, char *key, int cur_ind) {
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

void print_data(Header *header) {
    printf("BI %s\n", header->badge_info);
    printf("B %s\n", header->badges);
    printf("CN %s\n", header->client_nonce);
    printf("COL %s\n", header->color);
    printf("DN %s\n", header->display_name);
    printf("EM %s\n", header->emotes);
    printf("FM %d\n", header->first_msg);
    printf("FL %s\n", header->flags);
    printf("id %s\n", header->id);
    printf("MOD %d\n", header->mod);
    printf("RID %s\n", header->room_id);
    printf("SUB %d\n", header->subscriber);
    printf("TMI %s\n", header->tmi_sent_ts);
    printf("TURB %d\n", header->turbo);
    printf("UID %s\n", header->user_id);
    printf("user-type %s\n", header->user_type);
}

void parse_header(Header *header, char *header_str, int len) {
    int i = 1;
    char key[KEY_LEN];

    while (i <= len) {
        header_keys *hkey;
        i += get_key(header_str, key, i);
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

void init_irc(Irc *irc) {
    irc->processing_header = false;
    irc->processing_msg = false;
    irc->body[0] = '\0';
    irc->header_str[0] = '\0';
    irc->message.message[0] = '\0';
    irc->message.user[0] = '\0';
    _init_header(&irc->header);
}

void parse_irc(TwitchChat *chat, Irc *irc) {
    char to_post[HEADER_FRAG_LEN + BODY_LEN];
    char *tmp = NULL;
    int hind = 0;
    int bind = 0;
    int sz = chat_recv(chat, irc->buf);
    if (sz == -1) {
        return;
    }
    strcat(irc->to_process, irc->header_str);
    strcat(irc->to_process, irc->body);
    strcat(irc->to_process, irc->buf);

    for (int i = 0; i < sizeof(irc->to_process); i++) {
        if (irc->to_process[i] == '\0') {
            irc->to_process[0] = '\0';
            irc->header_str[hind] = '\0';
            irc->body[bind] = '\0';
            if (!irc->processing_header) {
                irc->header_str[0] = '\0';
            }
            return;
        }
        // check if \r\n and reset processing_header
        if (irc->to_process[i] == '\r' && (tmp = lookahead(irc->to_process, i, 2)) != NULL) {
            if (tmp[1] == '\n') {
                irc->processing_header = true;
                irc->body[bind] = '\0';
                if (irc->processing_msg) {
                    parse_message(&irc->message, irc->body);
                }
                irc->body[0] = '\0';
                bind = 0;
                hind = 0;
                irc->header_str[0] = '\0';
                free(tmp);
                i++;
                continue;
            }
        }
        if (irc->processing_header) {
            if (irc->to_process[i] == ' ') {
                irc->processing_header = false;
                irc->header_str[hind] = '\0';
                if (irc->header_str[0] == '@') {
                    irc->processing_msg = true;
                    parse_header(&irc->header, irc->header_str, hind);
                }
                continue;
            }
            irc->header_str[hind] = irc->to_process[i];
            hind++;
        } else {
            irc->body[bind] = irc->to_process[i];
            bind++;
        }
    }
}

void join_chat(TwitchChat *chat, const char *user, const char *token, const char *channel) {
    char msg[150];
    snprintf(msg, 150, "pass oauth:%s\r\n", token);
    printf("%s\n", msg);
}

void _init_header(Header *header) {
    header->badge_info[0] = '\0';
    header->badges[0] = '\0';
    header->client_nonce[0] = '\0';
    header->color[0] = '\0';
}
