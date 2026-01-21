#pragma once

#include <assert.h>
#include <stddef.h>


#define FIELD_H 15
#define FIELD_W 20
#define FIELD_LEN (FIELD_H * FIELD_W)

typedef enum {
    EntityType_None = 0,
    EntityType_Vent,
    EntityType_Tank,
    EntityType_PipeUp,
    EntityType_Pump,
    EntityType_Block0,
    EntityType_Block1,
} EntityType;

typedef struct {
    EntityType type[FIELD_LEN];
    int oil[FIELD_LEN];
    int oil_limit[FIELD_LEN];
} Field;

static inline size_t to_index(size_t x, size_t y) {
    assert(x < FIELD_W && y < FIELD_H && "Indexing out of game field bounds");
    return y * FIELD_W + x;
}

static inline void from_index(size_t i, int *x, int *y) {
    *x = i % FIELD_W;
    *y = i / FIELD_W;
}

