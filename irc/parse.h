#include "irc_struct.h"

void parse_message(Message *message, char *msg_str);
int chat_recv(TwitchChat *chat, char buf[BUFF_SIZE]);
void parse_header(Header *header, char *header_str, int len);
char *lookahead(char *str, int position, int end);
void parse_header_reply(char *field, char header_str[], int cur_ind);
