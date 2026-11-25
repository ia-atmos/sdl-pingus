// The C of the game, over the environment `ia-atmos/env-sdl`.
//
// It is included by `engine/sdl.atm`, in a prelude, after the one of
// the environment, and reaches SDL through the door in C of the
// environment (`sdl_ren`, `sdl_sfc`, `sdl_tex`, `sdl_new_image`). It
// holds what is of the game, and what works per pixel:
//
//   - images: loading, crops and modifiers (rotations and flips), as
//     RGBA32 surfaces, given to the environment as its images;
//   - bitmap fonts, from the `.font` descriptions;
//   - the ground: the collision map (a ground type per pixel) and the
//     ground map (its image), with put and remove through masks;
//   - the small map.
//
// The window, the loop of frames, the events, the shapes, the sounds,
// and the test mode are of the environment.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

static int PG_W = 800;      // the size the game draws in
static int PG_H = 600;

static int pg_int (AVal v) {
    return a_isnum(v) ? (int) a_dbl(v) : 0;
}

static const char* pg_cstr (AVal v) {
    return (v.type == A_STR) ? A_STR_OF(v)->s : NULL;
}

///////////////////////////////////////////////////////////////////////////////
// IMAGES
///////////////////////////////////////////////////////////////////////////////

// The modifiers of the data files, as numbers:
//   0 ROT0  1 ROT90  2 ROT180  3 ROT270
//   4 ROT0FLIP  5 ROT90FLIP  6 ROT180FLIP  7 ROT270FLIP
static SDL_Surface* pg_modify (SDL_Surface* src, int mod) {
    int w = src->w, h = src->h;
    int swap = (mod==1 || mod==3 || mod==5 || mod==7);
    int W = swap ? h : w;
    int H = swap ? w : h;
    SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, SDL_PIXELFORMAT_RGBA32);
    uint32_t* s = src->pixels;
    uint32_t* d = dst->pixels;
    int sp = src->pitch / 4;
    int dp = dst->pitch / 4;
    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            int col, row;
            switch (mod) {
                case 1:  col = h-y-1; row = x;       break;
                case 2:  col = w-x-1; row = h-y-1;   break;
                case 3:  col = y;     row = w-x-1;   break;
                case 4:  col = w-x-1; row = y;       break;
                case 5:  col = y;     row = x;       break;
                case 6:  col = x;     row = h-y-1;   break;
                case 7:  col = h-y-1; row = w-x-1;   break;
                default: col = x;     row = y;       break;
            }
            d[row*dp + col] = s[y*sp + x];
        }
    }
    return dst;
}

// Loads an image, crops it to `cw x ch` at `cx,cy` (if `cw>0`), and
// applies a modifier: an image of the environment, with an RGBA32
// surface (the masks of the ground read its alpha), or nil. The cache
// by name is in Atmos (`pg.img`).
static AVal pg_img_load (const char* path, int mod, int cx, int cy, int cw, int ch) {
    if (path == NULL) {
        return a_nil();
    }
    SDL_Surface* raw = IMG_Load(path);
    if (raw == NULL) {
        return a_nil();
    }
    SDL_Surface* sfc = SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(raw);
    if (sfc == NULL) {
        return a_nil();
    }
    if (cw > 0 && ch > 0) {
        SDL_Surface* sub = SDL_CreateRGBSurfaceWithFormat(0, cw, ch, 32, SDL_PIXELFORMAT_RGBA32);
        SDL_Rect r = { cx, cy, cw, ch };
        SDL_SetSurfaceBlendMode(sfc, SDL_BLENDMODE_NONE);
        SDL_BlitSurface(sfc, &r, sub, NULL);
        SDL_FreeSurface(sfc);
        sfc = sub;
    }
    if (mod != 0) {
        SDL_Surface* m = pg_modify(sfc, mod);
        SDL_FreeSurface(sfc);
        sfc = m;
    }
    SDL_SetSurfaceBlendMode(sfc, SDL_BLENDMODE_BLEND);
    return sdl_new_image(sfc);
}

// The surface of an image, with its size, for the masks of the ground.
typedef struct {
    SDL_Surface*  sfc;
    int           w, h;
} PgImg;

