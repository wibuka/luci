#include "luci.h"
#include <freetype/ft2build.h>
#include FT_FREETYPE_H

LUCI_Font load_font(const char* path, unsigned int size) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "ERROR::FREETYPE: Could not init FreeType Library\n");
        return (LUCI_Font){0};
    }

    FT_Face face;
    if (FT_New_Face(ft, path, 0, &face)) {
        fprintf(stderr, "ERROR::FREETYPE: Failed to load font: %s\n", path);
        FT_Done_FreeType(ft);
        return (LUCI_Font){0};
    }

    FT_Set_Pixel_Sizes(face, 0, size);

    int ascent = (int)(face->size->metrics.ascender >> 6);
    int line_height = (int)(face->size->metrics.height >> 6);

    size_t num_glyphs = 95;
    LUCI_Glyph* glyphs = malloc(sizeof(LUCI_Glyph) * num_glyphs);
    if (!glyphs) {
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return (LUCI_Font){0};
    }

    int atlas_width = 512;
    int pen_x = 0;
    int pen_y = 0;
    int row_height = 0;

    for (size_t i = 0; i < num_glyphs; i++) {
        uint32_t code = 32 + (uint32_t)i;
        if (FT_Load_Char(face, code, FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT)) {
            continue;
        }

        int g_w = (int)face->glyph->bitmap.width;
        int g_h = (int)face->glyph->bitmap.rows;

        if (pen_x + g_w + 1 > atlas_width) {
            pen_x = 0;
            pen_y += row_height + 1;
            row_height = 0;
        }

        if (g_h > row_height) {
            row_height = g_h;
        }
        pen_x += g_w + 1;

    }

    int atlas_height = pen_y + row_height + 1;
    int h = 512;
    while (h < atlas_height) {
        h *= 2;
    }
    atlas_height = h;

    unsigned char* atlas_data = calloc(atlas_width * atlas_height, 1);
    if (!atlas_data) {
        free(glyphs);
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return (LUCI_Font){0};
    }

    pen_x = 0;
    pen_y = 0;
    row_height = 0;

    for (size_t i = 0; i < num_glyphs; i++) {
        uint32_t code = 32 + (uint32_t)i;
        if (FT_Load_Char(face, code, FT_LOAD_RENDER)) {
            glyphs[i] = (LUCI_Glyph){ .code = code };
            continue;
        }

        FT_Bitmap* bitmap = &face->glyph->bitmap;
        int g_w = (int)bitmap->width;
        int g_h = (int)bitmap->rows;

        if (pen_x + g_w + 1 > atlas_width) {
            pen_x = 0;
            pen_y += row_height + 1;
            row_height = 0;
        }

        if (g_h > row_height) {
            row_height = g_h;
        }

        for (int y = 0; y < g_h; y++) {
            for (int x = 0; x < g_w; x++) {
                int ax = pen_x + x;
                int ay = pen_y + y;
                atlas_data[ay * atlas_width + ax] = bitmap->buffer[y * bitmap->pitch + x];
            }
        }

        glyphs[i] = (LUCI_Glyph){
            .code = code,
            .x = pen_x,
            .y = pen_y,
            .w = g_w,
            .h = g_h,
            .bearing_x = face->glyph->bitmap_left,
            .bearing_y = face->glyph->bitmap_top,
            .advance = (float)face->glyph->advance.x / 64.0f
        };

        pen_x += g_w + 1;
    }

    for (size_t i = 0; i < num_glyphs; i++) {
        if (glyphs[i].h > 0) {
            glyphs[i].y = atlas_height - glyphs[i].y - glyphs[i].h;
        }
    }

    unsigned char* rgba_data = calloc(atlas_width * atlas_height * 4, 1);
    if (!rgba_data) {
        free(atlas_data);
        free(glyphs);
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return (LUCI_Font){0};
    }

    for (int y = 0; y < atlas_height; y++) {
        int src_row = (atlas_height - 1 - y) * atlas_width;
        int dst_row = y * atlas_width * 4;
        for (int x = 0; x < atlas_width; x++) {
            rgba_data[dst_row + x * 4 + 0] = 255;
            rgba_data[dst_row + x * 4 + 1] = 255;
            rgba_data[dst_row + x * 4 + 2] = 255;
            rgba_data[dst_row + x * 4 + 3] = atlas_data[src_row + x];
        }
    }

    LUCI_Texture texture = LUCI_create_texture(rgba_data, atlas_width, atlas_height, 4);

    free(rgba_data);
    free(atlas_data);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    return (LUCI_Font){
        .texture = texture,
        .glyphs = glyphs,
        .length = num_glyphs,
        .ascent = ascent,
        .line_height = line_height,
        .font_size = size
    };
}

