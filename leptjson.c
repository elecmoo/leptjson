#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include "leptjson.h"

#ifndef LEPT_PARSE_STACK_INIT_SIZE
// 栈初始化大小
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

// 判断字符是否为0-9
static inline bool is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

// 判断字符是否为1-9
static inline bool is_digit_1_to_9(char ch) {
    return ch >= '1' && ch <= '9';
}

typedef struct {
    const char *json;
    char *stack;
    size_t size, top;
} lept_context;

// 判断字符串的首字母是否为预期值，不是则退出程序，是则将程序将指针后移一位
static inline void expect(lept_context *ctx, char ch) {
    assert(*ctx->json == ch);
    ctx->json++;
}

static void lept_parse_whitespace(lept_context *c);

static int lept_parse_value(lept_context *c, lept_value *v);

int lept_parse(lept_value *v, const char *json) {
    lept_context c;
    assert(v != NULL);
    int ret;
    c.json = json;
    c.stack = NULL;
    c.size = 0;
    c.top = 0;
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
    assert(c.top == 0); // 确保解析结束时，栈中所有的元素均被弹出
    free(c.stack); // 解析结束释放栈内存

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
    expect(c, literal[0]);

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
        if (!is_digit_1_to_9(*p))
            return LEPT_RETURN_PARSE_INVALID_VALUE;
        // 跳过后面的整数
        for (p++; is_digit(*p); p++);
    }
    // 如果整数后面出现小数点，则跳过小数点，且后面至少跟一位整数
    if (*p == '.') {
        p++;
        // 小数点后面至少一位整数
        if (!is_digit(*p))
            return LEPT_RETURN_PARSE_INVALID_VALUE;
        // 跳过后面的整数
        for (p++; is_digit(*p); p++);
    }
    // 判断指数部分
    if (*p == 'e' || *p == 'E') {
        p++;
        // e/E后面可能紧接着一个正号或者负号
        if (*p == '-' || *p == '+')
            p++;
        // 后面至少跟一位整数
        if (!is_digit(*p))
            return LEPT_RETURN_PARSE_INVALID_VALUE;
        // 跳过后面的整数
        for (p++; is_digit(*p); p++);
    }

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
    assert(v != NULL && v->type == LEPT_STRING);
    return v->s;
}

size_t lept_get_string_length(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->len;
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

static void *lept_context_push(lept_context *c, size_t size) {
    void *ret;
    if (c->top + size >= c->size) {
        if (c->size == 0) {
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        }
        while (c->top + size >= c->size) {
            c->size += c->size >> 2; // 将c->size增加为原来的1.5倍
        }
        c->stack = realloc(c->stack, c->size); // 重新为stack分配内存
    }
    ret = c->stack + c->top;
    c->top = c->top + size;
    return ret;
}

static void *lept_context_pop(lept_context *c, size_t size) {
    assert(c->top >= size);
    c->top -= size;
    return c->stack + c->top;
}

static inline void put_c(lept_context *c, char ch) {
    // 先为字符分配内存，然后将字符存储到该位置
    char *p = lept_context_push(c, sizeof(char));
    *p = ch;
}

static int lept_parse_string(lept_context *c, lept_value *v) {
    size_t head = c->top, len;
    const char *p = c->json;
    expect(c, '\"');
    for (;;) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                // 解析到json字符串末尾了
                len = c->top - head;
                // 从栈中弹出字符串，并将其设置到value中
                const char *s = lept_context_pop(c, len);
                lept_set_string(v, s, len);
                c->json = p;
                return LEPT_RETURN_PARSE_OK;
            case '\0':
                // 解析到c语言字符串的末尾也没有找到第二个双引号，解析失败
                c->top = head; // 恢复栈的位置
                return LEPT_RETURN_PARSE_MISS_QUOTATION_MARK;
            default:
                put_c(c, ch);
        }
    }
}

