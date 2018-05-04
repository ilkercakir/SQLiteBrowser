/*
 * SQLiteBrowser.c
 * 
 * Copyright 2018  <pi@raspberrypi>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

// compile with gcc -Wall -c "%f" -DIS_RPI -DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -g -ftree-vectorize -pipe -DUSE_VCHIQ_ARM -Wno-psabi -mcpu=cortex-a53 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits $(pkg-config --cflags gtk+-3.0) -Wno-deprecated-declarations
// link with gcc -Wall -o "%e" "%f" $(pkg-config --cflags gtk+-3.0) -Wl,--whole-archive -lrt -ldl -lm -Wl,--no-whole-archive -rdynamic $(pkg-config --libs gtk+-3.0) $(pkg-config --libs sqlite3)

#include "SQLiteBrowser.h"

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	return FALSE; // return FALSE to emit destroy signal
}

static void destroy(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

static void realize_cb(GtkWidget *widget, gpointer data)
{

}

void setup_default_icon(char *filename)
{
	GdkPixbuf *pixbuf;
	GError *err;

	err = NULL;
	pixbuf = gdk_pixbuf_new_from_file(filename, &err);

	if (pixbuf)
	{
		GList *list;      

		list = NULL;
		list = g_list_append(list, pixbuf);
		gtk_window_set_default_icon_list(list);
		g_list_free(list);
		g_object_unref(pixbuf);
    	}
}

int field_select_callback(void *data, int argc, char **argv, char **azColName) 
{
	sqlitebrowser *b = (sqlitebrowser *)data;
/*
	int i;
	for(i=0;i<argc;i++)
	{
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
*/
	gtk_tree_store_append(b->treestore, &(b->grandchild), &(b->child));
	gtk_tree_store_set(b->treestore, &(b->grandchild), COL_NAME, argv[1], COL_TYPE, argv[2], COL_PATH, NULL, -1);

//g_print("%s, %s\n", argv[0], argv[1]);
	return 0;
}

