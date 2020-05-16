//Fichier callback.c
//Contient toutes les fonctions Callback de l'interface
#include <stdlib.h>
#include <gtk/gtk.h>
#include "rs232.h"

#include "main.h"
#include "callback.h"
#include "error.h"
#include "thread.h"
#include "communication.h"


//----------------------Callback du menu de l'interface-----------------------//
/*gboolean CB_Menu_Nouveau_Status(GtkWidget * widget, GdkEventCrossing * event, gpointer user_data)
{
    gchar* sUtf8;

    // La souris rentre sur le widget
    if (event->type == GDK_ENTER_NOTIFY)
    {
        gtk_menu_item_select(GTK_MENU_ITEM(widget));
        sUtf8 = g_locale_to_utf8("Créer un nouveau fichier programme", -1, NULL, NULL, NULL);
        gtk_statusbar_push (GTK_STATUSBAR (pStatusBar), GPOINTER_TO_INT(user_data), sUtf8);
    }
    // La souris sort du widget
    else if (event->type == GDK_LEAVE_NOTIFY)
    {
        gtk_menu_item_deselect(GTK_MENU_ITEM(widget));
        gtk_statusbar_pop(GTK_STATUSBAR(pStatusBar), GPOINTER_TO_INT(user_data));
    }
    return TRUE;
}*/

void CB_Menu_Nouveau(GtkWidget* pWidget, global_t* pGlobal)
{
    GtkWidget* p_dialog = NULL;
    GtkTextIter start;
    GtkTextIter end;
    GtkTextBuffer *p_text_buffer = NULL;

    p_dialog = gtk_message_dialog_new(GTK_WINDOW(pGlobal->pMainWindow),
                                      GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_QUESTION,
                                      GTK_BUTTONS_YES_NO,
                                      "Voulez-vous sauvegarder le programme ?");

    switch(gtk_dialog_run(GTK_DIALOG(p_dialog)))
    {
        case GTK_RESPONSE_YES:
            free(pGlobal->chemin);
            pGlobal->chemin = NULL;
            CB_Menu_Sauvegarder(pWidget, pGlobal);
            gtk_widget_destroy(p_dialog);
            break;

        case GTK_RESPONSE_NO:
            gtk_widget_destroy(p_dialog);
            break;
    }

    p_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pGlobal->pTextView));
    gtk_text_buffer_get_bounds(p_text_buffer, &start, &end);
    gtk_text_buffer_delete(p_text_buffer, &start, &end);
    pGlobal->sauve = TRUE;
}


void CB_Menu_Ouvrir(GtkWidget* pWidget, global_t* pGlobal)
{
    gchar* file_name = NULL;
    gchar* utf8 = NULL;
    GtkTextIter start;
    GtkTextIter end;
    GtkTextIter iter;
    GtkTextBuffer* p_text_buffer = NULL;
    GtkWidget* p_dialog = NULL;
    gchar* contents = NULL;

    p_dialog = gtk_message_dialog_new(GTK_WINDOW(pGlobal->pMainWindow),
                                      GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_QUESTION,
                                      GTK_BUTTONS_YES_NO,
                                      "Voulez-vous sauvegarder le programme ?");

    switch(gtk_dialog_run(GTK_DIALOG(p_dialog)))
    {
        case GTK_RESPONSE_YES:
            free(pGlobal->chemin);
            pGlobal->chemin = NULL;
            CB_Menu_Sauvegarder(pWidget, pGlobal);
            gtk_widget_destroy(p_dialog);
            break;

        case GTK_RESPONSE_NO:
            gtk_widget_destroy(p_dialog);
            break;
    }

    //Ouverture du fichier
    p_dialog = gtk_file_chooser_dialog_new ("Ouvrir un fichier",
                                            GTK_WINDOW(pGlobal->pMainWindow),
                                            GTK_FILE_CHOOSER_ACTION_OPEN,
                                            "Annuler", GTK_RESPONSE_CANCEL,
                                            "Ouvrir", GTK_RESPONSE_ACCEPT,
                                            NULL);
    if (gtk_dialog_run (GTK_DIALOG (p_dialog)) == GTK_RESPONSE_ACCEPT)
    {
        file_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (p_dialog));
        g_return_if_fail(file_name && pGlobal->pTextView);

        if (g_file_get_contents(file_name, &contents, NULL, NULL))
        {
            p_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pGlobal->pTextView));
            // Effacement du contenu du GtkTextView
            gtk_text_buffer_get_bounds(p_text_buffer, &start, &end);
            gtk_text_buffer_delete(p_text_buffer, &start, &end);
            // Copie de contents dans le GtkTextView
            gtk_text_buffer_get_iter_at_line (p_text_buffer, &iter, 0);
            utf8 = g_locale_to_utf8 (contents, -1, NULL, NULL, NULL);
            g_free (contents), contents = NULL;
            gtk_text_buffer_insert (p_text_buffer, &iter, utf8, -1);
            g_free (utf8), utf8 = NULL;
            pGlobal->chemin = g_strdup(file_name);
            pGlobal->sauve = TRUE;
        }
        else
        {
            print_warning(pGlobal, "Impossible d'ouvrir le fichier %s\n", file_name);
        }
        g_free (file_name), file_name = NULL;
    }
    gtk_widget_destroy (p_dialog);
}


