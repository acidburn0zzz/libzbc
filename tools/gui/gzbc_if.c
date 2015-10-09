/*
 * This file is part of libzbc.
 *
 * Copyright (C) 2009-2014, HGST, Inc.  This software is distributed
 * under the terms of the GNU Lesser General Public License version 3,
 * or any later version, "as is," without technical support, and WITHOUT
 * ANY WARRANTY, without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  You should have received a copy
 * of the GNU Lesser General Public License along with libzbc.  If not,
 * see <http://www.gnu.org/licenses/>.
 *
 * Authors: Damien Le Moal (damien.lemoal@hgst.com)
 *          Christophe Louargant (christophe.louargant@hgst.com)
 */

/***** Including files *****/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#include "gzbc.h"

/***** Declaration of private functions *****/

static void
dz_if_show_nodev(void);

static void
dz_if_hide_nodev(void);

static gboolean
dz_if_resize_cb(GtkWidget *widget,
                GdkEvent *event,
                gpointer user_data);

static void
dz_if_delete_cb(GtkWidget *widget,
                GdkEvent *event,
                gpointer user_data);

static void
dz_if_open_cb(GtkWidget *widget,
              gpointer user_data);

static void
dz_if_close_cb(GtkWidget *widget,
	       gpointer user_data);

static void
dz_if_close_page_cb(GtkWidget *widget,
		    gpointer user_data);

static void
dz_if_switch_page_cb(GtkNotebook *notebook,
		     GtkWidget *page,
		     guint page_num,
		     gpointer user_data);

static void
dz_if_refresh_cb(GtkWidget *widget,
		 gpointer user_data);

static void
dz_if_set_block_size_cb(GtkWidget *widget,
			gpointer user_data);

static void
dz_if_exit_cb(GtkWidget *widget,
              gpointer user_data);

static gboolean
dz_if_timer_cb(gpointer user_data);

/***** Definition of public functions *****/

int
dz_if_create(void)
{
    GtkWidget *toolbar;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *sep;
    GtkToolItem *ti;
    char str[32];

    /* Get colors */
    gdk_rgba_parse(&dz.conv_color, "Magenta");
    gdk_rgba_parse(&dz.seqnw_color, "Green");
    gdk_rgba_parse(&dz.seqw_color, "Red");

    /* Window */
    dz.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(dz.window), "ZBC Device Zone State");
    gtk_container_set_border_width(GTK_CONTAINER(dz.window), 10);
    gtk_window_resize(GTK_WINDOW(dz.window), 1024, 768);

    g_signal_connect((gpointer) dz.window, "delete-event",
                     G_CALLBACK(dz_if_delete_cb),
                     NULL);

    /* Top vbox */
    dz.vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_show(dz.vbox);
    gtk_container_add(GTK_CONTAINER(dz.window), dz.vbox);

    /* Toolbar */
    toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
    gtk_widget_show(toolbar);
    gtk_box_pack_start(GTK_BOX(dz.vbox), toolbar, FALSE, FALSE, 0);

    /* Toolbar open button */
    ti = gtk_tool_button_new(gtk_image_new_from_icon_name("gtk-open", GTK_ICON_SIZE_LARGE_TOOLBAR), "Open");
    gtk_tool_item_set_tooltip_text(ti, "Open a device");
    gtk_tool_item_set_is_important(ti, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ti, -1);
    g_signal_connect(G_OBJECT(ti), "clicked", G_CALLBACK(dz_if_open_cb), NULL);

    /* Toolbar close button */
    ti = gtk_tool_button_new(gtk_image_new_from_icon_name("gtk-close", GTK_ICON_SIZE_LARGE_TOOLBAR), "Close");
    gtk_tool_item_set_tooltip_text(ti, "Close current device");
    gtk_tool_item_set_is_important(ti, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ti, -1);
    g_signal_connect(G_OBJECT(ti), "clicked", G_CALLBACK(dz_if_close_cb), NULL);

    /* Toolbar refresh button */
    ti = gtk_tool_button_new(gtk_image_new_from_icon_name("gtk-refresh", GTK_ICON_SIZE_LARGE_TOOLBAR), "Refresh");
    gtk_tool_item_set_tooltip_text(ti, "Refresh current device zone information");
    gtk_tool_item_set_is_important(ti, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ti, -1);
    g_signal_connect(G_OBJECT(ti), "clicked", G_CALLBACK(dz_if_refresh_cb), NULL);

    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);

    /* Toolbar block size */
    ti = gtk_tool_item_new();
    gtk_tool_item_set_tooltip_text(ti, "Block size unit (B)");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ti, -1);
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(ti), hbox);
    label = gtk_label_new("Block size (B)");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    dz.block_size_entry = gtk_entry_new();
    snprintf(str, sizeof(str) - 1, "%d", dz.block_size);
    gtk_entry_set_text(GTK_ENTRY(dz.block_size_entry), str);
    gtk_box_pack_start(GTK_BOX(hbox), dz.block_size_entry, FALSE, FALSE, 0);
    g_signal_connect((gpointer)dz.block_size_entry, "activate", G_CALLBACK(dz_if_set_block_size_cb), NULL);

    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);

    /* Toolbar block size */
    ti = gtk_tool_button_new(gtk_image_new_from_icon_name("application-exit", GTK_ICON_SIZE_LARGE_TOOLBAR), "Quit");
    gtk_tool_item_set_tooltip_text(ti, "Quit");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ti, -1);
    g_signal_connect(G_OBJECT(ti), "clicked", G_CALLBACK(dz_if_exit_cb), NULL);

    /* Separator */
    sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_show(sep);
    gtk_box_pack_start(GTK_BOX(dz.vbox), sep, FALSE, FALSE, 0);

    /* Initially, no device open: show "no device" frame */
    dz_if_show_nodev();

    /* Add timer for automatic refresh */
    if ( dz.interval >= DZ_INTERVAL ) {
        dz.timer_id = g_timeout_add(dz.interval, dz_if_timer_cb, NULL);
    }

    /* Finish setup */
    g_signal_connect((gpointer) dz.window, "configure-event",
		     G_CALLBACK(dz_if_resize_cb),
		     NULL);

    gtk_widget_show_all(dz.window);

    return( 0 );

}

