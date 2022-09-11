#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)
// 注意上面的while(0)后面是没有;的

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")


#define TEST_ERROR(error, json)\
    do {\
        lept_value v;\
        v.type = LEPT_FALSE;\
        EXPECT_EQ_INT(error, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));\
    } while(0)

#define TEST_NUMBER(expect, json)\
    do{\
        lept_value v;\
        EXPECT_EQ_INT(LEPT_RETURN_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
        EXPECT_EQ_DOUBLE(expect, lept_get_number(&v));\
    } while (0)

static void test_parse_null(void) {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_RETURN_PARSE_OK, lept_parse(&v, "null"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

static void test_parse_return_root_not_singular(void) {
    TEST_ERROR(LEPT_RETURN_PARSE_ROOT_NOT_SINGULAR, "null x");
}

static void test_parse_return_except_value(void) {
    TEST_ERROR(LEPT_RETURN_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(LEPT_RETURN_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_return_invalid_value(void) {
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, "True");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, "fa");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, "+0.0");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, ".123");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, "1.");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_VALUE, "nan");
}

static void test_parse_true(void) {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_RETURN_PARSE_OK, lept_parse(&v, "true"));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_RETURN_PARSE_OK, lept_parse(&v, "true "));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_RETURN_PARSE_OK, lept_parse(&v, " true "));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}

static void test_parse_false(void) {
    lept_value v;
    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_RETURN_PARSE_OK, lept_parse(&v, "false"));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));

    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_RETURN_PARSE_OK, lept_parse(&v, "false "));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));

    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_RETURN_PARSE_OK, lept_parse(&v, " false "));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
}

static void test_parse_number(void) {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(10.0, "10");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(3.14, "3.14");
    TEST_NUMBER(-3.14, "-3.14");
    TEST_NUMBER(3.1415926, "3.1415926");
    TEST_NUMBER(-3.1415926, "-3.1415926");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
}

static void test_parse(void) {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_return_root_not_singular();
    test_parse_return_except_value();
    test_parse_return_invalid_value();

}

int main(void) {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
