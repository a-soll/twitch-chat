#include "irc_struct.h"

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
        if (msg_str[cur_ind] == ':') {
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
    printf("FM %d\n", header->first_message);
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
        i += get_key(header_str, key, i);
        if (strcmp(key, "badge-info") == 0) {
            i += parse_token(header->badge_info, header_str, i);
        } else if (strcmp(key, "badges") == 0) {
            i += parse_token(header->badges, header_str, i);
        } else if (strcmp(key, "client-nonce") == 0) {
            i += parse_token(header->client_nonce, header_str, i);
        } else if (strcmp(key, "color") == 0) {
            i += parse_token(header->color, header_str, i);
        } else if (strcmp(key, "display-name") == 0) {
            i += parse_token(header->display_name, header_str, i);
        } else if (strcmp(key, "emote-only") == 0) {
            i += parse_bool(&header->emote_only, header_str, i);
        } else if (strcmp(key, "emotes") == 0) {
            i += parse_token(header->emotes, header_str, i);
        } else if (strcmp(key, "first-msg") == 0) {
            i += parse_bool(&header->first_message, header_str, i);
        } else if (strcmp(key, "flags") == 0) {
            i += parse_token(header->flags, header_str, i);
        } else if (strcmp(key, "id") == 0) {
            i += parse_token(header->id, header_str, i);
        } else if (strcmp(key, "mod") == 0) {
            i += parse_bool(&header->mod, header_str, i);
        } else if (strcmp(key, "room-id") == 0) {
            i += parse_token(header->room_id, header_str, i);
        } else if (strcmp(key, "subscriber") == 0) {
            i += parse_bool(&header->subscriber, header_str, i);
        } else if (strcmp(key, "tmi-sent-ts") == 0) {
            i += parse_token(header->tmi_sent_ts, header_str, i);
        } else if (strcmp(key, "turbo") == 0) {
            i += parse_bool(&header->turbo, header_str, i);
        } else if (strcmp(key, "user-id") == 0) {
            i += parse_token(header->user_id, header_str, i);
        } else if (strcmp(key, "user-type") == 0) {
            i += parse_token(header->user_type, header_str, i);
            header->user_type[0] = '\0';
        }
        i++;
    }
}

void init_irc(Irc *irc) {
    irc->processing_header = false;
    irc->processing_msg = false;
    irc->body[0] = '\0';
    irc->header_str[0] = '\0';
}