void CB_Menu_Sauvegarder(GtkWidget* pWidget, global_t* pGlobal)
{
    GtkWidget* p_dialog = NULL;
    FILE* fichier = NULL;
    gchar* contents = NULL;
    gchar* locale = NULL;
    GtkTextIter start;
    GtkTextIter end;
    GtkTextBuffer* p_text_buffer = NULL;

    // Le fichier n'a pas encore ete enregistre
    if(!pGlobal->chemin)
    {
        p_dialog = gtk_file_chooser_dialog_new("Sauvegarder le fichier",
                                               GTK_WINDOW(pGlobal->pMainWindow),
                                               GTK_FILE_CHOOSER_ACTION_SAVE,
                                               "Annuler", GTK_RESPONSE_CANCEL,
                                               "Enregistrer", GTK_RESPONSE_ACCEPT,
                                               NULL);
        if(gtk_dialog_run(GTK_DIALOG(p_dialog)) == GTK_RESPONSE_ACCEPT)
        {
            pGlobal->chemin = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(p_dialog));
        }
        gtk_widget_destroy(p_dialog);
    }
    // Soit le fichier a deja ete enregistre, soit l'utilisateur vient de nous
    // fournir son nouvel emplacement, on peut donc l'enregistrer
    if(pGlobal->chemin != NULL)
    {
        fichier = fopen(pGlobal->chemin, "w");
        if(fichier)
        {
            p_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pGlobal->pTextView));
            gtk_text_buffer_get_bounds(p_text_buffer, &start, &end);
            contents = gtk_text_buffer_get_text(p_text_buffer, &start, &end, FALSE);
            locale = g_locale_from_utf8(contents, -1, NULL, NULL, NULL);
            g_free(contents), contents = NULL;
            fprintf(fichier, "%s", locale);
            g_free(locale), locale = NULL;
            fclose(fichier), fichier = NULL;
            pGlobal->sauve = TRUE;
        }
        else
        {
            print_warning (pGlobal, "Impossible de sauvegarder le fichier %s");
        }
    }
}


void CB_Menu_SauvegarderSous(GtkWidget* pWidget, global_t* pGlobal)
{
    GtkWidget* p_dialog = NULL;

    p_dialog = gtk_file_chooser_dialog_new("Sauvegarder le fichier",
                                           GTK_WINDOW(pGlobal->pMainWindow),
                                           GTK_FILE_CHOOSER_ACTION_SAVE,
                                           "Annuler", GTK_RESPONSE_CANCEL,
                                           "Enregistrer", GTK_RESPONSE_ACCEPT,
                                           NULL);
    if(gtk_dialog_run(GTK_DIALOG(p_dialog)) == GTK_RESPONSE_ACCEPT)
    {
        pGlobal->chemin = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(p_dialog));
        CB_Menu_Sauvegarder(pWidget, pGlobal);
    }
    gtk_widget_destroy (p_dialog);
}


