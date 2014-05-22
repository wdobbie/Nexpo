#ifndef FONT_H
#define FONT_H

#include <string>
#include "path.h"
#include "common.h"

class Font;

DLLEXPORT Font* loadFont(const char* path);
DLLEXPORT Path* pathForCodepoint(Font* font, int codepoint);
DLLEXPORT void freeFont(Font* font);

#endif // FONT_H
