/*
    Display an animating text bar as a video overlay
    Copyright (C) 2020  Lester Vecsey

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ACTIONBAR_H
#define ACTIONBAR_H

#include <stdint.h>

#include "mini_gxkit.h"

#include "pixcoord.h"

typedef struct {

  uint64_t state;

  pixcoord base_start, base_end;

  long int xw, yh;

  pixel_t box_bgcolor;

  double box_alpha;
  
  pixel_t fill_color;

  char *text_str;
  
} actionbar;

#include <ft2build.h>

int draw_actionbar(image_t *img, FT_Face *face, actionbar *ab, double v);

#endif