void read_fields(sqlitebrowser *b, char *table)
{
	sqlite3 *db;
	char *err_msg = NULL;
	char sql[100];
	int rc;

	if ((rc = sqlite3_open(b->dbpath, &db)))
	{
		printf("Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
//printf("Opened database successfully\n");
		sprintf(sql, "pragma table_info(%s);", table);
		//sql = "pragma table_info(eqpresets);";
		if ((rc = sqlite3_exec(db, sql, field_select_callback, b, &err_msg)) != SQLITE_OK)
		{
			printf("Failed to select data, %s\n", err_msg);
			sqlite3_free(err_msg);
		}
		else
		{
// success
		}
	}
	sqlite3_close(db);
}

int table_select_callback(void *data, int argc, char **argv, char **azColName) 
{
	sqlitebrowser *b = (sqlitebrowser *)data;
/*
	int i;
	for(i=0;i<argc;i++)
	{
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
*/
	gtk_tree_store_append(b->treestore, &(b->child), &(b->toplevel));
	gtk_tree_store_set(b->treestore, &(b->child), COL_NAME, argv[1], COL_TYPE, argv[0], COL_PATH, argv[2], -1);

	read_fields(b, argv[1]);

//g_print("%s, %s\n", argv[0], argv[1]);
	return 0;
}

void read_tables(sqlitebrowser *b, char *path)
{
	sqlite3 *db;
	char *err_msg = NULL;
	char *sql = NULL;
	int rc;

	strcpy(b->dbpath, path);

	if ((rc = sqlite3_open(b->dbpath, &db)))
	{
		printf("Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
//printf("Opened database successfully\n");
		sql = "SELECT * FROM sqlite_master WHERE type='table';";
		//sql = "pragma table_info(eqpresets);";
		if ((rc = sqlite3_exec(db, sql, table_select_callback, b, &err_msg)) != SQLITE_OK)
		{
			printf("Failed to select data, %s\n", err_msg);
			sqlite3_free(err_msg);
		}
		else
		{
// success
		}
	}
	sqlite3_close(db);
}

GtkTreeModel *create_and_fill_model(sqlitebrowser *b, char *path)
{
	char *line = NULL;
	size_t len = 0;
	FILE *f = fopen(path, "r");

	b->treestore = gtk_tree_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	if (f)
	{
		while(getline(&line, &len, f) > 0)
		{
			//printf("%s", line);
			char *p1 = line;
			char *q;
			if ((q = strstr(p1, ";")))
			{
				q[0] = '\0';
				if (strlen(p1))
				{
					//strcpy(?, p1);
					char *p2 = q + 1;
					if ((q = strstr(p2, "\n"))) 
						q[0] = '\0';
					//strcpy(?, p2);
					if (strlen(p2))
					{
						gtk_tree_store_append(b->treestore, &(b->toplevel), NULL);
						gtk_tree_store_set(b->treestore, &(b->toplevel), COL_NAME, p1, COL_TYPE, "DB", COL_PATH, p2, -1);

						read_tables(b, p2);
						
					}
				}
//printf("%s\n", p1);

			}
			free(line); line = NULL; len = 0;
		}
		fclose(f);
	}
	else
	 printf("failed to read addresses from %s\n", path);

	return GTK_TREE_MODEL(b->treestore);
}

void create_view_and_model(sqlitebrowser *b, char *path)
{
  GtkTreeViewColumn *col;
  GtkCellRenderer *renderer;
  GtkTreeModel *model;

  b->view = gtk_tree_view_new();

  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Name");
  gtk_tree_view_append_column(GTK_TREE_VIEW(b->view), col);

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
  gtk_tree_view_column_add_attribute(col, renderer, "text", COL_NAME);


  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Type");
  gtk_tree_view_append_column(GTK_TREE_VIEW(b->view), col);

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
  gtk_tree_view_column_add_attribute(col, renderer, "text", COL_TYPE);


  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Path");
  gtk_tree_view_append_column(GTK_TREE_VIEW(b->view), col);

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
  gtk_tree_view_column_add_attribute(col, renderer, "text", COL_PATH);

  model = create_and_fill_model(b, path);
  gtk_tree_view_set_model(GTK_TREE_VIEW(b->view), model);
  g_object_unref(model); 
}

void treeview_selection_changed(GtkWidget *widget, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *name, *type, *path;

	sqlitebrowser *b = (sqlitebrowser *)data;

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter))
	{
		gtk_tree_model_get(model, &iter, COL_NAME, &name, COL_TYPE, &type, COL_PATH, &path, -1);
//printf("selected %s %s %s\n", name, type, path);

		g_free(name);
		gchar *gc = g_strnfill(10, '\0');
		g_stpcpy(gc, "DB");
		if (!g_strcmp0(type, gc))
		{
			strcpy(b->dbpath, path);
			gtk_statusbar_pop(GTK_STATUSBAR(b->statusbar), b->statusbar_cid);
			gtk_statusbar_push(GTK_STATUSBAR(b->statusbar), b->statusbar_cid, path);
		}
		g_free(gc);
		g_free(type);
		g_free(path);
	}
}

int run_query_callback(void *data, int argc, char **argv, char **azColName) 
{
	GtkCellRenderer *renderer;
	GType *columntypes;
	GtkTreeIter iter;
	int i;

	sqlitebrowser *b = (sqlitebrowser *)data;

	if (!b->recordcount)
	{
		b->resultview = gtk_tree_view_new();

		for(i=0;i<argc;i++)
		{
//			printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
			renderer = gtk_cell_renderer_text_new();
			gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(b->resultview), -1, azColName[i], renderer, "text", i, NULL);
		}
		columntypes = malloc(argc*sizeof(GType));
		for(i=0;i<argc;i++)
		{
			columntypes[i] = G_TYPE_STRING;
		}

		b->resultstore = gtk_list_store_newv(argc, columntypes);
	}

	gtk_list_store_append(b->resultstore, &iter);
	for(i=0;i<argc;i++)
	{
		gtk_list_store_set(b->resultstore, &iter, i, argv[i], -1);
	}

	b->recordcount++;

