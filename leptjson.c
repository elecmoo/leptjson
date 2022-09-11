#include <assert.h>
#include <stdlib.h>
#include "leptjson.h"

#define EXPECT(ctx, ch) do { assert(*ctx->json == (ch)); ctx->json++; } while(0)

typedef struct {
    const char *json;
} lept_context;

static void lept_parse_whitespace(lept_context *c);

static int lept_parse_null(lept_context *c, lept_value *v);

static int lept_parse_value(lept_context *c, lept_value *v);

static int lept_parse_true(lept_context *c, lept_value *v);

static int lept_parse_false(lept_context *c, lept_value *v);

int lept_parse(lept_value *v, const char *json) {
    lept_context c;
    assert(v != NULL);
    int ret;
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_RETURN_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0')
            ret = LEPT_RETURN_PARSE_ROOT_NOT_SINGULAR;
    }

    return ret;
}

static void lept_parse_whitespace(lept_context *c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_null(lept_context *c, lept_value *v) {
    EXPECT(c, 'n');
    // EXPECT宏中已经判断了首个字符是否为'n'，如果是再后移一位
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_RETURN_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_RETURN_PARSE_OK;
}

static int lept_parse_true(lept_context *c, lept_value *v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_RETURN_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_RETURN_PARSE_OK;
}

static int lept_parse_false(lept_context *c, lept_value *v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_RETURN_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_RETURN_PARSE_OK;
}

static int lept_parse_value(lept_context *c, lept_value *v) {
    switch (*c->json) {
        case 'n':
            return lept_parse_null(c, v);
        case 't':
            return lept_parse_true(c, v);
        case 'f':
            return lept_parse_false(c, v);
        case '\0':
            // 到达字符串末尾
            return LEPT_RETURN_PARSE_EXPECT_VALUE;
        default:
            return LEPT_RETURN_PARSE_INVALID_VALUE;
    }
}

lept_type lept_get_type(const lept_value *v) {
    assert(v != NULL);
    return v->type;
}
