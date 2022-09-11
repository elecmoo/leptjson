#ifndef LEPTJSON_LEPTJSON_H
#define LEPTJSON_LEPTJSON_H

// lept
typedef enum {
    LEPT_NULL, LEPT_TRUE, LEPT_FALSE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT
} lept_type;

typedef struct {
    lept_type type;
} lept_value;

int lept_parse(lept_value *v, const char *json);

lept_type lept_get_type(const lept_value *v);

// 返回值定义
enum {
    LEPT_RETURN_PARSE_OK = 0,
    LEPT_RETURN_PARSE_EXPECT_VALUE,
    LEPT_RETURN_PARSE_INVALID_VALUE,
    LEPT_RETURN_PARSE_ROOT_NOT_SINGULAR
};

#endif //LEPTJSON_LEPTJSON_H