void
dz_if_destroy(void)
{

    if ( dz.timer_id ) {
        g_source_remove(dz.timer_id);
        dz.timer_id = 0;
    }

    if ( dz.window ) {
        gtk_widget_destroy(dz.window);
        dz.window = NULL;
    }

    return;

}

void
dz_if_add_device(char *dev_path)
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *button;
    dz_dev_t *dzd;
    int page_no;
    char str[128];

    /* Open device */
    dzd = dz_if_dev_open(dev_path);
    if ( ! dzd ) {
	dialog = gtk_message_dialog_new(GTK_WINDOW(dz.window),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"Open device %s failed", dev_path);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return;
    }

    dz_if_hide_nodev();

    /* Add page */
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

    snprintf(str, sizeof(str) - 1, "<b>%s</b>", dzd->path);
    label = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(label), str);
    gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    button = gtk_button_new_from_icon_name("gtk-close", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    g_signal_connect((gpointer) button, "clicked", G_CALLBACK(dz_if_close_page_cb), dzd);

    gtk_widget_show_all(hbox);

    page_no = gtk_notebook_append_page(GTK_NOTEBOOK(dz.notebook),
				       dzd->page_frame,
				       hbox);
    dzd->page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dz.notebook), page_no);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(dz.notebook), page_no);
    snprintf(str, sizeof(str) - 1, "%d", dzd->block_size);
    gtk_entry_set_text(GTK_ENTRY(dz.block_size_entry), str);

    return;

}

/***** Definition of private functions *****/

static void
dz_if_show_nodev(void)
{

    if ( dz.notebook ) {
	/* Remove notebook */
	gtk_widget_destroy(dz.notebook);
	dz.notebook = NULL;
    }

    if ( ! dz.no_dev_frame ) {

	GtkWidget *frame;
	GtkWidget *label;

	frame = gtk_frame_new(NULL);
	gtk_widget_show(frame);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 10);
	gtk_box_pack_start(GTK_BOX(dz.vbox), frame, TRUE, TRUE, 0);

	label = gtk_label_new(NULL);
	gtk_widget_show(label);
	gtk_label_set_text(GTK_LABEL(label), "<b>No device open</b>");
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
	gtk_container_add(GTK_CONTAINER(frame), label);

	dz.no_dev_frame = frame;

    }

    return;

}

static void
dz_if_hide_nodev(void)
{

    if ( dz.no_dev_frame ) {
	/* Remove "no device" frame */
	gtk_widget_destroy(dz.no_dev_frame);
	dz.no_dev_frame = NULL;
    }

    if ( ! dz.notebook ) {
	/* Create the notebook */
	dz.notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(dz.notebook), GTK_POS_TOP);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(dz.notebook), TRUE);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(dz.notebook), TRUE);
	gtk_widget_show(dz.notebook);
	gtk_box_pack_start(GTK_BOX(dz.vbox), dz.notebook, TRUE, TRUE, 0);
	g_signal_connect((gpointer)dz.notebook, "switch_page", G_CALLBACK(dz_if_switch_page_cb), NULL);
    }

    return;

}

static void
dz_if_remove_device(dz_dev_t *dzd)
{
    int page_no = gtk_notebook_page_num(GTK_NOTEBOOK(dz.notebook), dzd->page_frame);

    /* Close the device */
    dz_if_dev_close(dzd);

    /* Remove the page */
    gtk_notebook_remove_page(GTK_NOTEBOOK(dz.notebook), page_no);
    dzd->page = NULL;

    if ( dz.nr_devs == 0 ) {
	/* Show "no device" */
	dz_if_show_nodev();
    }

    return;

}

