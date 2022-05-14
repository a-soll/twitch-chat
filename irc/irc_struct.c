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
    message->size = 0;
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

void init_irc(Irc *irc) {
    irc->finished = false;
    irc->message.message[0] = '\0';
    irc->message.user[0] = '\0';
    irc->message.size = 0;
    irc->size = 0;
    _init_iterator(&irc->iterator);
    _init_header(&irc->header);
}

void parse_irc(TwitchChat *chat, Irc *irc) {
    if (irc->finished) {
        irc->finished = false;
        reset_message(&irc->message);
    }
    Iterator *iter = &irc->iterator;

    // request the next buffer chunk once finished with the current
    if (iter->proc_ind == 0) {
        irc->size = chat_recv(chat, irc->buf);
        if (irc->size <= 0) {
            return;
        }
        irc->buf[irc->size] = '\0';
    }

    while (irc->buf[iter->proc_ind] != '\0') {
        if (iter->processing_header) {
            iter->proc_ind += parse_header_line(irc, chat, iter->proc_ind);
        }
        if (iter->processing_msg) {
            iter->proc_ind += parse_msg_line(irc, iter->proc_ind);
            if (!iter->processing_msg) {
                irc->finished = true;
            }
            return;
        }
        if (iter->proc_ind < irc->size) {
            iter->proc_ind++;
        }
    }
    iter->proc_ind = 0;
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
    header->reply_parent_display_name[0] = '\0';
    header->reply_parent_msg_body[0] = '\0';
    header->reply_parent_user_id[0] = '\0';
    header->reply_parent_msg_id[0] = '\0';
    header->reply_parent_user_login[0] = '\0';
    header->room_id[0] = '\0';
}

int parse_header_line(Irc *irc, TwitchChat *chat, int i) {
    Iterator *iter = &irc->iterator;
    if (!iter->header_str[0]) {
        _init_header(&irc->header);
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

void _init_iterator(Iterator *iterator) {
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

void pong_check(Irc *irc, TwitchChat *chat, int i) {
    Iterator *iter = &irc->iterator;
    if (strcmp(iter->header_str, "PING") == 0 ) {
        if (irc->buf[i + 1] == ':') {
            chat_send(chat, "PONG :tmi.twitch.tv\r\n");
        }
    }
}
