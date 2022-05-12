#ifndef CLIENT_H
#define CLIENT_H

#include <curl/curl.h>
#include <json-c/json.h>

#define URL_LEN 2048

typedef enum CurlMethod {
    curl_POST,
    curl_GET,
    curl_DELETE,
    curl_PATCH
} CurlMethod;

typedef struct Response {
    struct json_object *response;
    struct json_object *data;
    struct json_object *data_array_obj;
    CURLcode res;
    int data_len;
    char *memory;
    size_t size;
    char error[CURL_ERROR_SIZE];
} Response;

typedef struct Client {
    const char *base_url;
    const char *client_id;
    const char *client_secret;
    const char *token;
    struct json_object *fields;
    struct curl_slist *headers;
    CURL *curl_handle;
    char post_data[999];
} Client;

Client Client_init(const char *client_id, const char *access_token);
void Client_deinit(Client *c);
Response curl_request(Client *client, const char *url, CurlMethod method);
void Client_startup(void);
void clean_up(void *client);
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
void reset_headers(Client *client);
void clear_headers(Client *client);
void clean_response(void *response);

#endif /* client_h */
