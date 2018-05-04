#ifndef SQLiteBrowserH
#define SQLiteBrowserH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <sqlite3.h>

enum
{
	COL_NAME = 0,
	COL_TYPE,
	COL_PATH,
	NUM_COLS
};

typedef struct
{
	GtkWidget *window;
	GtkWidget *box;
	GtkWidget *frame;
	GtkWidget *hpaned;
	GtkWidget *frame1;
	GtkWidget *frame2;
	GtkWidget *vpaned;
	GtkWidget *frameh1;
	GtkWidget *frameh2;
	GtkWidget *vboxdb;
	GtkWidget *httbox;
	GtkWidget *ttoolbar;
	GtkToolItem *dbrefreshbutton;
	GtkWidget *view;
	GtkTreeSelection *selection;
	GtkTreeStore *treestore;
	GtkTreeIter toplevel, child, grandchild;
	GtkWidget *treescroll;
	GtkWidget *vqbox;
	GtkWidget *queryscroll;
	GtkWidget *htbox;
	GtkWidget *qtoolbar;
	GtkWidget *icon_widget;
	GtkToolItem *openquerybutton;
	GtkToolItem *savequerybutton;
	GtkToolItem *runquerybutton;
	GtkWidget *hqbox;
	GtkWidget *querytext;

	GtkWidget *resultview;
	GtkListStore *resultstore;
	GtkWidget *hboxres;
	GtkWidget *resultscroll;
	GtkWidget *vboxres;

	GtkWidget *statusbox;
	GtkWidget *statusbar;
	gint statusbar_cid;

	char dbpath[256];
	int recordcount;
}sqlitebrowser;

#endif
