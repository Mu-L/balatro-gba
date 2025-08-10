#include "affine_background.h"
#include "affine_background_gfx.h"
#include "affine_main_menu_background_gfx.h"

#include "graphic_utils.h"

BG_AFFINE bgaff;
AFF_SRC_EX asx = {0};

enum AffineBackgroundID background = AFFINE_BG_MAIN_MENU;
static uint timer = 0;

#define TOP_SCANLINE_OFFSET 228
#define ANIMATION_SPEED_DIVISOR 16


void affine_background_init()
{   
    affine_background_change_background(background);
    affine_background_update();

    bgaff = bg_aff_default;
}

void affine_background_hblank()
{
    vu16 vcount = REG_VCOUNT;

    if ((vcount > 160 && vcount < 225) || vcount > TOP_SCANLINE_OFFSET) // Exit the function if the scanline is outside the valid range
    {
        return;
    }
    else if (vcount > TOP_SCANLINE_OFFSET - 3) // This fixes a visual issue where the top 3 pixels of the scanline are out of sync
    {
        vcount -= TOP_SCANLINE_OFFSET;
    }

    const s32 timer_s32 = timer << 8;
    const s32 vcount_s32 = vcount << 8;
    const s16 vcount_s16 = vcount;
    const s32 vcount_sine = lu_sin(vcount_s32 + timer_s32 / ANIMATION_SPEED_DIVISOR); // dividing the timer by 16 to make the animation slower

    switch (background)
    {
    case AFFINE_BG_MAIN_MENU:
        //const s16 offset = 128;
        asx.scr_x = (SCREEN_WIDTH / 2); // 128 on x and y is an offset used to center the rotation
        asx.scr_y = vcount_s16 - (SCREEN_HEIGHT / 2); // scr_y must equal vcount otherwise the background will have no vertical difference
        // asx.tex_x = (vcount_sine);
        // asx.tex_y = (vcount_sine / 64);
        // Shift offset to s32
        asx.tex_x = (1000 * 1000) + (vcount_sine);
        asx.tex_y = (1000 * 1000);
        asx.sx = 128;
        asx.sy = 128;
        asx.alpha = vcount_sine + (timer_s32 / ANIMATION_SPEED_DIVISOR);
        break;
    case AFFINE_BG_GAME:
        break;
    }

    bg_rotscale_ex(&bgaff, &asx);
    REG_BG_AFFINE[2] = bgaff;
}

void affine_background_update()
{
    timer++;

    switch (background)
    {
    case AFFINE_BG_MAIN_MENU:
        // asx.scr_x = SCREEN_WIDTH / 2;
        // asx.scr_y = SCREEN_HEIGHT / 2;
        // asx.tex_x = 16560;
        // asx.tex_y = 14400;
        // asx.sx = 128;
        // asx.sy = 128;
        // asx.alpha = 1000; // This slightly rotates it which increases the perceived resolution of the background
        break;
    case AFFINE_BG_GAME:
        asx.scr_x = 0;
        asx.scr_y = 0;
        asx.tex_x += 5;
        asx.tex_y += 12;
        asx.sx = (lu_sin(timer * 100)) >> 8; // Scale the sine value to fit in a s16
        asx.sx += 256; // Add 256 to the sine value to make it positive
        asx.sy = (lu_sin(timer * 100 + 0x4000)) >> 8; // Scale the sine value to fit in a s16
        asx.sy += 256; // Add 256 to the sine value to make it positive
        asx.alpha = 0;
        break;
    }

    bg_rotscale_ex(&bgaff, &asx);
    REG_BG_AFFINE[2] = bgaff;
}

void affine_background_set_color(COLOR color)
{
    memcpy16(&pal_bg_mem[AFFINE_BG_PB], affine_main_menu_background_gfxPal, AFFINE_BG_PAL_LEN);
    for (int i = 0; i < AFFINE_BG_PAL_LEN; i++)
    {
        clr_rgbscale(&pal_bg_mem[AFFINE_BG_PB] + i, &pal_bg_mem[AFFINE_BG_PB] + i, 1, color);
    }
}

void affine_background_load_palette(const u16 *src)
{
    memcpy16(&pal_bg_mem[AFFINE_BG_PB], src, AFFINE_BG_PAL_LEN);
}

void affine_background_change_background(enum AffineBackgroundID new_bg)
{
    background = new_bg;

    switch (background)
    {
    case AFFINE_BG_MAIN_MENU:
        REG_BG2CNT &= ~BG_AFF_32x32;
        REG_BG2CNT |= BG_AFF_16x16;
        REG_IE |= IRQ_HBLANK;

        memcpy32_tile8_with_palette_offset((u32*)&tile8_mem[AFFINE_BG_CBB], (const u32*)affine_main_menu_background_gfxTiles, affine_main_menu_background_gfxTilesLen/4, AFFINE_BG_PB);
        GRIT_CPY(&se_mem[AFFINE_BG_SBB], affine_main_menu_background_gfxMap);
        affine_background_load_palette(affine_main_menu_background_gfxPal);
        break;
    case AFFINE_BG_GAME:
        REG_BG2CNT &= ~BG_AFF_16x16;
        REG_BG2CNT |= BG_AFF_32x32;
        REG_IE &= ~IRQ_HBLANK;

        memcpy32_tile8_with_palette_offset((u32*)&tile8_mem[AFFINE_BG_CBB], (const u32*)affine_background_gfxTiles, affine_background_gfxTilesLen/4, AFFINE_BG_PB);
        GRIT_CPY(&se_mem[AFFINE_BG_SBB], affine_background_gfxMap);
        affine_background_load_palette(affine_background_gfxPal);
        break;
    }
}