/*
 *   Copyright (C) 2007-2016 Tristan Heaven <tristan@tristanheaven.net>
 *
 *   This file is part of GtkHash.
 *
 *   GtkHash is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   GtkHash is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with GtkHash. If not, see <https://gnu.org/licenses/gpl-2.0.txt>.
 */

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <gtk/gtk.h>

#include "properties.h"
#include "properties-hash.h"
#include "properties-list.h"
#include "../hash/hash-string.h"
#include "../hash/hash-file.h"
#include "../hash/digest-format.h"

void gtkhash_hash_string_finish_cb(G_GNUC_UNUSED const enum hash_func_e id,
	G_GNUC_UNUSED const char *digest)
{
}

void gtkhash_hash_file_report_cb(void *data, const goffset file_size,
	const goffset total_read, GTimer *timer)
{
	struct page_s *page = data;

	gtk_progress_bar_set_fraction(page->progressbar,
		(double)total_read /
		(double)file_size);

	const double elapsed = g_timer_elapsed(timer, NULL);
	if (elapsed <= 1)
		return;

	// Update progressbar text...

	const unsigned int s = elapsed / total_read * (file_size - total_read);
	char *total_read_str = g_format_size(total_read);
	char *file_size_str = g_format_size(file_size);
	char *speed_str = g_format_size(total_read / elapsed);
	char *text = NULL;

	if (s > 60) {
		const unsigned int m = s / 60;
		if (m == 1)
			text = g_strdup_printf(_("%s of %s - 1 minute left (%s/sec)"),
				total_read_str, file_size_str, speed_str);
		else
			text = g_strdup_printf(_("%s of %s - %u minutes left (%s/sec)"),
				total_read_str, file_size_str, m, speed_str);
	} else {
		if (s == 1)
			text = g_strdup_printf(_("%s of %s - 1 second left (%s/sec)"),
				total_read_str, file_size_str, speed_str);
		else
			text = g_strdup_printf(_("%s of %s - %u seconds left (%s/sec)"),
				total_read_str, file_size_str, s, speed_str);
	}

	gtk_progress_bar_set_text(page->progressbar, text);

	g_free(text);
	g_free(speed_str);
	g_free(file_size_str);
	g_free(total_read_str);
}

void gtkhash_hash_file_digest_cb(const enum hash_func_e id,
	const char *digest, void *data)
{
	gtkhash_properties_list_set_digest(data, id, digest);
}

void gtkhash_hash_file_finish_cb(void *data)
{
	gtkhash_properties_idle(data);
}

void gtkhash_hash_file_stop_cb(void *data)
{
	gtkhash_properties_idle((struct page_s *)data);
}

void gtkhash_properties_hash_start(struct page_s *page,
	const uint8_t *hmac_key, const size_t key_size)
{
	gtkhash_hash_file(page->hfile, page->uri, DIGEST_FORMAT_HEX_LOWER,
		hmac_key, key_size, page);
}

void gtkhash_properties_hash_stop(struct page_s *page)
{
	gtkhash_hash_file_cancel(page->hfile);
}

int gtkhash_properties_hash_funcs_supported(struct page_s *page)
{
	int supported = 0;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (page->funcs[i].supported)
			supported++;
	}

	return supported;
}

void gtkhash_properties_hash_init(struct page_s *page)
{
	gtkhash_hash_func_init_all(page->funcs);

	page->hfile = gtkhash_hash_file_new(page->funcs);
}

void gtkhash_properties_hash_deinit(struct page_s *page)
{
	gtkhash_hash_file_free(page->hfile);
	page->hfile = NULL;

	gtkhash_hash_func_deinit_all(page->funcs);
}