static PgImg* pg_img (AVal v, PgImg* out) {
    SDL_Surface* sfc = sdl_sfc(v);
    if (sfc == NULL) {
        return NULL;
    }
    out->sfc = sfc;
    out->w   = sfc->w;
    out->h   = sfc->h;
    return out;
}

// Draws the part `sx,sy,sw,sh` of an image at `dx,dy`, for the fonts.
static void pg_img_part (AVal v, int sx, int sy, int sw, int sh, int dx, int dy, int alpha) {
    SDL_Renderer* ren = sdl_ren();
    if (ren == NULL || dx >= PG_W || dy >= PG_H || dx+sw <= 0 || dy+sh <= 0) {
        return;
    }
    SDL_Texture* tex = sdl_tex(v);
    if (tex == NULL) {
        return;
    }
    SDL_Rect s = { sx, sy, sw, sh };
    SDL_Rect d = { dx, dy, sw, sh };
    if (alpha < 255) {
        SDL_SetTextureAlphaMod(tex, alpha<0 ? 0 : alpha);
    }
    SDL_RenderCopy(ren, tex, &s, &d);
    if (alpha < 255) {
        SDL_SetTextureAlphaMod(tex, 255);
    }
}

///////////////////////////////////////////////////////////////////////////////
// FONTS
///////////////////////////////////////////////////////////////////////////////

typedef struct {
    int16_t x, y, w, h;     // in the image
    int16_t ox, oy;         // offset of the drawing
    int16_t adv;            // advance
    int16_t img;            // the image in the font, or -1 if absent
} PgGlyph;

#define PG_FONT_IMGS 8

typedef struct {
    PgGlyph* glyphs;        // by unicode, 65536
    int      size;
    float    char_spacing;
    float    vertical_spacing;
    AVal     imgs[PG_FONT_IMGS];    // kept alive by the program
    int      imgs_n;
} PgFont;

#define PG_FONTS_MAX 16
static PgFont PG_FONTS[PG_FONTS_MAX];
static int    PG_FONTS_N = 0;

static int pg_font_new (int size, double cs, double vs) {
    if (PG_FONTS_N == PG_FONTS_MAX) {
        return -1;
    }
    PgFont* f = &PG_FONTS[PG_FONTS_N];
    f->glyphs = malloc(65536 * sizeof(PgGlyph));
    for (int i=0; i<65536; i++) {
        f->glyphs[i].img = -1;
    }
    f->size = size;
    f->char_spacing = cs;
    f->vertical_spacing = size * vs;
    f->imgs_n = 0;
    return PG_FONTS_N++;
}

static void pg_font_glyph (int fi, int u, AVal img, int x1, int y1, int x2, int y2,
                           int ox, int oy, int adv) {
    if (fi<0 || fi>=PG_FONTS_N || u<0 || u>=65536 || sdl_sfc(img)==NULL) {
        return;
    }
    PgFont* f = &PG_FONTS[fi];
    int k = 0;
    while (k < f->imgs_n && sdl_sfc(f->imgs[k]) != sdl_sfc(img)) {
        k++;
    }
    if (k == f->imgs_n) {
        if (k == PG_FONT_IMGS) {
            return;
        }
        f->imgs[f->imgs_n++] = img;
    }
    PgGlyph g = { x1, y1, x2-x1, y2-y1, ox, oy, adv, k };
    PG_FONTS[fi].glyphs[u] = g;
}

// The next unicode of an UTF-8 string, or -1 at the end.
static int pg_utf8 (const char** s) {
    const unsigned char* p = (const unsigned char*) *s;
    if (*p == 0) {
        return -1;
    }
    int c = *p++;
    int n = 0;
    if      (c >= 0xF0) { c &= 0x07; n = 3; }
    else if (c >= 0xE0) { c &= 0x0F; n = 2; }
    else if (c >= 0xC0) { c &= 0x1F; n = 1; }
    while (n-- > 0 && (*p & 0xC0) == 0x80) {
        c = (c << 6) | (*p++ & 0x3F);
    }
    *s = (const char*) p;
    return c;
}

