#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include "leptjson.h"

#define EXPECT(ctx, ch) do { assert(*ctx->json == (ch)); ctx->json++; } while(0)

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')

#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char *json;
} lept_context;

static void lept_parse_whitespace(lept_context *c);

static int lept_parse_value(lept_context *c, lept_value *v);

int lept_parse(lept_value *v, const char *json) {
    lept_context c;
    assert(v != NULL);
    int ret;
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_RETURN_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            // 注意，如果解析字面值之后，后面仍然有字符，说明字符不单一，解析失败，务必将type设置为LETP_NULL
            v->type = LEPT_NULL;
            ret = LEPT_RETURN_PARSE_ROOT_NOT_SINGULAR;
        }

    }

    return ret;
}

static void lept_parse_whitespace(lept_context *c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context *c, lept_value *v, const char *literal, lept_type type) {
    // 注意，EXPECT宏会将c->json指针向后移动一位
    EXPECT(c, literal[0]);

    size_t i; // i是数组的索引，最好申明为size_t类型
    for (i = 0; literal[i + 1]; i++) {
        // 注意，由于指针向后移动了一位，需要从literal的第i+1位置开始比较
        if (c->json[i] != literal[i + 1])
            return LEPT_RETURN_PARSE_INVALID_VALUE;
    }
    c->json += i;
    v->type = type;
    return LEPT_RETURN_PARSE_OK;
}

static int lept_parse_number(lept_context *c, lept_value *v) {
    const char *p = c->json;
    // 第一位如果是负号，直接跳过
    if (*p == '-')
        p++;
    // 数字要么是一个单独的0，或者是一个1-9开头的数
    if (*p == '0')
        p++;
    else {
        if (!ISDIGIT1TO9(*p))
            return LEPT_RETURN_PARSE_INVALID_VALUE;
        // 跳过后面的整数
        for (p++; ISDIGIT(*p); p++);
    }
    // 如果整数后面出现小数点，则跳过小数点，且后面至少跟一位整数
    if (*p == '.') {
        p++;
        // 小数点后面至少一位整数
        if (!ISDIGIT(*p))
            return LEPT_RETURN_PARSE_INVALID_VALUE;
        // 跳过后面的整数
        for (p++; ISDIGIT(*p); p++);
    }
    // 判断指数部分
    if (*p == 'e' || *p == 'E') {
        p++;
        // e/E后面可能紧接着一个正号或者负号
        if (*p == '-' || *p == '+')
            p++;
        // 后面至少跟一位整数
        if (!ISDIGIT(*p))
            return LEPT_RETURN_PARSE_INVALID_VALUE;
        // 跳过后面的整数
        for (p++; ISDIGIT(*p); p++);
    }

//    const char *pre, *p = c->json;
//    for (; *p != '\0'; pre = p, p++) {
//        if (!ISDIGIT(*p) && *p != '-' && *p != 'e' && *p != 'E' && *p != '.' && *p != '+')
//            return LEPT_RETURN_PARSE_INVALID_VALUE;
//        else if (p == c->json && (*p == '.' || *p == '+'))
//            return LEPT_RETURN_PARSE_INVALID_VALUE;
//    }
//    if (*pre == '.')
//        return LEPT_RETURN_PARSE_INVALID_VALUE;

    errno = 0; // 先将errno设置为0，成功
    v->n = strtod(c->json, NULL);
    // 判断数值是否过大，超出了double的承载范围了
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    c->json = p;
    v->type = LEPT_NUMBER;
    return LEPT_RETURN_PARSE_OK;
}

void lept_free(lept_value *v) {
    assert(v != NULL);
    // 仅仅当类型为string时，去释放内存
    if (v->type == LEPT_STRING)
        free(v->s);
    v->type = LEPT_NULL;
}

inline void lept_init(lept_value *v) {
    v->type = LEPT_NULL;
}

inline void lept_set_null(lept_value *v) {
    lept_init(v);
}

static int lept_parse_value(lept_context *c, lept_value *v) {
    switch (*c->json) {
        case 'n':
            return lept_parse_literal(c, v, "null", LEPT_NULL);
        case 't':
            return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':
            return lept_parse_literal(c, v, "false", LEPT_FALSE);
        default:
            return lept_parse_number(c, v);
        case '\0':
            // 到达字符串末尾
            return LEPT_RETURN_PARSE_EXPECT_VALUE;
    }
}

lept_type lept_get_type(const lept_value *v) {
    assert(v != NULL);
    return v->type;
}


int lept_get_boolean(const lept_value *v) {
    // TODO:
}

void lept_set_boolean(lept_value *v, int b) {
    // TODO:
}

double lept_get_number(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}

void lept_set_number(lept_value *v, double n) {
    // TODO:
}

const char *lept_get_string(const lept_value *v) {
    // TODO:
}

size_t lept_get_string_length(const lept_value *v) {
    // TODO:
}

void lept_set_string(lept_value *v, const char *s, size_t len) {
    assert(v != NULL && s != NULL);
    // 分配内存前，先将v的内存释放下
    lept_free(v);
    v->s = malloc(len + 1);
    memcpy(v->s, s, len);
    v->s[len] = '\0';
    v->len = len;
    v->type = LEPT_STRING;
}
