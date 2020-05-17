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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "mini_gxkit.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string.h>

#include "actionbar.h"

#include "writefile.h"

int draw_actionbar(image_t *img, FT_Face *face, actionbar *ab, double v) {

  long int xi, yi;

  long int xpos, ypos;

  pixcoord cur_base;

  pixel_t withalpha_boxbgcolor;

  int glyph_index;

  char *ch;

  int retval;

  unsigned char value;

  pixel_t text_color;
  long int textbase_x;
  long int textbase_y;

  long int overall_textspan;

  long int overall_height;
  
  cur_base.base_xpos = (1.0 - v) * ab->base_start.base_xpos + v * ab->base_end.base_xpos;
  cur_base.base_ypos = (1.0 - v) * ab->base_start.base_ypos + v * ab->base_end.base_ypos;

  withalpha_boxbgcolor.r = ab->box_bgcolor.r * ab->box_alpha;
  withalpha_boxbgcolor.g = ab->box_bgcolor.g * ab->box_alpha;
  withalpha_boxbgcolor.b = ab->box_bgcolor.b * ab->box_alpha;
  
  for (yi = 0; yi < ab->yh; yi++) {
    for (xi = 0; xi < ab->xw; xi++) {

      xpos = cur_base.base_xpos + xi;
      ypos = cur_base.base_ypos + yi;
      
      if (xpos < 0 || xpos >= img->xres) {
	continue;
      }

      if (ypos < 0 || ypos >= img->yres) {
	continue;
      }
      
      img->rgb[ypos * img->xres + xpos] = withalpha_boxbgcolor;
      
    }
  }

  overall_textspan = 0;
  overall_height = 0;
  
  for (ch = ab->text_str; ch[0]; ch++) {

    glyph_index = FT_Get_Char_Index(face[0], ch[0]);

    retval = FT_Load_Glyph(face[0], glyph_index, FT_LOAD_DEFAULT);

    overall_textspan += face[0]->glyph->metrics.horiAdvance / 64;

    if (face[0]->glyph->metrics.height / 64 > overall_height) {
      overall_height = face[0]->glyph->metrics.height / 64;
    }
    
  }    

  textbase_x = 0;
  textbase_y = 0;
  
  for (ch = ab->text_str; ch[0]; ch++) {

    glyph_index = FT_Get_Char_Index(face[0], ch[0]);

    retval = FT_Load_Glyph(face[0], glyph_index, FT_LOAD_DEFAULT);
    
    retval = FT_Render_Glyph(face[0]->glyph, FT_RENDER_MODE_NORMAL);

    if (retval) {
      continue;
    }
    
    textbase_y = 0;

    for (yi = 0; yi < face[0]->glyph->bitmap.rows; yi++) {
      for (xi = 0; xi < face[0]->glyph->bitmap.width; xi++) {

	value = face[0]->glyph->bitmap.buffer[yi * face[0]->glyph->bitmap.pitch + xi];

	text_color.r = (value / 255.0) * ab->fill_color.r + ((255 - value) / 255.0) * withalpha_boxbgcolor.r;
	text_color.g = (value / 255.0) * ab->fill_color.g + ((255 - value) / 255.0) * withalpha_boxbgcolor.g;
	text_color.b = (value / 255.0) * ab->fill_color.b + ((255 - value) / 255.0) * withalpha_boxbgcolor.b;	

	ypos = (textbase_y + yi);	
	xpos = textbase_x + xi;

	ypos += (overall_height - face[0]->glyph->metrics.horiBearingY / 64);
	
	ypos += cur_base.base_ypos;
	xpos += cur_base.base_xpos;

	xpos += ((ab->xw - overall_textspan) >> 1);
	ypos += ((ab->yh - overall_height) >> 1);
	
	if (xpos < 0 || xpos >= img->xres) {
	  continue;
	}

	if (ypos < 0 || ypos >= img->yres) {
	  continue;
	}
	
	img->rgb[ ypos * img->xres + xpos] = text_color;

      }

    }

    textbase_x += face[0]->glyph->metrics.horiAdvance / 64;
    
  }
  
  return 0;
  
}