gint CB_Delete_Event(GtkWidget* p_widget, GdkEvent* event, global_t* pGlobal)
{
    CB_Menu_Quitter(p_widget, pGlobal);

    return TRUE; //retourne TRUE pour ne pas detruire la fenêtre
}


void CB_Menu_Quitter(GtkWidget* pWidget, global_t* pGlobal)
{
    if (!pGlobal->sauve)
    {
        CB_Menu_Sauvegarder(pWidget, pGlobal);
    }
    CB_Deconnecter(pWidget, pGlobal);
    pGlobal->quit = 1;
    gtk_main_quit();
}



void CB_OnRadio(GtkWidget* pWidget, global_t* pGlobal)
{
    const gchar* sRadioName = NULL;

    if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(pWidget)))
    {
        // Recuperer le label du bouton radio active
        sRadioName = gtk_menu_item_get_label(GTK_MENU_ITEM(pWidget));

        // Si le port série est ouvert, on le clos
        if(pGlobal->comport_open == 0)
        {
            printf("Femeture du port COM%d\n", pGlobal->comport_number);
            RS232_CloseComport(pGlobal->comport_number);
            pGlobal->comport_open = 1;
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemDeconnecter), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemConnecter), TRUE);
        }

        pGlobal->comport_number = RS232_GetPortnr(sRadioName);
        printf("Port %s, pGlobal->comport_number = %d\n", sRadioName, pGlobal->comport_number);

    }
}



void CB_APropos(GtkWidget* pWidget, global_t* pGlobal)
{
    GtkWidget* p_about_dialog = NULL;
    const gchar* authors[] = {"Stephane DANIEL", "Gwenael DANIEL", NULL};
    GdkPixbuf* p_logo = NULL;

    p_about_dialog = gtk_about_dialog_new();
    gtk_window_set_transient_for(GTK_WINDOW(p_about_dialog), GTK_WINDOW(pGlobal->pMainWindow));

    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(p_about_dialog), "1.0");
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(p_about_dialog), authors);

    gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(p_about_dialog), GTK_LICENSE_LGPL_3_0);

    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(p_about_dialog), "https://github.com/stephdaniel133/Fraisor.git");
    gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(p_about_dialog), "Github");

    p_logo = gdk_pixbuf_new_from_file("logo.png", NULL);
    gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG(p_about_dialog), p_logo);

    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(p_about_dialog), "@2010-2020 Stephane DANIEL");

    gtk_dialog_run(GTK_DIALOG(p_about_dialog));

    gtk_widget_destroy(p_about_dialog);
}




//----------------------Callback de la barre d'outils de l'interface-----------------------//
void CB_Connecter(GtkWidget* pWidget, global_t* pGlobal)
{
    char mode[]={'8', 'N', '1', 0};

    if(pGlobal->comport_number != -1)
    {
        printf("\nClique sur Connecter, connexion sur %s\n", RS232_GetPortName(pGlobal->comport_number));

        if(RS232_OpenComport(pGlobal->comport_number, 115200, mode, 0))
        {
            pGlobal->comport_open = 1;
            print_info(pGlobal, "%s %s", "\nImpossible d'ouvrir le port serie", RS232_GetPortName(pGlobal->comport_number));
            return;
        }
        else
        {                   //On a réussi à ouvrir le port, on essaye de se connecter à la fraiseuse
            if(Attente_Connexion(pGlobal) == 1)
            {
                pGlobal->comport_open = 0;
                gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemDeconnecter), TRUE);
                gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemConnecter), FALSE);
                printf("--- Connexion etablie ---\n");
            }
            else
            {
                RS232_CloseComport(pGlobal->comport_number);
                printf("--- Connexion non etablie ! ---\n");
            }
        }
    }
    else
    {
        print_info(pGlobal, "%s", "\nVeuillez selectionner un port serie !");
    }
}

void CB_Deconnecter(GtkWidget* pWidget, global_t* pGlobal)
{
    if(0 == pGlobal->comport_open)
    {
        Envoi_Reset_Fraiseuse(pGlobal);
        g_usleep(100*1000); //On laisse le temps au dsPIC de traiter la ligne pendant 20ms
        RS232_CloseComport(pGlobal->comport_number);
        pGlobal->comport_open = 1;
        gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemDeconnecter), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemConnecter), TRUE);
    }
}