// The width of the first line of `s`.
static int pg_font_line_width (int fi, const char* s) {
    PgFont* f = &PG_FONTS[fi];
    float w = 0;
    int u;
    while ((u = pg_utf8(&s)) > 0 && u != '\n') {
        if (u < 65536 && f->glyphs[u].img >= 0) {
            w += f->glyphs[u].adv + f->char_spacing;
        }
    }
    return (int) w;
}

static int pg_font_width (int fi, const char* s) {
    if (fi<0 || fi>=PG_FONTS_N || s==NULL) {
        return 0;
    }
    int w = 0;
    while (1) {
        int l = pg_font_line_width(fi, s);
        if (l > w) w = l;
        const char* nl = strchr(s, '\n');
        if (nl == NULL) break;
        s = nl + 1;
    }
    return w;
}

// Draws a text. The origin is 0 (left), 1 (center), or 2 (right) of
// the top of the text, as in the original.
static void pg_font_draw (int fi, const char* s, int x, int y, int origin, int alpha) {
    if (fi<0 || fi>=PG_FONTS_N || s==NULL || sdl_ren()==NULL) {
        return;
    }
    PgFont* f = &PG_FONTS[fi];
    float fy = y + f->size;
    while (1) {
        int lw = pg_font_line_width(fi, s);
        float dx = x - (origin==1 ? lw/2 : (origin==2 ? lw : 0));
        int u;
        while ((u = pg_utf8(&s)) > 0 && u != '\n') {
            if (u < 65536 && f->glyphs[u].img >= 0) {
                PgGlyph* g = &f->glyphs[u];
                if (g->w > 0 && g->h > 0)       // a space has no image
                pg_img_part(f->imgs[g->img], g->x, g->y, g->w, g->h,
                            (int)dx + g->ox, (int)fy + g->oy, alpha);
                dx += g->adv + f->char_spacing;
            }
        }
        if (u != '\n') {
            break;
        }
        fy += f->vertical_spacing;
    }
}

///////////////////////////////////////////////////////////////////////////////
// GROUND: the collision map and the ground map of the level
///////////////////////////////////////////////////////////////////////////////

enum {
    GP_NOTHING, GP_SOLID, GP_TRANSPARENT, GP_GROUND, GP_BRIDGE,
    GP_WATER, GP_LAVA, GP_REMOVE, GP_OUTOFSCREEN
};

#define PG_TILE 128

typedef struct {
    int           w, h;
    uint8_t*      col;      // ground type per pixel
    SDL_Surface*  gfx;      // the image of the ground, RGBA32
    int           tw, th;   // number of tiles
    SDL_Texture** tiles;
    uint8_t*      dirty;
    unsigned      serial;   // changes of the collision map
    SDL_Texture*  small;    // the small map
    int           small_w, small_h;
    unsigned      small_serial;
} PgMap;

static PgMap PG_MAP;

static void pg_map_free (void) {
    PgMap* m = &PG_MAP;
    if (m->col == NULL) {
        return;
    }
    free(m->col);
    SDL_FreeSurface(m->gfx);
    for (int i=0; i<m->tw*m->th; i++) {
        if (m->tiles[i] != NULL) {
            SDL_DestroyTexture(m->tiles[i]);
        }
    }
    free(m->tiles);
    free(m->dirty);
    if (m->small != NULL) {
        SDL_DestroyTexture(m->small);
    }
    memset(m, 0, sizeof(PgMap));
}

static void pg_map_new (int w, int h) {
    pg_map_free();
    PgMap* m = &PG_MAP;
    m->w = w;
    m->h = h;
    m->col = calloc(w*h, 1);
    m->gfx = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(m->gfx, NULL, 0);
    m->tw = (w + PG_TILE - 1) / PG_TILE;
    m->th = (h + PG_TILE - 1) / PG_TILE;
    m->tiles = calloc(m->tw*m->th, sizeof(SDL_Texture*));
    m->dirty = calloc(m->tw*m->th, 1);
}

static int pg_map_get (int x, int y) {
    PgMap* m = &PG_MAP;
    if (x>=0 && x<m->w && y>=0 && y<m->h) {
        return m->col[y*m->w + x];
    }
    return GP_OUTOFSCREEN;
}

