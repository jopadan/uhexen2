/*
 * main.c initial g�n�r� par Glade. �diter ce fichier � votre
 * convenance. Glade n'�crira plus dans ce fichier.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>

#include "interface.h"
#include "support.h"
#include "config_file.h"

char *argv_0;
static  char    userdir[1024];

int
main (int argc, char *argv[])
{
  GtkWidget *window1;
  argv_0 = argv[0];

#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif

  gtk_set_locale ();
  gtk_init (&argc, &argv);

  add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps");
  add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps");

  if (!(Sys_GetUserdir(userdir,sizeof(userdir)))) {
    fprintf (stderr,"Couldn't determine userspace directory");
    exit(0);
  }
  else printf("userdir is: %s\n",userdir);

  fill_default_options();
  read_config_file();

  /*
   * The following code was added by Glade to create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */
  window1 = create_window1 ();
  gtk_widget_show (window1);

  gtk_main ();
  return 0;
}