void CB_Lecture(GtkWidget* pWidget, global_t* pGlobal)
{
    GError *err = NULL;
    GtkTextBuffer* buffer = NULL;
    GtkTextIter start;
    GtkTextIter end;
    gchar* buffer_ligne = NULL;
    char ajout = 0x0A;  //Caractere saut de ligne
    int nombre_ligne = gtk_text_buffer_get_line_count(gtk_text_view_get_buffer(GTK_TEXT_VIEW(pGlobal->pTextView)));


    gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), FALSE);

    //Ajout d'un saut de ligne à la fin du TextView s'il n'y en a pas
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pGlobal->pTextView));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_get_iter_at_line(buffer, &start, nombre_ligne-1);
    buffer_ligne = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if(strcmp(buffer_ligne, "") != 0)
    {
        gtk_text_buffer_insert(buffer, &end, &ajout, 1);
    }

    printf("Clique sur Lecture\n");

    pGlobal->Etat = LECTURE;
    pGlobal->nombre_ligne = 0;

    if((pGlobal->Thread1 = g_thread_try_new("Thread_LectureStop", (GThreadFunc)Thread_LectureStop, (global_t *)pGlobal, &err)) == NULL)
    {
        g_error("Thread LectureStop create failed: %s!!\n", err->message );
        g_error_free(err);
    }
}

void CB_Stop(GtkWidget* pWidget, global_t* pGlobal)
{
    gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), TRUE);

    Envoi_Arret_Programme(pGlobal);

    g_mutex_lock(pGlobal->Mutex_LectureStop);
    pGlobal->Etat = STOP;
    g_mutex_unlock(pGlobal->Mutex_LectureStop);
}

void CB_Pause(GtkWidget* pWidget, global_t* pGlobal)
{
    //gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), TRUE);

    Envoi_Pause_Programme(pGlobal);

    g_mutex_lock(pGlobal->Mutex_LectureStop);
    if(pGlobal->Etat == PAUSE)
    {
        pGlobal->Etat = LECTURE;
    }
    else
    {
        pGlobal->Etat = PAUSE;
    }
    g_mutex_unlock(pGlobal->Mutex_LectureStop);
}



//----------------------Callback des boutons de l'interface-----------------------//
void CB_Bouton_Xp_Click(GtkWidget* pWidget, global_t* pGlobal)
{
    Envoi_Deplacement('X', 1, pGlobal);
}

void CB_Bouton_Xm_Click(GtkWidget* pWidget, global_t* pGlobal)
{
    Envoi_Deplacement('X', 0, pGlobal);
}

void CB_Bouton_X_Affiche_Status(GtkWidget* pWidget, gpointer pStatusBar)
{
    // Ajout d'un message
    gtk_statusbar_push(GTK_STATUSBAR(pStatusBar), 0, "Bouge l'axe X de la fraiseuse");
}

void CB_Bouton_X_Efface_Status(GtkWidget* pWidget, gpointer pStatusBar)
{
    // Suppression d'un message
    gtk_statusbar_pop(GTK_STATUSBAR(pStatusBar), 0);
}

void CB_Bouton_Yp_Click(GtkWidget* pWidget, global_t* pGlobal)
{
    Envoi_Deplacement('Y', 1, pGlobal);
}

void CB_Bouton_Ym_Click(GtkWidget* pWidget, global_t* pGlobal)
{
    Envoi_Deplacement('Y', 0, pGlobal);
}

void CB_Bouton_Y_Affiche_Status(GtkWidget* pWidget, gpointer pStatusBar)
{
    // Ajout d'un message
    gtk_statusbar_push(GTK_STATUSBAR(pStatusBar), 0, "Bouge l'axe Y de la fraiseuse");
}

void CB_Bouton_Y_Efface_Status(GtkWidget* pWidget, gpointer pStatusBar)
{
    // Suppression d'un message
    gtk_statusbar_pop(GTK_STATUSBAR(pStatusBar), 0);
}