//g_print("%s, %s\n", argv[0], argv[1]);
	return 0;
}

void run_query(sqlitebrowser *b, char *query)
{
	sqlite3 *db;
	char *err_msg = NULL;
	char sql[200];
	int rc;

	if (b->resultview)
	{
		gtk_widget_destroy(b->resultview);
		b->resultview = NULL;
	}

	b->recordcount = 0;

	if ((rc = sqlite3_open(b->dbpath, &db)))
	{
		printf("Can't open database: %s\n", sqlite3_errmsg(db));
	}
	else
	{
//printf("Opened database successfully\n");
		sprintf(sql, "%s", query);
		if ((rc = sqlite3_exec(db, sql, run_query_callback, b, &err_msg)) != SQLITE_OK)
		{
			printf("Failed to select data, %s\n", err_msg);
			sqlite3_free(err_msg);
		}
		else
		{
// success
		}
	}
	sqlite3_close(db);

	if (b->recordcount)
	{
		GtkTreeModel *model = GTK_TREE_MODEL(b->resultstore);
		gtk_tree_view_set_model(GTK_TREE_VIEW(b->resultview), model);
		g_object_unref(model);

		gtk_box_pack_start(GTK_BOX(b->vboxres), b->resultview, TRUE, TRUE, 0);

		gtk_widget_show_all(b->vboxres);
	}
}

void openquerybutton_clicked(GtkToolButton *toolbutton, gpointer data)
{
	sqlitebrowser *b = (sqlitebrowser*)data;

	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GSList *chosenfile;

	dialog = gtk_file_chooser_dialog_new("Open Query", GTK_WINDOW(b->window), action, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
	gtk_file_chooser_set_select_multiple(chooser, FALSE);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GSList *filelist = gtk_file_chooser_get_filenames(chooser);
		for(chosenfile=filelist;chosenfile;chosenfile=chosenfile->next)
		{
//printf("%s\n", (char*)chosenfile->data);
			char *line = NULL;
			size_t len = 0;
			FILE *f = fopen((char*)chosenfile->data, "r");
			if (f)
			{
				gchar *qstring = g_strnfill(1024, '\0');
				gchar *q = qstring;
				while(getline(&line, &len, f) > 0)
				{
					//printf("%s", line);
					q = g_stpcpy(q, line);
					free(line); line = NULL; len = 0;
				}
				fclose(f);

				GtkTextBuffer *txtbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(b->querytext));
				gtk_text_buffer_set_text(txtbuf, qstring, -1);

				g_free(qstring);
			}
			else
				printf("failed to open file %s\n", (char*)chosenfile->data);
		}
	}
	gtk_widget_destroy(dialog);
}

void savequerybutton_clicked(GtkToolButton *toolbutton, gpointer data)
{
	sqlitebrowser *b = (sqlitebrowser*)data;

	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
	GSList *chosenfile;

	dialog = gtk_file_chooser_dialog_new("Save Query", GTK_WINDOW(b->window), action, "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
	gtk_file_chooser_set_select_multiple(chooser, FALSE);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GSList *filelist = gtk_file_chooser_get_filenames(chooser);
		for(chosenfile=filelist;chosenfile;chosenfile=chosenfile->next)
		{
//printf("%s\n", (char*)chosenfile->data);
			FILE *f = fopen((char*)chosenfile->data, "w");
			if (f)
			{
				GtkTextBuffer *txtbuf;
				GtkTextIter start, end;
				gchar *qstring;

				txtbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(b->querytext));
				gtk_text_buffer_get_bounds(txtbuf, &start, &end);
				qstring = gtk_text_buffer_get_text(txtbuf, &start, &end, TRUE);
				int i;
				for(i=0;qstring[i];i++)
					fwrite(&(qstring[i]), sizeof(*qstring), 1, f);
				g_free(qstring);

				fclose(f);
			}
			else
				printf("failed to save file %s\n", (char*)chosenfile->data);
		}
	}
	gtk_widget_destroy(dialog);
}