int main(int argc, char *argv[]) {

  long int xres, yres;
  long int num_pixels;
  size_t img_sz;
  
  image_t img;

  ssize_t bytes_written;

  int out_fd;

  long int frameno;

  long int num_frames;

  double duration;

  double fps;

  actionbar ab;

  pixel_t white = { 65535, 65535, 65535 };
  
  pixel_t red = { 65535, 0, 0 };

  double v;

  FT_Library ft_lib;
  FT_Face face;
  
  int retval;

  duration = argc>2 ? strtod(argv[2],NULL) : 5.0;

  fps = argc>3 ? strtod(argv[3],NULL) : 60;

  retval = FT_Init_FreeType(&ft_lib);
  if (retval) {
    printf("Error initializing FreeType library.\n");
    return -1;
  }

  retval = FT_New_Face( ft_lib, "/usr/share/fonts/truetype/lyx/cmr10.ttf", 0, &face);
  if (retval == FT_Err_Unknown_File_Format ) {
    printf("Font could not be opened.\n");
    return -1;
  }
    
  else if (retval) {
    printf("Error loading freetype font. %d\n", retval);
    return -1;
  }

  retval = FT_Set_Char_Size(
          face,    /* handle to face object           */
          0,       /* char_width in 1/64th of points  */
          72*64,   /* char_height in 1/64th of points */
          75,     /* horizontal device resolution    */
          75);   /* vertical device resolution      */
  
  xres = 1920;
  yres = 1080;

  num_pixels = xres * yres;
  img_sz = num_pixels * sizeof(pixel_t);
  
  img = (image_t) { .rgb = malloc(img_sz), .xres = xres, .yres = yres };
  if (img.rgb == NULL) {
    perror("malloc");
    return -1;
  }

  out_fd = 1;

  num_frames = duration * fps;

  ab.state = 0;
  ab.xw = img.xres / 3;
  ab.yh = img.yres / 10;

  ab.box_bgcolor = red;
  ab.box_alpha = 0.75;

  ab.fill_color = white;
  ab.text_str = "Video Test";

  ab.base_start.base_xpos = (img.xres / 3);
  ab.base_start.base_ypos = (12 * img.yres / 10);
  ab.base_end.base_xpos = (img.xres / 3);
  ab.base_end.base_ypos = (8 * img.yres / 10);
  
  for (frameno = 0; frameno < num_frames / 3; frameno++) {

    v = frameno; v /= num_frames / 3 - 1;
    
    memset(img.rgb, 0, img_sz);
    
    draw_actionbar(&img, &face, &ab, v);

    {

      bytes_written = writefile(out_fd, img.rgb, img_sz);
      if (bytes_written != img_sz) {
	perror("write");
	return -1;
      }

    }

  }

  ab.base_start.base_xpos = (img.xres / 3);
  ab.base_start.base_ypos = (8 * img.yres / 10);
  ab.base_end.base_xpos = (img.xres / 3);
  ab.base_end.base_ypos = (8 * img.yres / 10);

  for (frameno = num_frames / 3; frameno < 2 * num_frames / 3; frameno++) {

    v = frameno - num_frames / 3; v /= num_frames / 3 - 1;
    
    memset(img.rgb, 0, img_sz);
    
    draw_actionbar(&img, &face, &ab, v);

    {

      bytes_written = writefile(out_fd, img.rgb, img_sz);
      if (bytes_written != img_sz) {
	perror("write");
	return -1;
      }

    }

  }
  
  ab.base_start.base_xpos = (img.xres / 3);
  ab.base_start.base_ypos = (8 * img.yres / 10);
  ab.base_end.base_xpos = (img.xres / 3);
  ab.base_end.base_ypos = (12 * img.yres / 10);
  
  for (frameno = 2 * num_frames / 3; frameno < num_frames; frameno++) {

    v = frameno - 2 * num_frames / 3; v /= num_frames / 3 - 1;
    
    memset(img.rgb, 0, img_sz);
    
    draw_actionbar(&img, &face, &ab, v);

    {

      bytes_written = writefile(out_fd, img.rgb, img_sz);
      if (bytes_written != img_sz) {
	perror("write");
	return -1;
      }

    }

  }
  
  free(img.rgb);
  
  return 0;

}