static void pg_map_dirty (int x, int y, int w, int h) {
    PgMap* m = &PG_MAP;
    int x1 = x < 0 ? 0 : x / PG_TILE;
    int y1 = y < 0 ? 0 : y / PG_TILE;
    int x2 = (x + w) / PG_TILE;
    int y2 = (y + h) / PG_TILE;
    for (int ty=y1; ty<=y2 && ty<m->th; ty++) {
        for (int tx=x1; tx<=x2 && tx<m->tw; tx++) {
            m->dirty[ty*m->tw + tx] = 1;
        }
    }
}

// Puts an image in the ground map, and its opaque pixels in the
// collision map with the given type.
static void pg_map_put (AVal i, int x0, int y0, int type) {
    PgMap* m = &PG_MAP;
    PgImg img_;
    PgImg* img = pg_img(i, &img_);
    if (img==NULL || m->col==NULL) {
        return;
    }
    SDL_Rect r = { x0, y0, img->w, img->h };
    SDL_BlitSurface(img->sfc, NULL, m->gfx, &r);
    pg_map_dirty(x0, y0, img->w, img->h);
    if (type == GP_TRANSPARENT) {
        return;     // only drawn, as in the original
    }
    m->serial++;
    uint8_t* src = img->sfc->pixels;
    int pitch = img->sfc->pitch;
    for (int y=0; y<img->h; y++) {
        int yy = y + y0;
        if (yy<0 || yy>=m->h) continue;
        for (int x=0; x<img->w; x++) {
            int xx = x + x0;
            if (xx<0 || xx>=m->w) continue;
            if (src[y*pitch + 4*x + 3] != 255) continue;
            uint8_t* p = &m->col[yy*m->w + xx];
            if (type!=GP_BRIDGE || *p==GP_NOTHING) {
                *p = type;
            }
        }
    }
}

// Sets the type of the opaque pixels of an image in the collision map
// only, whatever is there (to close or open a door).
static void pg_map_col (AVal i, int x0, int y0, int type) {
    PgMap* m = &PG_MAP;
    PgImg img_;
    PgImg* img = pg_img(i, &img_);
    if (img==NULL || m->col==NULL) {
        return;
    }
    m->serial++;
    uint8_t* src = img->sfc->pixels;
    int pitch = img->sfc->pitch;
    for (int y=0; y<img->h; y++) {
        int yy = y + y0;
        if (yy<0 || yy>=m->h) continue;
        for (int x=0; x<img->w; x++) {
            int xx = x + x0;
            if (xx<0 || xx>=m->w) continue;
            if (src[y*pitch + 4*x + 3] == 255) {
                m->col[yy*m->w + xx] = type;
            }
        }
    }
}

// Removes from the ground map the opaque pixels of `gi`, and from the
// collision map those of `ci`, except where the ground is solid.
static void pg_map_remove (AVal gi, AVal ci, int x0, int y0) {
    PgMap* m = &PG_MAP;
    PgImg g_, c_;
    PgImg* g = pg_img(gi, &g_);
    PgImg* c = pg_img(ci, &c_);
    if (g==NULL || c==NULL || m->col==NULL) {
        return;
    }
    uint8_t* dst = m->gfx->pixels;
    int dpitch = m->gfx->pitch;
    uint8_t* src = g->sfc->pixels;
    int spitch = g->sfc->pitch;
    for (int y=0; y<g->h; y++) {
        int yy = y + y0;
        if (yy<0 || yy>=m->h) continue;
        for (int x=0; x<g->w; x++) {
            int xx = x + x0;
            if (xx<0 || xx>=m->w) continue;
            if (src[y*spitch + 4*x + 3]==255 && m->col[yy*m->w+xx]!=GP_SOLID) {
                dst[yy*dpitch + 4*xx + 3] = 0;
            }
        }
    }
    pg_map_dirty(x0, y0, g->w, g->h);
    m->serial++;
    src = c->sfc->pixels;
    spitch = c->sfc->pitch;
    for (int y=0; y<c->h; y++) {
        int yy = y + y0;
        if (yy<0 || yy>=m->h) continue;
        for (int x=0; x<c->w; x++) {
            int xx = x + x0;
            if (xx<0 || xx>=m->w) continue;
            if (src[y*spitch + 4*x + 3] == 255) {
                uint8_t* p = &m->col[yy*m->w + xx];
                if (*p != GP_SOLID) {
                    *p = GP_NOTHING;
                }
            }
        }
    }
}