void runquerybutton_clicked(GtkToolButton *toolbutton, gpointer data)
{
	sqlitebrowser *b = (sqlitebrowser*)data;

	GtkTextBuffer *txtbuf;
	GtkTextIter start, end;
	gchar *qstring;
	
	txtbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(b->querytext));
	gtk_text_buffer_get_bounds(txtbuf, &start, &end);
	qstring = gtk_text_buffer_get_text(txtbuf, &start, &end, TRUE);

//printf("%s\n", qstring);
	run_query(b, qstring);

	g_free(qstring);
}

int main(int argc, char **argv)
{
	sqlitebrowser b;

	setup_default_icon("./images/SQLiteBrowser.png");

	gtk_init(&argc, &argv);

	/* create a new window */
	b.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(b.window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER (b.window), 2);
	//gtk_widget_set_size_request(window, 100, 100);
	gtk_window_set_title(GTK_WINDOW(b.window), "SQLite Browser");
	gtk_window_set_resizable(GTK_WINDOW(b.window), TRUE);
	/* When the window is given the "delete-event" signal (this is given
	* by the window manager, usually by the "close" option, or on the
	* titlebar), we ask it to call the delete_event () function
	* as defined above. The data passed to the callback
	* function is NULL and is ignored in the callback function. */
	g_signal_connect (b.window, "delete-event", G_CALLBACK (delete_event), NULL);
	/* Here we connect the "destroy" event to a signal handler.  
	* This event occurs when we call gtk_widget_destroy() on the window,
	* or if we return FALSE in the "delete-event" callback. */
	g_signal_connect(b.window, "destroy", G_CALLBACK (destroy), NULL);
	g_signal_connect(b.window, "realize", G_CALLBACK (realize_cb), NULL);

// vertical box
	b.box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(b.window), b.box);

// frame
	b.frame = gtk_frame_new("SQLite Browser");
	//gtk_container_add(GTK_CONTAINER(b.box), b.frame);
	gtk_box_pack_start(GTK_BOX(b.box), b.frame, TRUE, TRUE, 0);

// frame left
	b.frame1 = gtk_frame_new("Database");
	gtk_frame_set_shadow_type(GTK_FRAME(b.frame1), GTK_SHADOW_IN);

// treeview
	b.vboxdb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(b.frame1), b.vboxdb);

	b.treescroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(b.treescroll), 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(b.treescroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	//gtk_widget_set_size_request(b.treescroll, 100, 100);
	//gtk_container_add(GTK_CONTAINER(b.vboxdb), b.treescroll);
	gtk_box_pack_start(GTK_BOX(b.vboxdb), b.treescroll, TRUE, TRUE, 0);

	create_view_and_model(&b, "./SQLiteBrowser.txt");
	b.selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(b.view));
	g_signal_connect(b.selection, "changed", G_CALLBACK(treeview_selection_changed), &b);
	gtk_container_add(GTK_CONTAINER(b.treescroll), b.view);

// frame right
	b.frame2 = gtk_frame_new("Query");
	gtk_frame_set_shadow_type(GTK_FRAME(b.frame2), GTK_SHADOW_IN);

