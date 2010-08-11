#include <event.h>
#include <evhttp.h>
#include <stdbool.h>
#include <Block.h>
#include <string.h>
#include <hiredis.h>
#include "trie.h"

typedef struct {
    const char * uri;
    struct evkeyvalq * params;
    struct evhttp_request * ev_req;
} Request;

typedef struct {
    int code;
    struct evbuffer * buffer;
} Response;

typedef void (^Handler)(Request *, Response *);

typedef struct _Route {
    enum evhttp_cmd_type type;
    char * pattern;
    Handler handler;
    struct _Route * next;
} Route;

typedef struct {
    uint16_t port;
    void (^ init_func)();
    Handler not_found;
    Route * route;
} BogartContext;

#define Bogart \
    BogartContext globalContext; \
    void setupHandlers(BogartContext * bogart)

#define Start(_port)                            \
        finalizeRoutes(bogart->route);         \
    }                                           \
    int main() {                                \
        startBogart(_port, &globalContext);     \
        return 0;

#define get(_pattern)                                                   \
    nextRoute(_pattern, EVHTTP_REQ_GET, bogart)->handler = ^ void (Request * request, Response * response)

#define post(_pattern)                                                  \
    nextRoute(_pattern, EVHTTP_REQ_POST, bogart)->handler = ^ void (Request * request, Response * response)

#define status(_status) \
    response->code = _status;

#define body(_pattern, ...)                     \
    setBody(response, _pattern, ##__VA_ARGS__)

#define params(_key) getParam(request, _key)

const char * getParam(Request *, const char *);
void setBody(Response *, const char *, ...);
void startBogart(uint16_t, BogartContext *);
Route * nextRoute(char *, enum evhttp_cmd_type, BogartContext *);
void finalizeRoutes(Route *);

typedef struct {
    char * key;
    char * value;
} CharTuple;

typedef Trie * Map;

#define map(...) makeMap(NULL, __VA_ARGS__, NULL)
Map makeMap(void *, ...);

#define render(_template, _map) renderText(response, _template, _map)
void renderText(Response *, char *, Map);

#define view(_filename, _map) renderTemplate(response, _filename, _map)
void renderTemplate(Response *, char *, Map);

/*
typedef struct _ModelField {
    char * fieldName;
    struct _ModelField * next;
} ModelField;

typedef struct {
    char * name;
    int redisFd;
    ModelField * field;
} RedisModel;

void modelCreate(RedisModel, char **, char *, Map);
Map modelGet(RedisModel, char **, char *);

#define Model(_name, ...)                   \
    char * _name##_fields[] = __VA_ARGS__; \
    RedisModel _name##Model = { #_name, _redisFd, _name##_fields, sizeof(_name##_fields) }
*/

#define UseRedis                                      \
    int _redisFd;                                     \
    redisConnect(&_redisFd, "127.0.0.1", 6379)
