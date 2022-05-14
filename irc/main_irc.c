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

void init_irc(Irc *irc) {
    irc->finished = false;
    irc->message.message[0] = '\0';
    irc->message.user[0] = '\0';
    irc->message.size = 0;
    irc->size = 0;
    init_iterator(&irc->iterator);
    init_header(&irc->header);
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

void pong_check(Irc *irc, TwitchChat *chat, int i) {
    Iterator *iter = &irc->iterator;
    if (strcmp(iter->header_str, "PING") == 0) {
        if (irc->buf[i + 1] == ':') {
            chat_send(chat, "PONG :tmi.twitch.tv\r\n");
        }
    }
}
