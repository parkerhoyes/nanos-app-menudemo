/*
 * License for the Ledger Nano S Menu Demo project, originally found here:
 * https://github.com/parkerhoyes/nanos-app-menudemo
 *
 * Copyright (C) 2016 Parker Hoyes <contact@parkerhoyes.com>
 *
 * This software is provided "as-is", without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim
 *    that you wrote the original software. If you use this software in a
 *    product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "app.h"

#include <stdbool.h>
#include <string.h>

#include "os.h"
#include "os_io_seproxyhal.h"

#include "bui.h"
#include "bui_bkb.h"
#include "bui_font.h"
#include "bui_menu.h"

typedef enum {
	APP_MODE_MENU,
	APP_MODE_TYPE,
} app_mode_e;

static app_mode_e app_mode;
static bui_bitmap_128x32_t app_disp_buffer;
static int8_t app_disp_progress;
static bool app_disp_invalidated; // true if the display needs to be redrawn
static bui_menu_menu_t app_menu;
static bui_bkb_bkb_t app_bkb;
static char app_type_buff[21];

static void app_menu_elem_draw(const bui_menu_menu_t *menu, uint8_t i, bui_bitmap_128x32_t *buffer, int y) {
	switch (i) {
	case 0:
		bui_font_draw_string(buffer, "BUI Menu Demo", 64, y + 16, BUI_DIR_CENTER, BUI_FONT_OPEN_SANS_EXTRABOLD_11);
		break;
	case 1:
		bui_font_draw_string(buffer, "Vires", 64, y + 6, BUI_DIR_CENTER, BUI_FONT_LUCIDA_CONSOLE_8);
		break;
	case 2:
		bui_font_draw_string(buffer, "in", 64, y + 6, BUI_DIR_CENTER, BUI_FONT_LUCIDA_CONSOLE_8);
		break;
	case 3:
		bui_font_draw_string(buffer, "numeris", 64, y + 6, BUI_DIR_CENTER, BUI_FONT_LUCIDA_CONSOLE_8);
		break;
	case 4:
		bui_font_draw_string(buffer, "Press both buttons", 64, y, BUI_DIR_TOP, BUI_FONT_LUCIDA_CONSOLE_8);
		bui_font_draw_string(buffer, "to type here:", 64, y + 12, BUI_DIR_TOP, BUI_FONT_LUCIDA_CONSOLE_8);
		bui_font_draw_string(buffer, app_type_buff, 64, y + 24, BUI_DIR_TOP, BUI_FONT_LUCIDA_CONSOLE_8);
		break;
	case 5:
		bui_font_draw_string(buffer, "\"[Bitcoin]", 64, y, BUI_DIR_TOP, BUI_FONT_LUCIDA_CONSOLE_8);
		bui_font_draw_string(buffer, "is a techno", 64, y + 12, BUI_DIR_TOP, BUI_FONT_LUCIDA_CONSOLE_8);
		bui_font_draw_string(buffer, "tour de force.\"", 64, y + 24, BUI_DIR_TOP, BUI_FONT_LUCIDA_CONSOLE_8);
		break;
	case 6:
		bui_font_draw_string(buffer, "- Bill Gates", 64, y + 16, BUI_DIR_CENTER, BUI_FONT_OPEN_SANS_SEMIBOLD_18);
		break;
	case 7:
		bui_draw_bitmap(buffer, bui_bitmap_badge_dashboard_bitmap, bui_bitmap_badge_dashboard_w, 0, 0, 29, y + 9,
				bui_bitmap_badge_dashboard_w, bui_bitmap_badge_dashboard_h);
		bui_font_draw_string(buffer, "Quit app", 52, y + 16, BUI_DIR_LEFT, BUI_FONT_OPEN_SANS_EXTRABOLD_11);
		break;
	}
}

static void app_menu_elem_selected(uint8_t i) {
	switch (i) {
	case 4:
		bui_bkb_init(&app_bkb, bui_bkb_layout_standard, sizeof(bui_bkb_layout_standard), app_type_buff,
				strlen(app_type_buff), sizeof(app_type_buff) - 1, true);
		app_mode = APP_MODE_TYPE;
		app_disp_invalidated = true;
		break;
	case 7:
		os_sched_exit(0); // Go back to the dashboard
		break;
	}
}

static void app_bkb_selected() {
	app_type_buff[bui_bkb_get_type_buff_size(&app_bkb)] = '\0';
	app_mode = APP_MODE_MENU;
	app_disp_invalidated = true;
}

void app_init() {
	// Set a ticker interval of 40 ms
	G_io_seproxyhal_spi_buffer[0] = SEPROXYHAL_TAG_SET_TICKER_INTERVAL;
	G_io_seproxyhal_spi_buffer[1] = 0;
	G_io_seproxyhal_spi_buffer[2] = 2;
	G_io_seproxyhal_spi_buffer[3] = 0;
	G_io_seproxyhal_spi_buffer[4] = 40;
	io_seproxyhal_spi_send(G_io_seproxyhal_spi_buffer, 5);

	app_mode = APP_MODE_MENU;
	app_disp_progress = -1;
	app_disp_invalidated = true;
	bui_menu_init(&app_menu, (bui_menu_elem_data_t) {0b10001111000000000000000000000000, 8}, 0,
			&app_menu_elem_draw, true);
	app_type_buff[0] = '\0';
}

void app_draw() {
	if (app_mode == APP_MODE_MENU)
		bui_menu_draw(&app_menu, &app_disp_buffer);
	else
		bui_bkb_draw(&app_bkb, &app_disp_buffer);
}

void app_display() {
	bui_fill(&app_disp_buffer, false);
	app_draw();
	if (app_disp_progress == -1)
		app_disp_progress = bui_display(&app_disp_buffer, 0);
	else
		app_disp_progress = 0;
}

void app_event_button_push(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		if (app_mode == APP_MODE_MENU)
			app_menu_elem_selected(bui_menu_get_focused(&app_menu));
		else
			app_bkb_selected();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		if (app_mode == APP_MODE_MENU) {
			if (bui_menu_scroll(&app_menu, true))
				app_disp_invalidated = true;
		} else {
			bui_bkb_choose(&app_bkb, BUI_DIR_LEFT);
			app_disp_invalidated = true;
		}
		break;
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		if (app_mode == APP_MODE_MENU) {
			if (bui_menu_scroll(&app_menu, false))
				app_disp_invalidated = true;
		} else {
			bui_bkb_choose(&app_bkb, BUI_DIR_RIGHT);
			app_disp_invalidated = true;
		}
		break;
	}
}

void app_event_ticker() {
	if (app_mode == APP_MODE_MENU) {
		if (bui_menu_animate(&app_menu, 40))
			app_disp_invalidated = true;
	} else {
		if (bui_bkb_tick(&app_bkb, 4))
			app_disp_invalidated = true;
	}
	if (app_disp_invalidated && app_disp_progress == -1) {
		app_display();
		app_disp_invalidated = false;
	}
}

void app_event_display_processed() {
	if (app_disp_progress != -1)
		app_disp_progress = bui_display(&app_disp_buffer, app_disp_progress);
}