static dz_dev_t *
dz_if_get_device(void)
{
    dz_dev_t *dzd = NULL;
    GtkWidget *page = NULL;
    int i;

    if ( ! dz.notebook ) {
	return( NULL );
    }

    page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dz.notebook),
				     gtk_notebook_get_current_page(GTK_NOTEBOOK(dz.notebook)));
    for(i = 0; i < DZ_MAX_DEV; i++) {
	dzd = &dz.dev[i];
	if ( dzd->dev && (dzd->page == page) ) {
	    return( dzd );
	}
    }

    return( NULL );

}

static void
dz_if_open_cb(GtkWidget *widget,
	      gpointer user_data)
{
    GtkWidget *dialog;
    char *dev_path = NULL;
    gint res;

    /* File chooser */
    dialog = gtk_file_chooser_dialog_new ("Open Device",
					  GTK_WINDOW(dz.window),
					  GTK_FILE_CHOOSER_ACTION_OPEN,
					  "_Cancel",
					  GTK_RESPONSE_CANCEL,
					  "_Open",
					  GTK_RESPONSE_ACCEPT,
					  NULL);
    /* gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), "/dev/sg*"); */

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if ( res == GTK_RESPONSE_ACCEPT ) {
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
	dev_path = gtk_file_chooser_get_filename(chooser);
    }

    gtk_widget_destroy(dialog);

    if ( dev_path ) {
	dz_if_add_device(dev_path);
	g_free(dev_path);
    }

    return;

}

static void
dz_if_close_cb(GtkWidget *widget,
	       gpointer user_data)
{
    dz_dev_t *dzd = dz_if_get_device();

    if ( dzd ) {
	dz_if_remove_device(dzd);
    }

    return;

}

static void
dz_if_close_page_cb(GtkWidget *widget,
		    gpointer user_data)
{
    dz_dev_t *dzd = (dz_dev_t *) user_data;

    if ( dzd ) {
	dz_if_remove_device(dzd);
    }

    return;

}

static void
dz_if_switch_page_cb(GtkNotebook *notebook,
		     GtkWidget *page,
		     guint page_num,
		     gpointer user_data)
{
    dz_dev_t *dzd = NULL;
    char str[32];
    int i;

    for(i = 0; i < DZ_MAX_DEV; i++) {
	dzd = &dz.dev[i];
	if ( dzd->dev && (dzd->page == page) ) {
	    snprintf(str, sizeof(str) - 1, "%d", dzd->block_size);
	    gtk_entry_set_text(GTK_ENTRY(dz.block_size_entry), str);
	    break;
	}
    }

    return;

}

static void
dz_if_refresh_cb(GtkWidget *widget,
		 gpointer user_data)
{
    dz_dev_t *dzd = dz_if_get_device();

    if ( dzd ) {
	dz_if_dev_refresh(dzd, 1);
    }

    return;

}

static void
dz_if_set_block_size_cb(GtkWidget *widget,
			gpointer user_data)
{
    char *val_str = (char *) gtk_entry_get_text(GTK_ENTRY(dz.block_size_entry));
    dz_dev_t *dzd = dz_if_get_device();

    if ( val_str && dzd ) {

	int block_size = atoi(val_str);

	if ( block_size > 0 ) {
	    dzd->block_size = block_size;
	} else {
	    char str[32];
	    if ( dz.block_size ) {
		dzd->block_size = dz.block_size;
	    } else {
		dzd->block_size = dzd->info.zbd_logical_block_size;
	    }
	    snprintf(str, sizeof(str) - 1, "%d", dzd->block_size);
	    gtk_entry_set_text(GTK_ENTRY(dz.block_size_entry), str);
	}
	dz_if_dev_refresh(dzd, 1);

    }

    return;

}

static void
dz_if_exit_cb(GtkWidget *widget,
              gpointer user_data)
{
    dz_dev_t *dzd;
    int i;

    if ( dz.notebook ) {
	for(i = 0; i < DZ_MAX_DEV; i++) {
	    dzd = &dz.dev[i];
	    if ( dzd->dev ) {
		dz_if_remove_device(dzd);
	    }
	}
    }

    gtk_main_quit();

    return;

}

static gboolean
dz_if_timer_cb(gpointer user_data)
{
    dz_dev_t *dzd = dz_if_get_device();

    if ( dzd ) {
	dz_if_dev_refresh(dzd, 1);
    }

    return( TRUE );

}

static gboolean
dz_if_resize_cb(GtkWidget *widget,
                GdkEvent *event,
                gpointer user_data)
{
    dz_dev_t *dzd = dz_if_get_device();

    if ( dzd ) {
	dz_if_dev_refresh(dzd, 0);
    }

    return( FALSE );

}

static void
dz_if_delete_cb(GtkWidget *widget,
                GdkEvent *event,
                gpointer user_data)
{
    dz_dev_t *dzd;
    int i;

    dz.window = NULL;

    if ( dz.notebook ) {
	for(i = 0; i < DZ_MAX_DEV; i++) {
	    dzd = &dz.dev[i];
	    if ( dzd->dev ) {
		dz_if_remove_device(dzd);
	    }
	}
    }

    gtk_main_quit();

    return;

}
