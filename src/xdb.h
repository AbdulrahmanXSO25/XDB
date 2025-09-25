#ifndef XDB_H
#define XDB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define XDB_VERSION "0.0.1"

#define PLACEHOLDER

#define MAX_INPUT_LEN 512

#define MAX_REL_NAME_LEN 32
#define MAX_ATTR_NAME_LEN 32

#define MAX_RELS 16
#define MAX_ATTRS 16
#define MAX_TUPLES 1024

#define ATTR_TYPE_INT 1
#define ATTR_TYPE_BIGINT 2
#define ATTR_TYPE_FLOAT 3
#define ATTR_TYPE_DOUBLE 4
#define ATTR_TYPE_CHAR 5
#define ATTR_TYPE_BOOL 6
#define ATTR_TYPE_TIMESTAMP 7
#define ATTR_TYPE_TEXT 8

typedef struct {
    char name[MAX_ATTR_NAME_LEN];
    uint8_t type;
    uint16_t size;
} AttrDesc;

typedef struct {
    uint8_t nattrs;
    AttrDesc attrs[MAX_ATTRS];
} TupleDesc;

typedef union {
    int32_t   i32;
    int64_t   i64;
    char      c;
    bool      b;
    float     f;
    double    d;
    int64_t   ts;
    char     *str;
} Value;

typedef struct {
    Value *values;
} Tuple;

typedef struct {
    char name[MAX_REL_NAME_LEN];
    TupleDesc tuple_desc;
    Tuple tuples[MAX_TUPLES];
    uint32_t ntuples;
} Relation;

#endif /* XDB_H */