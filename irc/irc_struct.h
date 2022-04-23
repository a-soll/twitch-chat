#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MYPORT "6667"
#define BACKLOG 10
#define BUFF_SIZE 1001
#define COLOR_LEN 11
#define NONCE_LEN 51
#define NAME_LEN 26
#define EMOTES_LEN 650
#define BADGE_LEN 250
#define BADGE_INFO_LEN 55
#define FLAG_LEN 150
#define ID_LEN 50
#define ROOM_ID_LEN 15
#define TMI_SENT_LEN 15
#define USER_ID_LEN 11
#define USER_TYPE_LEN 15
#define KEY_LEN 30

#define MESSAGE_LEN 550
#define USER_LEN 26

#define HEADER_FRAG_LEN 2500
#define BODY_LEN 1700

typedef struct TwitchChat {
    int status;
    int sockfd;
    socklen_t addr_size;
    struct sockaddr_storage their_addr;
    struct addrinfo hints;
    struct addrinfo *res, *p;
    char ipstr[INET6_ADDRSTRLEN];
    int con;

    const char *pw;
    const char *nick;
} TwitchChat;

typedef struct Message {
    char user[USER_LEN];
    char message[MESSAGE_LEN];
} Message;

typedef struct Header {
    char badge_info[BADGE_INFO_LEN];
    char badges[BADGE_LEN];
    char client_nonce[NONCE_LEN];
    char color[COLOR_LEN];
    char display_name[NAME_LEN];
    char emotes[EMOTES_LEN];
    bool first_msg;
    char flags[FLAG_LEN];
    char id[ID_LEN];
    bool mod;
    char room_id[ROOM_ID_LEN];
    bool subscriber;
    char tmi_sent_ts[TMI_SENT_LEN];
    bool turbo;
    char user_id[USER_ID_LEN];
    char user_type[USER_TYPE_LEN];
    int ind;
    char user_name[USER_LEN];
    bool emote_only;
    char reply_parent_display_name[USER_LEN];
    char reply_parent_msg_body[MESSAGE_LEN];
    char reply_parent_msg_id[NONCE_LEN];
    char reply_parent_user_id[USER_ID_LEN];
    char reply_parent_user_login[USER_LEN];
    char msg_id[ID_LEN];
} Header;

typedef struct Irc {
    bool processing_header;
    bool processing_msg;
    char header_str[HEADER_FRAG_LEN];
    char *to_process;
    size_t process_len;
    char body[BODY_LEN];
    Message message;
    Header header;
    char buf[BUFF_SIZE];
} Irc;

void chat_init(TwitchChat *chat);
int chat_send(TwitchChat *chat, const char *msg);
void reset_message(Message *message);
void init_irc(Irc *irc);
void parse_irc(TwitchChat *chat, Irc *irc);
void join_chat(TwitchChat *chat, const char *user, const char *token, const char *channel);
void _init_header(Header *header);
