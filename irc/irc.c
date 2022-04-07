#include "irc_struct.h"
#include <stdbool.h>
#include <unistd.h>

void parse_irc(TwitchChat *chat, Irc *irc) {
    char buf[BUFF_SIZE];
    char to_post[HEADER_FRAG_LEN + BODY_LEN];
    char *tmp = NULL;
    int hind = 0;
    int bind = 0;
    int sz = chat_recv(chat, buf);
    if (sz == -1) {
        return;
    }
    strcat(irc->to_process, irc->header_str);
    strcat(irc->to_process, irc->body);
    strcat(irc->to_process, buf);

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

int main() {
    TwitchChat chat;
    chat_init(&chat);

    chat_send(&chat, "pass oauth:<token>\n");
    chat_send(&chat, "nick swifcheese\n");
    chat_send(&chat, ":swifcheese!swifcheese@swifcheese.tmi.twitch.tv JOIN #xqcow\n");

    // chat tag for extra msg data https://dev.twitch.tv/docs/irc/tags#overview
    chat_send(&chat, "CAP REQ :twitch.tv/tags\n");
    Irc irc;
    init_irc(&irc);

    while (1) {
        parse_irc(&chat, &irc);
    }
}
