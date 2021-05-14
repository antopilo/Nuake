#ifndef MAP_PARSER_H
#define MAP_PARSER_H

#include <stdbool.h>

#include "libmap.h"
#include "entity.h"
#include "brush.h"
#include "face.h"

bool map_parser_load(const char *map_file);

void token(const char *buf);
void newline();

bool strings_match(const char *lhs, const char *rhs);

void commit_face();
void commit_brush();
void commit_entity();

#endif
