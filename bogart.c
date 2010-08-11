#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdarg.h>

#include "bogart.h"

// hax!  libevent doesn't export evkeyvalq definition properly
#include <sys/queue.h>
TAILQ_HEAD (evkeyvalq, evkeyval);

#define BOGART_NOT_FOUND_DEFAULT ^ void (Request * request, Response * response) {}


Map makeMap(void * dummy, ...) {
    va_list ap;
    char * key, * value;
    Map newMap = Trie_new();

    va_start(ap, dummy);
    while((key = va_arg(ap, char *)) && (value = va_arg(ap, char *))) {
        Trie_add(newMap, key, strlen(key), value);
    }

    return newMap;
}

Response * Response_new(struct evbuffer * buffer) {
    Response * r = (Response *) malloc(sizeof(Response));
    r->code = 200;
    r->buffer = buffer;
    return r;
}

void Response_free(Response * r) {
    free(r);
}

Request * Request_new(struct evhttp_request * ev_req) {
    Request * r = (Request *) malloc(sizeof(Request));
    r->ev_req = ev_req;
    r->uri = evhttp_request_uri(ev_req);
    r->params = (struct evkeyvalq *) malloc(sizeof(struct evkeyvalq));
    evhttp_parse_query(r->uri, r->params);
    return r;
}

void Request_free(Request * r) {
    free(r);
}


bool match_uri(const char * pattern, const char * uri) {
    while(*pattern && *uri) {
        if(*pattern == *uri) {
            pattern++;
            uri++;
        } else if(*pattern == '*') {
            if(*uri == '/' || !*(uri+1)) {
                pattern++;
            } else {
                uri++;
            }
        } else {
            return false;
        }
    }
    return (!*pattern && !*uri) || (!*pattern && *uri);
}

Route * match_route(Route * route, Request * req) {
    while(route) {
        if(req->ev_req->type == route->type && match_uri(route->pattern, req->ev_req->uri))
            return route;
        route = route->next;
    } 

    return NULL;
}

Route * nextRoute(char * pattern, enum evhttp_cmd_type type, BogartContext * bogart) {
    Route * new_route = (Route *) malloc(sizeof(Route));
    new_route->pattern = pattern;
    if(bogart->route) {
        Route * cursor = bogart->route;
        while(cursor->next)
            cursor = cursor->next;
        cursor->next = new_route;
    } else {
        bogart->route = new_route;
    }
    return new_route;
}

const char * getParam(Request * request, const char * key) {
    return evhttp_find_header(request->params, key);
}

void renderText(Response * response, char * template, Map args) {
    char anchor[] = "%{";
    char * cursor;
    char * val;
    while((cursor = strstr(template, anchor))) {
        evbuffer_add(response->buffer, template, cursor - template);
        template = cursor + sizeof(anchor) - 1;
        cursor = strchr(template, '}');
        val = Trie_get(args, template, cursor - template);
        evbuffer_add(response->buffer, val, strlen(val));
        template = cursor + 1;
    }
    evbuffer_add(response->buffer, template, strlen(template));
}

char * loadTemplate(char * filename) {
    char * result;
    int size = 0;
    FILE *f = fopen(filename, "r");
    if(!f) return NULL;

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    result = (char *) malloc(size + 1);
    fread(result, sizeof(char), size, f);
    fclose(f);

    result[size] = 0;

    return result;
}

void renderTemplate(Response * response, char * filename, Map args) {
    char * template = loadTemplate(filename);
    renderText(response, template, args);
    free(template);
}

void setBody(Response * response, const char * pattern, ...) {
    va_list ap;
    va_start(ap, pattern);
    evbuffer_add_vprintf(response->buffer, pattern, ap);
}

void finalizeRoutes(Route * route) {
    while(route) {
        route->handler = Block_copy(route->handler);
        route = route->next;
    }
}

void setupHandlers(BogartContext *);

void request_handler(struct evhttp_request * ev_req, void * context) {
    struct timeval t0, t1, tr;

    BogartContext * bogart = (BogartContext *) context;

    gettimeofday(&t0, NULL);
    
    Request * request = Request_new(ev_req);
    Response * response = Response_new(evbuffer_new());

    Route * matched_route = match_route(bogart->route, request);

    if(matched_route) {
        matched_route->handler(request, response);
        evhttp_send_reply(ev_req, response->code, "OK", response->buffer);
    } else {
        bogart->not_found(request, response);
        evhttp_send_reply(ev_req, 404, "Not Found", response->buffer);
    }

    evbuffer_free(response->buffer);

    gettimeofday(&t1, NULL);
    timersub(&t1, &t0, &tr);
    printf("Request processed in: %ld secs, %d usecs\n", tr.tv_sec, tr.tv_usec);
}

void setupBogart(BogartContext * bogart) {
    bogart->not_found = BOGART_NOT_FOUND_DEFAULT;
    setupHandlers(bogart);
}

void startBogart(uint16_t port, BogartContext * bogart) {
    bogart->port = port;

    setupBogart(bogart);

    struct event_base * base = event_init();
    struct evhttp * http = evhttp_new(base);
    evhttp_bind_socket(http, "0.0.0.0", bogart->port);
    evhttp_set_gencb(http, request_handler, bogart);

    printf("Showtime! Bogart's ready on camera %u...\n", bogart->port);

    event_loop(0);

    evhttp_free(http);
}
/*
void modelCreate(RedisModel model, char * fields[], char * key, Map attrs) {
    int i;
    for(i = 0; i < model.fieldCount ; i++) {
        char * field = fields[i];
        char * attr = Trie_get(attrs, field, strlen(field));
        if(attr)
            redisCommand(model.redisFd, "HSET %s:%s %s %s", model.name, key, field, attr);
    }
}

Map modelGet(RedisModel model, char * fields[], char * key) {
    int i;
    Map record = Trie_new();
    for(i = 0; i < model.fieldCount ; i++) {
        char * field = fields[i];
        redisReply * reply = redisCommand(model.redisFd, "HGET %s:%s %s", model.name, key, field);
        if(reply->type == REDIS_REPLY_STRING)
            Trie_add(record, field, strlen(field), reply->reply);
    }
}
*/
