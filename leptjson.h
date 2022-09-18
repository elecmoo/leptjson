#ifndef LEPTJSON_LEPTJSON_H
#define LEPTJSON_LEPTJSON_H

// lept
typedef enum {
    LEPT_NULL, LEPT_TRUE, LEPT_FALSE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT, LEPT_PARSE_NUMBER_TOO_BIG
} lept_type;

typedef struct {
    union {
        struct {
            char *s;
            size_t len;
        }; // 存储string
        double n; // 存储number
    };

    lept_type type;
} lept_value;

int lept_parse(lept_value *v, const char *json);

lept_type lept_get_type(const lept_value *v);

void lept_free(lept_value *v);

void lept_init(lept_value *v);

void lept_set_null(lept_value *v);

int lept_get_boolean(const lept_value *v);

void lept_set_boolean(lept_value *v, int b);

double lept_get_number(const lept_value *v);

void lept_set_number(lept_value *v, double n);

const char *lept_get_string(const lept_value *v);

size_t lept_get_string_length(const lept_value *v);

void lept_set_string(lept_value *v, const char *s, size_t len);

// 返回值定义
enum {
    LEPT_RETURN_PARSE_OK = 0,
    LEPT_RETURN_PARSE_EXPECT_VALUE,
    LEPT_RETURN_PARSE_INVALID_VALUE,
    LEPT_RETURN_PARSE_ROOT_NOT_SINGULAR,
    LEPT_RETURN_PARSE_MISS_QUOTATION_MARK,
};

#endif //LEPTJSON_LEPTJSON_H
