/*
 * Copyright (C) 2009 Alexander Kerner <lunohod@openinkpot.org>
 * Copyright © 2009 Mikhail Gusarov <dottedmag@dottedmag.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <time.h>

#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Con.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Edje.h>

#include <libkeys.h>

#ifndef DATADIR
#define DATADIR "."
#endif

#define USB_PLUGGED "USB-plugged"
#define USB_UNPLUGGGED "USB-unplugged"

static Ecore_Evas *main_win;

void x_shutdown(void* param) { 
	ecore_main_loop_quit();
}

void main_win_close_handler(Ecore_Evas *mw)
{
	ecore_evas_hide(mw);
}

static void die(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

static void
key_handler(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    const char* action = keys_lookup_by_event((keys_t*)data, "default",
                                              (Evas_Event_Key_Up*)event_info);

    if(action && !strcmp(action, "Close"))
        do_nothing();
    else if(action && !strcmp(action, "MassStorage"))
        load_massstorage();
    else if(action && !strcmp(action, "Usbnet"))
        load_usbnet();
}

typedef struct
{
	char* msg;
	int size;
} client_data_t;

static int _client_add(void* param, int ev_type, void* ev)
{
	Ecore_Con_Event_Client_Add* e = ev;
	client_data_t* msg = malloc(sizeof(client_data_t));
	msg->msg = strdup("");
	msg->size = 0;
	ecore_con_client_data_set(e->client, msg);
	return 0;
}

static int _client_del(void* param, int ev_type, void* ev)
{
	Ecore_Con_Event_Client_Del* e = ev;
	client_data_t* msg = ecore_con_client_data_get(e->client);

	/* Handle */
	if(strlen(USB_PLUGGED) == msg->size && !strncmp(USB_PLUGGED, msg->msg, msg->size))
		usb_pluggged();
	else if(strlen(USB_UNPLUGGGED) == msg->size && !strncmp(USB_UNPLUGGGED, msg->msg, msg->size))
		usb_unplugged();

	free(msg->msg);
	free(msg);
	return 0;
}

static int _client_data(void* param, int ev_type, void* ev)
{
	Ecore_Con_Event_Client_Data* e = ev;
	client_data_t* msg = ecore_con_client_data_get(e->client);
	msg->msg = realloc(msg->msg, msg->size + e->size);
	memcpy(msg->msg + msg->size, e->data, e->size);
	msg->size += e->size;
	return 0;
}

void usb_pluggged()
{
	ecore_evas_hide(main_win);
	ecore_evas_show(main_win);
}

void usb_unplugged()
{
	ecore_evas_hide(main_win);
}

void load_massstorage()
{
	ecore_evas_hide(main_win);
	system("usb-mass-storage");
}

void load_usbnet()
{
	ecore_evas_hide(main_win);
	system("usb-usbnet");
}

void do_nothing()
{
	ecore_evas_hide(main_win);
}

static void main_win_resize_handler(Ecore_Evas* main_win)
{
	ecore_evas_hide(main_win);
	Evas* canvas = ecore_evas_get(main_win);
	int w, h;
	evas_output_size_get(canvas, &w, &h);

	Evas_Object* edje = evas_object_name_find(canvas, "edje");
	evas_object_resize(edje, w, h);
	ecore_evas_show(main_win);
}

int main(int argc, char **argv)
{
	if(!evas_init())
		die("Unable to initialize Evas\n");
	if(!ecore_init())
		die("Unable to initialize Ecore\n");
	if(!ecore_con_init())
		die("Unable to initialize Ecore_Con\n");
	if(!ecore_evas_init())
		die("Unable to initialize Ecore_Evas\n");
	if(!edje_init())
		die("Unable to initialize Edje\n");

	setlocale(LC_ALL, "");
	textdomain("usbwatcher");

    keys_t* keys = keys_alloc("usbwatcher");

	ecore_x_io_error_handler_set(x_shutdown, NULL);

	ecore_con_server_add(ECORE_CON_LOCAL_USER, "usbwatcher", 0, NULL);

	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, _client_add, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, _client_data, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, _client_del, NULL);

	main_win = ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);
	ecore_evas_borderless_set(main_win, 0);
	ecore_evas_shaped_set(main_win, 0);
	ecore_evas_title_set(main_win, "Usbwatcher");
	Evas *main_canvas = ecore_evas_get(main_win);

	Evas_Object *main_edje = edje_object_add(main_canvas);
	evas_object_name_set(main_edje, "edje");
	edje_object_file_set(main_edje, DATADIR "/usbwatcher/usbwatcher.edj", "usbwatcher");
	evas_object_move(main_edje, 0, 0);
	evas_object_resize(main_edje, 600, 800);
	evas_object_show(main_edje);

	evas_object_focus_set(main_edje, 1);
	evas_object_event_callback_add(main_edje, EVAS_CALLBACK_KEY_UP, &key_handler, keys);

	ecore_evas_callback_resize_set(main_win, main_win_resize_handler);

	edje_object_part_text_set(main_edje, "usbwatcher/title", gettext("USB Connection"));
	char *t;
	asprintf(&t, "%s<br><br>%s",
			gettext("USB Mass Storage - press \"1\""),
			gettext("Battery Charging - press \"2\""));
	edje_object_part_text_set(main_edje, "usbwatcher/text", t);
	free(t);

	ecore_main_loop_begin();

	ecore_evas_free(main_win);

	edje_shutdown();
	ecore_evas_shutdown();
	ecore_con_shutdown();
	ecore_shutdown();
	evas_shutdown();

    keys_free(keys);

	return 0;
}