void CB_Bouton_Zp_Click(GtkWidget* pWidget, global_t* pGlobal)
{
    Envoi_Deplacement('Z', 1, pGlobal);
}

void CB_Bouton_Zm_Click(GtkWidget* pWidget, global_t* pGlobal)
{
    Envoi_Deplacement('Z', 0, pGlobal);
}

void CB_Bouton_Z_Affiche_Status(GtkWidget* pWidget, gpointer pStatusBar)
{
    // Ajout d'un message
    gtk_statusbar_push(GTK_STATUSBAR(pStatusBar), 0, "Bouge l'axe Z de la fraiseuse");
}

void CB_Bouton_Z_Efface_Status(GtkWidget* pWidget, gpointer pStatusBar)
{
    // Suppression d'un message
    gtk_statusbar_pop(GTK_STATUSBAR(pStatusBar), 0);
}

void ComboBoxVitProg_on_changed(GtkWidget* pWidget, global_t* pGlobal)
{
    Envoi_VitesseProgramme(pGlobal);
}

/*----------------------Callback des changements de repères piece de l'interface-----------------------*/
gint CB_Bouton_OK_ReperePiece(GtkButton* button, global_t* pGlobal)
{
    const gchar* sNom = NULL;
    float mesX = 0;
    float mesY = 0;
    float mesZ = 0;
    char temp[40] = "\0";

    sNom = gtk_entry_get_text(GTK_ENTRY(pGlobal->pLabelXnew));
    mesX = atof(sNom);
    sprintf(temp, "%3.2f", mesX);
    gtk_label_set_label(GTK_LABEL(pGlobal->pLabelXmm), temp);

    sNom = gtk_entry_get_text(GTK_ENTRY(pGlobal->pLabelYnew));
    mesY = atof(sNom);
    sprintf(temp, "%3.2f", mesY);
    gtk_label_set_label(GTK_LABEL(pGlobal->pLabelYmm), temp);

    sNom = gtk_entry_get_text(GTK_ENTRY(pGlobal->pLabelZnew));
    mesZ = atof(sNom);
    sprintf(temp, "%3.2f", mesZ);
    gtk_label_set_label(GTK_LABEL(pGlobal->pLabelZmm), temp);


    if(pGlobal->comport_open == 0)  //Si le port série est ouvert
    {
        Envoi_Changement_Repere_Piece('X', mesX, pGlobal);
        Envoi_Changement_Repere_Piece('Y', mesY, pGlobal);
        Envoi_Changement_Repere_Piece('Z', mesZ, pGlobal);
    }
    else
    {
        print_warning(pGlobal, "Port COM non ouvert !\n");
    }

    gtk_widget_destroy(pGlobal->pWindow);

    return EXIT_SUCCESS;
}


gint CB_Bouton_Annuler(GtkButton* button, gpointer p)
{
    gtk_widget_destroy(p);

    return EXIT_SUCCESS;
}

