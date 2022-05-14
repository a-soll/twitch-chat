//
//  client.c
//
//  Created by Adam Solloway on 3/8/22.
//

#include "client.h"
#include "util.h"
#include <string.h>

Client Client_init(const char *client_id, const char *access_token) {
    Client c;
    c.base_url = "https://api.twitch.tv/helix";
    c.client_id = client_id;
    c.token = access_token;
    c.headers = NULL;
    c.curl_handle = NULL;

    char header[URL_LEN];
    c.headers = curl_slist_append(c.headers, "Content-Type: application/json");
    c.headers = curl_slist_append(c.headers, "Accept: application/json");
    fmt_string(header, URL_LEN, "Authorization: Bearer %s", c.token);
    c.headers = curl_slist_append(c.headers, header);

    fmt_string(header, URL_LEN, "client-Id: %s", c.client_id);
    c.headers = curl_slist_append(c.headers, header);
    return c;
}

void Client_deinit(Client *c) {
    json_object_put(c->fields);
    curl_slist_free_all(c->headers);
}

Response curl_request(Client *client, const char *url, CurlMethod method) {
    Response response;
    response.memory = malloc(1);
    response.size = 0;
    client->curl_handle = curl_easy_init();
    response.error[0] = 0;

    switch (method) {
    case curl_GET:
        curl_easy_setopt(client->curl_handle, CURLOPT_CUSTOMREQUEST, "GET");
        break;
    case curl_POST:
        curl_easy_setopt(client->curl_handle, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(client->curl_handle, CURLOPT_POSTFIELDS, client->post_data);
        break;
    case curl_DELETE:
        curl_easy_setopt(client->curl_handle, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(client->curl_handle, CURLOPT_POSTFIELDS, client->post_data);
        break;
    case curl_PATCH:
        curl_easy_setopt(client->curl_handle, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(client->curl_handle, CURLOPT_POSTFIELDS, client->post_data);
        break;
    default:
        curl_easy_setopt(client->curl_handle, CURLOPT_CUSTOMREQUEST, "GET");
    }

    curl_easy_setopt(client->curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(client->curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(client->curl_handle, CURLOPT_HTTPHEADER, client->headers);
    curl_easy_setopt(client->curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(client->curl_handle, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(client->curl_handle, CURLOPT_ERRORBUFFER, response.error);

    // curl_easy_setopt(client->curl_handle, CURLOPT_VERBOSE, client->curl_handle);

    response.res = curl_easy_perform(client->curl_handle);
    response.response = json_tokener_parse(response.memory);
    clean_up(client);
    return response;
}

void clean_response(void *response) {
    struct Response *res = (struct Response *)response;
    free(res->memory);
}

void clean_up(void *client) {
    struct Client *mem = (struct Client *)client;
    curl_easy_cleanup(mem->curl_handle);
}

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct Response *mem = (struct Response *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        printf("error: not enough memory\n");
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

void reset_headers(Client *client) {
    clear_headers(client);
    char header[100];

    fmt_string(header, 100, "Authorization: Bearer %s", client->token);
    client->headers = curl_slist_append(client->headers, header);

    fmt_string(header, 100, "client-Id: %s", client->client_id);
    client->headers = curl_slist_append(client->headers, header);
}

void clear_headers(Client *client) {
    curl_slist_free_all(client->headers);
    client->headers = NULL;
}