// Draws the ground map, with the level at `ox,oy` of the screen.
static void pg_map_draw (int ox, int oy) {
    PgMap* m = &PG_MAP;
    if (m->col==NULL || sdl_ren()==NULL) {
        return;
    }
    for (int ty=0; ty<m->th; ty++) {
        int y = ty*PG_TILE + oy;
        if (y >= PG_H || y+PG_TILE <= 0) continue;
        for (int tx=0; tx<m->tw; tx++) {
            int x = tx*PG_TILE + ox;
            if (x >= PG_W || x+PG_TILE <= 0) continue;
            int k = ty*m->tw + tx;
            int w = (tx==m->tw-1) ? m->w - tx*PG_TILE : PG_TILE;
            int h = (ty==m->th-1) ? m->h - ty*PG_TILE : PG_TILE;
            if (m->tiles[k] == NULL) {
                m->tiles[k] = SDL_CreateTexture(sdl_ren(), SDL_PIXELFORMAT_RGBA32,
                                SDL_TEXTUREACCESS_STREAMING, w, h);
                SDL_SetTextureBlendMode(m->tiles[k], SDL_BLENDMODE_BLEND);
                m->dirty[k] = 1;
            }
            if (m->dirty[k]) {
                uint8_t* p = (uint8_t*)m->gfx->pixels
                           + ty*PG_TILE*m->gfx->pitch + tx*PG_TILE*4;
                SDL_UpdateTexture(m->tiles[k], NULL, p, m->gfx->pitch);
                m->dirty[k] = 0;
            }
            SDL_Rect d = { x, y, w, h };
            SDL_RenderCopy(sdl_ren(), m->tiles[k], NULL, &d);
        }
    }
}

// Draws the small map at `x,y` with size `w x h`. It is rebuilt only
// when the collision map changes.
static void pg_map_small (int x, int y, int w, int h) {
    PgMap* m = &PG_MAP;
    if (m->col==NULL || sdl_ren()==NULL || w<=0 || h<=0) {
        return;
    }
    if (m->small==NULL || m->small_w!=w || m->small_h!=h) {
        if (m->small != NULL) {
            SDL_DestroyTexture(m->small);
        }
        m->small = SDL_CreateTexture(sdl_ren(), SDL_PIXELFORMAT_RGBA32,
                        SDL_TEXTUREACCESS_STREAMING, w, h);
        SDL_SetTextureBlendMode(m->small, SDL_BLENDMODE_BLEND);
        m->small_w = w;
        m->small_h = h;
        m->small_serial = m->serial - 1;
    }
    if (m->small_serial != m->serial) {
        m->small_serial = m->serial;
        uint32_t* px = malloc(w*h*4);
        for (int sy=0; sy<h; sy++) {
            for (int sx=0; sx<w; sx++) {
                int cx = sx * m->w / w;
                int cy = sy * m->h / h;
                uint8_t r=0, g=0, b=0, a=255;
                switch (m->col[cy*m->w + cx]) {
                    case GP_NOTHING:                         a = 200;        break;
                    case GP_SOLID:  r=100; g=100; b=125;                     break;
                    case GP_BRIDGE: r=0;   g=255; b=100;                     break;
                    case GP_WATER:
                    case GP_LAVA:   r=0;   g=0;   b=200;                     break;
                    default:        r=200; g=200; b=200;                     break;
                }
                uint8_t* q = (uint8_t*) &px[sy*w + sx];
                q[0]=r; q[1]=g; q[2]=b; q[3]=a;
            }
        }
        SDL_UpdateTexture(m->small, NULL, px, w*4);
        free(px);
    }
    SDL_Rect d = { x, y, w, h };
    SDL_RenderCopy(sdl_ren(), m->small, NULL, &d);
}