void CB_ReperePiece(GtkWidget* pWidget, GdkEvent* event, global_t* pGlobal)
{
    GtkWidget* pWindow = NULL;
    GtkWidget* pVBox = NULL;
    GtkWidget* pHBox = NULL;
    GtkWidget* pLabel = NULL;
    GtkWidget* pEntryX = NULL;
    GtkWidget* pEntryY = NULL;
    GtkWidget* pEntryZ = NULL;
    GtkWidget* pBouton = NULL;
    const gchar* sNom = NULL;

    /* Creation de la fenêtre */
    pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 300, 200);
    gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_MOUSE);
    gtk_window_set_title(GTK_WINDOW(pWindow), "Repere piece");
    gtk_window_set_decorated(GTK_WINDOW(pWindow), TRUE);

    /* Creation d'un box verticale */
    pVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(pWindow), pVBox);


    /* Creation de la zone d'entree */
    pLabel = gtk_label_new("Entrer les nouvelles coordonees piece:");
    gtk_box_pack_start(GTK_BOX(pVBox), pLabel, TRUE, TRUE, 0);


    /* Creation d'un box horizontale Axe X*/
    pHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(pVBox), pHBox);

    /* Creation de la zone d'entree */
    pLabel = gtk_label_new("Axe X:");
    gtk_box_pack_start(GTK_BOX(pHBox), pLabel, TRUE, TRUE, 0);
    pEntryX = gtk_entry_new();
    sNom = gtk_label_get_label(GTK_LABEL(pGlobal->pLabelXmm));
    gtk_entry_set_text(GTK_ENTRY(pEntryX), sNom);
    gtk_entry_set_alignment(GTK_ENTRY(pEntryX), 1);
    gtk_box_pack_start(GTK_BOX(pHBox), pEntryX, TRUE, TRUE, 0);
    pLabel = gtk_label_new("mm");
    gtk_box_pack_start(GTK_BOX(pHBox), pLabel, TRUE, TRUE, 0);


    /* Creation d'un box horizontale */
    pHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(pVBox), pHBox);

    /* Creation de la zone d'entree Axe Y */
    pLabel = gtk_label_new("Axe Y:");
    gtk_box_pack_start(GTK_BOX(pHBox), pLabel, TRUE, TRUE, 0);
    pEntryY = gtk_entry_new();
    sNom = gtk_label_get_label(GTK_LABEL(pGlobal->pLabelYmm));
    gtk_entry_set_text(GTK_ENTRY(pEntryY), sNom);
    gtk_entry_set_alignment(GTK_ENTRY(pEntryY), 1);
    gtk_box_pack_start(GTK_BOX(pHBox), pEntryY, TRUE, TRUE, 0);
    pLabel = gtk_label_new(" mm");
    gtk_box_pack_start(GTK_BOX(pHBox), pLabel, TRUE, TRUE, 0);


    /* Creation d'un box horizontale Axe Z */
    pHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(pVBox), pHBox);

    /* Creation de la zone d'entree Axe Z */
    pLabel = gtk_label_new("Axe Z:");
    gtk_box_pack_start(GTK_BOX(pHBox), pLabel, TRUE, TRUE, 0);
    pEntryZ = gtk_entry_new();
    sNom = gtk_label_get_label(GTK_LABEL(pGlobal->pLabelZmm));
    gtk_entry_set_text(GTK_ENTRY(pEntryZ), sNom);
    gtk_entry_set_alignment(GTK_ENTRY(pEntryZ), 1);
    gtk_box_pack_start(GTK_BOX(pHBox), pEntryZ, TRUE, TRUE, 0);
    pLabel = gtk_label_new(" mm");
    gtk_box_pack_start(GTK_BOX(pHBox), pLabel, TRUE, TRUE, 0);


    /* Creation d'un box horizontale */
    pHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(pHBox), TRUE);
    gtk_container_add(GTK_CONTAINER(pVBox), pHBox);

    pBouton = gtk_button_new_with_label("OK");
    gtk_container_set_border_width(GTK_CONTAINER(pBouton), 5);
    pGlobal->pWindow = pWindow;
    pGlobal->pLabelXnew = pEntryX;
    pGlobal->pLabelYnew = pEntryY;
    pGlobal->pLabelZnew = pEntryZ;
    g_signal_connect(G_OBJECT(pBouton), "clicked", G_CALLBACK(CB_Bouton_OK_ReperePiece), pGlobal);
    gtk_box_pack_start(GTK_BOX(pHBox), pBouton, TRUE, TRUE, 0);

    pBouton = gtk_button_new_with_label("Annuler");
    gtk_container_set_border_width(GTK_CONTAINER(pBouton), 5);
    g_signal_connect(G_OBJECT(pBouton), "clicked", G_CALLBACK(CB_Bouton_Annuler), pWindow);
    gtk_box_pack_start(GTK_BOX(pHBox), pBouton, TRUE, TRUE, 0);

    gtk_widget_show_all(pWindow);
}

