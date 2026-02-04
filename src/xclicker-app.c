#include <gtk/gtk.h>

#include "xclicker-app.h"
#include "mainwin.h"
#include "utils.h"

struct _XClickerApp
{
	GtkApplication parent;
	GtkStatusIcon *status_icon;
	MainAppWindow *main_window;
	gboolean window_visible;
};

G_DEFINE_TYPE(XClickerApp, xclicker_app, GTK_TYPE_APPLICATION);

static void status_icon_activate(GtkStatusIcon *status_icon, XClickerApp *app);
static void status_icon_popup_menu(GtkStatusIcon *status_icon, guint button, guint activate_time, XClickerApp *app);

static void xclicker_app_init(XClickerApp *app)
{
	app->status_icon = NULL;
	app->main_window = NULL;
	app->window_visible = FALSE;
}

/**
 * Creates and shows the system tray icon
 */
static void create_status_icon(XClickerApp *app)
{
	if (app->status_icon)
	{
		return;
	}

	// Load icon
	GError *error = NULL;
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_resource("/res/icon.png", &error);

	if (pixbuf)
	{
		app->status_icon = gtk_status_icon_new_from_pixbuf(pixbuf);
		g_object_unref(pixbuf);
	}

	gtk_status_icon_set_visible(app->status_icon, TRUE);
	gtk_status_icon_set_tooltip_text(app->status_icon, "XClicker");
	gtk_status_icon_set_has_tooltip(app->status_icon, TRUE);

	g_signal_connect(app->status_icon, "activate", G_CALLBACK(status_icon_activate), app);
	g_signal_connect(app->status_icon, "popup-menu", G_CALLBACK(status_icon_popup_menu), app);
}

/**
 * Opens up main window.
 */
static void xclicker_app_activate(GApplication *gapp)
{
	XClickerApp *app = XCLICKER_APP(gapp);

	if (!app->main_window)
	{
		app->main_window = main_app_window_new(app);
		app->window_visible = TRUE;

		// Connect to window hide event
		g_signal_connect(app->main_window, "hide", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
		g_signal_connect(app->main_window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
	}

	if (!app->window_visible)
	{
		gtk_window_present(GTK_WINDOW(app->main_window));
		app->window_visible = TRUE;
	}

	// Create status icon after window is created
	create_status_icon(app);

	// Show window immediately on startup
	gtk_window_present(GTK_WINDOW(app->main_window));
	app->window_visible = TRUE;
}

static void xclicker_app_class_init(XClickerAppClass *class)
{
	G_APPLICATION_CLASS(class)->activate = xclicker_app_activate;
}

XClickerApp *xclicker_app_new()
{
	return g_object_new(XCLICKER_APP_TYPE, NULL);
}

/**
 * Show window when status icon is left-clicked.
 */
static void status_icon_activate(GtkStatusIcon *status_icon, XClickerApp *app)
{
	show_window(app);
}

/**
 * Show popup menu when status icon is right-clicked.
 */
static void status_icon_popup_menu(GtkStatusIcon *status_icon, guint button, guint activate_time, XClickerApp *app)
{
	GtkWidget *menu = gtk_menu_new();

	// Show menu item
	GtkWidget *toggle_item = gtk_menu_item_new_with_label("Show Window");
	g_signal_connect_swapped(toggle_item, "activate",
							 G_CALLBACK(show_window), app);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_item);

	// Separator
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	// Quit menu item
	GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");
	g_signal_connect_swapped(quit_item, "activate",
							 G_CALLBACK(xclicker_app_quit), app);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);

	gtk_widget_show_all(menu);
	gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}

/**
 * Shows the main window.
 */
void show_window(XClickerApp *app)
{
	if (!app->main_window)
		return;

	gtk_window_present(GTK_WINDOW(app->main_window));
	app->window_visible = TRUE;
}

/**
 * Quits the application completely.
 */
void xclicker_app_quit(XClickerApp *app)
{
	g_application_quit(G_APPLICATION(app));
}
