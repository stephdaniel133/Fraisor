#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "main.h"
#include "error.h"

static void print_message(global_t *pGlobal, GtkMessageType, const gchar *, va_list);

void print_info(global_t *pGlobal, char *format, ...)
{
    va_list va;

    va_start (va, format);
    print_message(pGlobal, GTK_MESSAGE_INFO, format, va);
    va_end(va);
}

void print_warning(global_t *pGlobal, char *format, ...)
{
    va_list va;

    va_start (va, format);
    print_message(pGlobal, GTK_MESSAGE_WARNING, format, va);
    va_end(va);
}

void print_error(global_t *pGlobal, char *format, ...)
{
    va_list va;

    va_start (va, format);
    print_message (pGlobal, GTK_MESSAGE_ERROR, format, va);
    va_end(va);
    exit (EXIT_FAILURE);
}

static void print_message(global_t *pGlobal, GtkMessageType type, const gchar *format, va_list va)
{
    gchar *message = NULL;
    GtkWidget *p_dialog = NULL;

    message = g_strdup_vprintf (format, va);
    printf("%s", message);
    p_dialog = gtk_message_dialog_new (GTK_WINDOW(pGlobal->pMainWindow), GTK_DIALOG_MODAL, type, GTK_BUTTONS_OK, "%s", message);

    gtk_dialog_run (GTK_DIALOG (p_dialog));
    gtk_widget_destroy (p_dialog);

    g_free (message);
    message = NULL;
}
