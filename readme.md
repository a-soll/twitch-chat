[Twitch IRC docs](https://dev.twitch.tv/docs/irc/guide)

# Usage example

```c
#include <twitchchat/twitchchat.h>

int main() {
    TwitchChat chat;
    chat_init(&chat);
    chat_send(&chat, "pass oauth:<chat token>\n");
    chat_send(&chat, "nick swifcheese\n");
    chat_send(&chat, ":swifcheese!swifcheese@swifcheese.tmi.twitch.tv JOIN #xqcow\n");

    // chat tag for extra msg data https://dev.twitch.tv/docs/irc/tags#overview
    chat_send(&chat, "CAP REQ :twitch.tv/tags\n");
    Irc irc;
    init_irc(&irc);
    while (1) {
        parse_irc(&chat, &irc);
        printf("[%s]%s: %s\n", irc.header.color, irc.message.user, irc.message.message);
        // prints: [#008000]foley83: game spoils itself haha
    }
}
```

Example IRC header:
```
"@badge-info=;badges=premium/1;client-nonce=651f690d8301f4d3b22773d6ed241c7f;color=#0000FF;display-name=tempest0128;emotes=;first-msg=0;flags=;id=c0abcd40-ba08-47d4-853c-b050d8b8e2ae;mod=0;reply-parent-display-name=imacutieeee1;reply-parent-msg-body=TYLER1\\sFANS\\sACTING\\sLIKE\\sTHEY\\sALSO\\sARE\\sNOT\\sVIRGINS\\sOMEGALUL\\s\\sTYLER1\\sFANS\\sACTING\\sLIKE\\sTHEY\\sALSO\\sARE\\sNOT\\sVIRGINS\\sOMEGALUL\\s\\sTYLER1\\sFANS\\sACTING\\sLIKE\\sTHEY\\sALSO\\sARE\\sNOT\\sVIRGINS\\sOMEGALUL\\s\\sTYLER1\\sFANS\\sACTING\\sLIKE\\sTHEY\\sALSO\\sARE\\sNOT\\sVIRGINS\\sOMEGALUL\\s\\sTYLER1\\sFANS\\sACTING\\sLIKE\\sTHEY\\sALSO\\sARE\\sNOT\\sVIRGINS\\sOMEGALUL\\s\\sTYLER1\\sFANS\\sACTING\\sLIKE\\sTHEY\\sALSO\\sARE\\sNOT\\sVIRGINS\\sOMEGALUL;reply-parent-msg-id=ae39001b-fd5b-4aa2-ac9c-4763d87de9a1;reply-parent-user-id=775608410;reply-parent-user-login=imacutieeee1;room-id=51496027;subscriber=0;tmi-sent-ts=1649462985172;turbo=0;user-id=530148568;user-type="
```

# Structs
`TwitchChat` contains everything needed for chat connectivity. Create the struct, init it, and then use `chat_send` to send messages.

`IRC` is the main header for chat message data. It contains both `Header` and `Message` structs, as long as housing other data needed for parsing IRC messages from Twitch. It is only used for storing the current chat message data.

- `Header` contains all of the keys for a the header in an IRC message. These keys contain information about the actual message, like emotes, username color, subscriber and length of subscription, etc.
- `Message` contains the username and message text.
- on each iteration, your code can work with the information in IRC to display chat messages in whatever way you need.

# Contributing
The header hash map is generated using gperf. If there is a new header key that needs to be added, add it to header_table_generator.gperf and run gperf on it. Copy the new contents of the generated wordlist, asso_values, and #define values to parse.c.

For example, if I need to add a key called msg-id, I would add `msg-id, &parse_token, NULL, offsetof(Header, msg_id)`
- msg-id is the string to map from Twitch's header string. Because it is a char, I would add a poiner to `parse_token`, and `NULL` for `parse_bool`
- I would then add the corresponding member `msg_id[ID_LEN]` to `irc_struct.h`
- the last field is the offset of `msg_id` so that the code can dynamically grab the correct header member to pass to the parse function