// frame top
	b.frameh1 = gtk_frame_new("Editor");
	gtk_frame_set_shadow_type(GTK_FRAME(b.frameh1), GTK_SHADOW_IN);

	b.vqbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(b.frameh1), b.vqbox);

	b.htbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_container_add(GTK_CONTAINER(b.vqbox), b.htbox);

	b.qtoolbar = gtk_toolbar_new();
	gtk_container_add(GTK_CONTAINER(b.htbox), b.qtoolbar);

	b.icon_widget = gtk_image_new_from_icon_name("document-open", GTK_ICON_SIZE_BUTTON);
	b.openquerybutton = gtk_tool_button_new(b.icon_widget, "Open Query");
	gtk_tool_item_set_tooltip_text(b.openquerybutton, "Open Query");
	g_signal_connect(b.openquerybutton, "clicked", G_CALLBACK(openquerybutton_clicked), &b);
	gtk_toolbar_insert(GTK_TOOLBAR(b.qtoolbar), b.openquerybutton, -1);

	b.icon_widget = gtk_image_new_from_icon_name("document-save", GTK_ICON_SIZE_BUTTON);
	b.savequerybutton = gtk_tool_button_new(b.icon_widget, "Save Query");
	gtk_tool_item_set_tooltip_text(b.savequerybutton, "Save Query");
	g_signal_connect(b.savequerybutton, "clicked", G_CALLBACK(savequerybutton_clicked), &b);
	gtk_toolbar_insert(GTK_TOOLBAR(b.qtoolbar), b.savequerybutton, -1);

	b.icon_widget = gtk_image_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_BUTTON);
	b.runquerybutton = gtk_tool_button_new(b.icon_widget, "Run Query");
	gtk_tool_item_set_tooltip_text(b.runquerybutton, "Run Query");
	g_signal_connect(b.runquerybutton, "clicked", G_CALLBACK(runquerybutton_clicked), &b);
	gtk_toolbar_insert(GTK_TOOLBAR(b.qtoolbar), b.runquerybutton, -1);

	b.queryscroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(b.queryscroll), 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(b.queryscroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	//gtk_widget_set_size_request(b.queryscroll, 100, 100);
	//gtk_container_add(GTK_CONTAINER(b.vqbox), b.queryscroll);
	gtk_box_pack_start(GTK_BOX(b.vqbox), b.queryscroll, TRUE, TRUE, 0);

	b.hqbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(b.queryscroll), b.hqbox);

	b.querytext = gtk_text_view_new();
	gtk_widget_set_size_request(b.querytext, 100, 100);
	gtk_container_add(GTK_CONTAINER(b.hqbox), b.querytext);

// frame bottom
	b.frameh2 = gtk_frame_new("Records");
	gtk_frame_set_shadow_type(GTK_FRAME(b.frameh2), GTK_SHADOW_IN);

	b.hboxres = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_container_add(GTK_CONTAINER(b.frameh2), b.hboxres);

	b.resultscroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(b.resultscroll), 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(b.resultscroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	//gtk_widget_set_size_request(b.resultscroll, 100, 100);
	//gtk_container_add(GTK_CONTAINER(b.hboxres), b.resultscroll);
	gtk_box_pack_start(GTK_BOX(b.hboxres), b.resultscroll, TRUE, TRUE, 0);

	b.vboxres = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(b.resultscroll), b.vboxres);

	b.resultview = NULL;

// horizontal panes
	b.vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
	gtk_container_add(GTK_CONTAINER(b.frame2), b.vpaned);

	gtk_paned_pack1(GTK_PANED(b.vpaned), b.frameh1, TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED(b.vpaned), b.frameh2, TRUE, TRUE);

// vertical panes
	b.hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_container_add(GTK_CONTAINER(b.frame), b.hpaned);

	gtk_paned_pack1(GTK_PANED(b.hpaned), b.frame1, TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED(b.hpaned), b.frame2, TRUE, TRUE);

// horizontal box
	b.statusbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_container_add(GTK_CONTAINER(b.box), b.statusbox);

// statusbar
	b.statusbar = gtk_statusbar_new();
	gtk_container_add(GTK_CONTAINER(b.statusbox), b.statusbar);
	//gtk_box_pack_start(GTK_BOX(b.box), b.statusbar, TRUE, TRUE, 0);
	b.statusbar_cid = gtk_statusbar_get_context_id(GTK_STATUSBAR(b.statusbar), "SQLiteBrowser");
	gtk_statusbar_push(GTK_STATUSBAR(b.statusbar), b.statusbar_cid, "SQLiteBrowser");

	gtk_widget_show_all(b.window);
	gtk_main();

	return 0;
}