/*----------------------Callback de réglage de repères fraiseuse de l'interface-----------------------*/
gint CB_Bouton_OK_RepereFraiseuse(GtkButton* button, global_t* pGlobal)
{
    if(pGlobal->comport_open == 0)  //Si le port série est ouvert
    {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pGlobal->pCheckEntryX)))
        {
            Envoi_Reset_Axe_Fraiseuse('X', pGlobal);
            Envoi_Changement_Repere_Piece('X', 0, pGlobal);
        }

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pGlobal->pCheckEntryY)))
        {
            Envoi_Reset_Axe_Fraiseuse('Y', pGlobal);
            Envoi_Changement_Repere_Piece('Y', 0, pGlobal);
        }

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pGlobal->pCheckEntryZ)))
        {
            Envoi_Reset_Axe_Fraiseuse('Z', pGlobal);
            Envoi_Changement_Repere_Piece('Z', 0, pGlobal);
        }
    }
    else
    {
        print_warning(pGlobal, "Port COM non ouvert !\n");
    }

    gtk_widget_destroy(pGlobal->pWindow);

    return EXIT_SUCCESS;
}

void CB_RepereFraiseuse(GtkWidget* pWidget, GdkEvent* event, global_t* pGlobal)
{
    GtkWidget *pWindow = NULL;
    GtkWidget *pVBox = NULL;
    GtkWidget *pHBox = NULL;
    GtkWidget *pLabel = NULL;
    GtkWidget *pCheckEntryX = NULL;
    GtkWidget *pCheckEntryY = NULL;
    GtkWidget *pCheckEntryZ = NULL;
    GtkWidget *pBouton = NULL;

    /* Creation de la fenêtre */
    pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 300, 200);
    gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_MOUSE);
    gtk_window_set_title(GTK_WINDOW(pWindow), "Repere fraiseuse");
    gtk_window_set_decorated(GTK_WINDOW(pWindow), TRUE);

    /* Creation d'un box verticale */
    pVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(pWindow), pVBox);

    /* Creation de la zone de label*/
    pLabel = gtk_label_new("Cocher la case pour reseter l'axe:");
    gtk_box_pack_start(GTK_BOX(pVBox), pLabel, TRUE, TRUE, 0);

    /* Creation de la zone d'entree Axe X */
    pCheckEntryX = gtk_check_button_new_with_label("Reset Axe X");
    gtk_box_pack_start(GTK_BOX(pVBox), pCheckEntryX, TRUE, TRUE, 0);

    /* Creation de la zone d'entree Axe Y */
    pCheckEntryY = gtk_check_button_new_with_label("Reset Axe Y");
    gtk_box_pack_start(GTK_BOX(pVBox), pCheckEntryY, TRUE, TRUE, 0);

    /* Creation de la zone d'entree Axe Z */
    pCheckEntryZ = gtk_check_button_new_with_label("Reset Axe Z");
    gtk_box_pack_start(GTK_BOX(pVBox), pCheckEntryZ, TRUE, TRUE, 0);

    /* Creation d'un box horizontale pour les boutons */
    pHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(pHBox), TRUE);
    gtk_container_add(GTK_CONTAINER(pVBox), pHBox);

    pBouton = gtk_button_new_with_label("OK");
    gtk_container_set_border_width(GTK_CONTAINER(pBouton), 5);
    g_signal_connect(G_OBJECT(pBouton), "clicked", G_CALLBACK(CB_Bouton_OK_RepereFraiseuse), pGlobal);
    gtk_box_pack_start(GTK_BOX(pHBox), pBouton, TRUE, TRUE, 0);

    pBouton = gtk_button_new_with_label("Annuler");
    gtk_container_set_border_width(GTK_CONTAINER(pBouton), 5);
    g_signal_connect(G_OBJECT(pBouton), "clicked", G_CALLBACK(CB_Bouton_Annuler), pWindow);
    gtk_box_pack_start(GTK_BOX(pHBox), pBouton, TRUE, TRUE, 0);

    pGlobal->pWindow = pWindow;
    pGlobal->pCheckEntryX = pCheckEntryX;
    pGlobal->pCheckEntryY = pCheckEntryY;
    pGlobal->pCheckEntryZ = pCheckEntryZ;

    gtk_widget_show_all(pWindow);
}


/*----------------------Callback de la fenêtre de texte de l'interface-----------------------*/
void CB_TextView_Modifie(GtkWidget* pWidget, global_t* pGlobal)
{
    pGlobal->sauve = FALSE;
}
