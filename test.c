#include <stdio.h>
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

#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")


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

    TEST_ERROR(LEPT_RETURN_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
    TEST_ERROR(LEPT_RETURN_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(LEPT_RETURN_PARSE_ROOT_NOT_SINGULAR, "0x123");
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
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_number_too_big(void) {
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
}

#define TEST_STRING(expect, json)\
    do {\
        lept_value v;\
        lept_init(&v);\
        EXPECT_EQ_INT(LEPT_RETURN_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));\
        EXPECT_EQ_STRING(expect, lept_get_string(&v), lept_get_string_length(&v));\
        lept_free(&v);\
    } while(0)

static void test_parse_string(void) {
    TEST_STRING("hello", "\"hello\"");
    TEST_STRING("", "\"\"");
    TEST_STRING("hello world!", "\"hello world!\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

static void test_parse_missing_quotation_mark(void) {
    TEST_ERROR(LEPT_RETURN_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(LEPT_RETURN_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape(void) {
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char(void) {
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(LEPT_RETURN_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_access_null(void) {
    lept_value v;
    lept_init(&v);
    lept_set_string(&v, "a", 1);
    lept_set_null(&v);
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
    lept_free(&v);
}

static void test_access_string(void) {
    lept_value v;
    lept_init(&v);
    lept_set_string(&v, "", 0);
    EXPECT_EQ_STRING("", lept_get_string(&v), lept_get_string_length(&v));
    lept_set_string(&v, "Hello", 5);
    EXPECT_EQ_STRING("Hello", lept_get_string(&v), lept_get_string_length(&v));
    lept_free(&v);
}

static void test_access_boolean(void) {
    lept_value v;
    lept_init(&v);
    lept_set_string(&v, "a", 1);
    lept_set_boolean(&v, 1);
    EXPECT_TRUE(lept_get_boolean(&v));
    lept_set_boolean(&v, 0);
    EXPECT_FALSE(lept_get_boolean(&v));
    lept_free(&v);
}

static void test_access_number(void) {
    lept_value v;
    lept_init(&v);
    lept_set_string(&v, "a", 1);
    lept_set_number(&v, 1234.5);
    EXPECT_EQ_DOUBLE(1234.5, lept_get_number(&v));
    lept_free(&v);
}

static void test_parse(void) {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();

    test_parse_return_root_not_singular();
    test_parse_return_except_value();
    test_parse_return_invalid_value();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();

    test_access_string();
    test_access_number();
    test_access_boolean();
    test_access_null();
}

int main(void) {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
