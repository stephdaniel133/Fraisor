#include <stdlib.h>
#include <gtk/gtk.h>

#include "main.h"
#include "callback.h"
#include "error.h"
#include "rs232.h"
#include "thread.h"


//Variables globales

//Prototypes



int main(int argc, char **argv)
{
    volatile global_t global;

    /* Variables */
    static GtkWidget* pVBox             = NULL;
    static GtkWidget* pHBox             = NULL;

    static GtkWidget* pBarreMenu        = NULL;
    static GtkWidget* pMenu             = NULL;
    static GtkWidget* pMenuItem         = NULL;
    int    comport;
    char   nomportcom[50];
    static GSList*    pList;

    static GtkWidget* pToolBar          = NULL;
    static GtkToolItem* pToolItem       = NULL;

    static GtkWidget* pGrid             = NULL;
    static GtkWidget* pBoutonXp         = NULL;
    static GtkWidget* pBoutonXm         = NULL;
    static GtkWidget* pBoutonYp         = NULL;
    static GtkWidget* pBoutonYm         = NULL;
    static GtkWidget* pBoutonZp         = NULL;
    static GtkWidget* pBoutonZm         = NULL;

    static GtkWidget* pVBox1            = NULL;
    static GtkWidget* pVBox2            = NULL;
    static GtkWidget* pHBox1            = NULL;
    static GtkWidget* pLabel            = NULL;
    static GtkWidget* pEventBox1        = NULL;
    static GtkWidget* pRepPiece         = NULL;
    static GdkCursor* pHandCursor       = NULL;
    static GtkWidget* pEventBox2        = NULL;
    static GtkWidget* pRepFraiseuse     = NULL;


    static GtkTextBuffer* pTextBuffer   = NULL;
    static GtkWidget* pScrolledWindow   = NULL;

    static GtkWidget* pStatusBar        = NULL;

    gchar* sUtf8             = NULL;
    static char mode[]={'8', 'N', '1', 0};

//--------------------------------------------------------------------------------------------------------
    memset((void*)&global, 0, sizeof(global_t));
    global.sauve        = TRUE;
    global.comport_open = 1;
    global.comport_number = -1; //Port COM non valide pour forcer la selection avant la connexion
    global.Etat         = STOP;



    // Initialisation de GTK+
    gtk_init(&argc, &argv);



    //Creation des tailles de la police des labels
    PangoAttrList *attrlist;
    PangoAttribute *attr;
    PangoFontDescription *df;

    attrlist = pango_attr_list_new();

    df = pango_font_description_new();
    pango_font_description_set_family(df, "Liberation Mono"); //police a chasse fixe
    pango_font_description_set_size(df, 20 * PANGO_SCALE);

    attr = pango_attr_font_desc_new(df);
    pango_attr_list_insert(attrlist, attr);



    // Création de la fenêtre
    global.pMainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(global.pMainWindow), "delete-event", G_CALLBACK(CB_Delete_Event), (void*)&global);
    gtk_window_set_decorated(GTK_WINDOW(global.pMainWindow), TRUE);
    gtk_window_set_title(GTK_WINDOW(global.pMainWindow), "Fraizor");
    gtk_window_set_default_size(GTK_WINDOW(global.pMainWindow), 750, 700);
    gtk_window_set_position(GTK_WINDOW(global.pMainWindow), GTK_WIN_POS_CENTER);

    // Creation de la box verticale
    pVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(global.pMainWindow), pVBox);


    // Création de la barre de status
    pStatusBar = gtk_statusbar_new();

    // Création du menu
    pBarreMenu = gtk_menu_bar_new();
    pMenu = gtk_menu_new();

    //Menu Fichier
    pMenuItem = gtk_menu_item_new_with_mnemonic("Nouveau Fichier");
    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(CB_Menu_Nouveau), (void*)&global);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_mnemonic("_Ouvrir");
    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(CB_Menu_Ouvrir), (void*)&global);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_mnemonic("_Enregistrer");
    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(CB_Menu_Sauvegarder), (void*)&global);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_mnemonic("E_nregistrer sous...");
    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(CB_Menu_SauvegarderSous), (void*)&global);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_mnemonic("_Quitter");
    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(CB_Menu_Quitter), (void*)&global);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_mnemonic("_Fichier");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(pBarreMenu), pMenuItem);




    // Menu Port Serie
    pMenu = gtk_menu_new();

    sUtf8 = g_locale_to_utf8("Non selectionne", -1, NULL, NULL, NULL);
    pMenuItem = gtk_radio_menu_item_new_with_label(NULL, sUtf8);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
    pList = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(pMenuItem));
    g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(CB_OnRadio), (void*)&global);

    for(comport = 0; comport < RS232_PORTNR ; comport++)  //Recherche des ports serie présents
    {
        strcpy(nomportcom, RS232_GetPortName(comport));
        printf("\nTentative d'ouverture du port %s : ", nomportcom);
        if(!RS232_OpenComport(comport, 115200, mode, 0))
        {
            RS232_CloseComport(comport);
            printf("OK\n");
            pMenuItem = gtk_radio_menu_item_new_with_label(pList, nomportcom);
            gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
            pList = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(pMenuItem));
            g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(CB_OnRadio), (void*)&global);
        }
        else
        {
            printf("NOK\n");
        }
    }

    pMenuItem = gtk_menu_item_new_with_mnemonic("_Port Serie");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(pBarreMenu), pMenuItem);



    // Menu A propos
    pMenu = gtk_menu_new();

    pMenuItem = gtk_menu_item_new_with_mnemonic("A Propos");
    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(CB_APropos), (void*)&global);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_mnemonic("_?");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(pBarreMenu), pMenuItem);

    /* Intégration du menu dans le fenêtre */
    gtk_box_pack_start(GTK_BOX(pVBox), pBarreMenu, FALSE, FALSE, 0);



    //--------------------------------------------------------------------------------------------------
    // Creation de la boite à outil
    pToolBar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(pToolBar), GTK_TOOLBAR_ICONS);
    gtk_box_pack_start (GTK_BOX(pVBox), GTK_WIDGET(pToolBar), FALSE, FALSE, 0);

    pToolItem = gtk_tool_button_new(gtk_image_new_from_icon_name("document-new", GTK_ICON_SIZE_SMALL_TOOLBAR), "Nouveau");
    g_signal_connect(G_OBJECT(pToolItem), "clicked", G_CALLBACK(CB_Menu_Nouveau), (void*)&global);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolBar), pToolItem, -1);

    pToolItem = gtk_tool_button_new(gtk_image_new_from_icon_name("document-open", GTK_ICON_SIZE_SMALL_TOOLBAR), "Nouveau");
    g_signal_connect(G_OBJECT(pToolItem), "clicked", G_CALLBACK(CB_Menu_Ouvrir), (void*)&global);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolBar), pToolItem, -1);

    pToolItem = gtk_tool_button_new(gtk_image_new_from_icon_name("document-save", GTK_ICON_SIZE_SMALL_TOOLBAR), "Nouveau");
    g_signal_connect(G_OBJECT(pToolItem), "clicked", G_CALLBACK(CB_Menu_Sauvegarder), (void*)&global);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolBar), pToolItem, -1);

    pToolItem = gtk_tool_button_new(gtk_image_new_from_icon_name("document-save-as", GTK_ICON_SIZE_SMALL_TOOLBAR), "Nouveau");
    g_signal_connect(G_OBJECT(pToolItem), "clicked", G_CALLBACK(CB_Menu_SauvegarderSous), (void*)&global);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolBar), pToolItem, -1);

    pToolItem = gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(pToolItem), FALSE);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolBar), pToolItem, -1);

    global.pToolItemConnecter = gtk_tool_button_new(gtk_image_new_from_icon_name("gtk-connect", GTK_ICON_SIZE_SMALL_TOOLBAR), "Connecter");
    g_signal_connect(G_OBJECT(global.pToolItemConnecter), "clicked", G_CALLBACK(CB_Connecter), (void*)&global);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolBar), global.pToolItemConnecter, -1);

    global.pToolItemDeconnecter = gtk_tool_button_new(gtk_image_new_from_icon_name("gtk-disconnect", GTK_ICON_SIZE_SMALL_TOOLBAR), "Deconnecter");
    gtk_widget_set_sensitive(GTK_WIDGET(global.pToolItemDeconnecter), FALSE);
    g_signal_connect(G_OBJECT(global.pToolItemDeconnecter), "clicked", G_CALLBACK(CB_Deconnecter), (void*)&global);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolBar), global.pToolItemDeconnecter, -1);

    pToolItem = gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(pToolItem), FALSE);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolBar), pToolItem, -1);

    global.pToolItemLecture = gtk_tool_button_new(gtk_image_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_SMALL_TOOLBAR), "Lire");
    g_signal_connect(G_OBJECT(global.pToolItemLecture), "clicked", G_CALLBACK(CB_Lecture), (void*)&global);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolBar), global.pToolItemLecture, -1);

    global.pToolItemStop = gtk_tool_button_new(gtk_image_new_from_icon_name("media-playback-stop", GTK_ICON_SIZE_SMALL_TOOLBAR), "Stop");
    gtk_widget_set_sensitive(GTK_WIDGET(global.pToolItemStop), FALSE);
    g_signal_connect(G_OBJECT(global.pToolItemStop), "clicked", G_CALLBACK(CB_Stop), (void*)&global);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolBar), global.pToolItemStop, -1);

    global.pToolItemPause = gtk_tool_button_new(gtk_image_new_from_icon_name("media-playback-pause", GTK_ICON_SIZE_SMALL_TOOLBAR), "Pause");
    gtk_widget_set_sensitive(GTK_WIDGET(global.pToolItemPause), FALSE);
    g_signal_connect(G_OBJECT(global.pToolItemPause), "clicked", G_CALLBACK(CB_Pause), (void*)&global);
    gtk_toolbar_insert(GTK_TOOLBAR(pToolBar), global.pToolItemPause, -1);


    //--------------------------------------------------------------------------------------------------
    // Creation de la box horizontale : Boutons fraiseuse, reperes piece et fraiseuse
    pHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pHBox, TRUE, TRUE, 0);
    gtk_box_set_homogeneous(GTK_BOX(pHBox), TRUE);

    // Création de la grid de placement des boutons
    pGrid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(pHBox), pGrid, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(pGrid), 5);

    gtk_widget_set_halign(pGrid, GTK_ALIGN_FILL);
    //gtk_widget_set_valign(pGrid, GTK_ALIGN_FILL);
    gtk_grid_set_row_spacing(GTK_GRID(pGrid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(pGrid), 5);

    // Création des boutons
    pBoutonXp = gtk_button_new_with_label("X+");
    pLabel = gtk_bin_get_child(GTK_BIN(pBoutonXp));
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_widget_set_hexpand(pBoutonXp, TRUE);
    gtk_widget_set_vexpand(pBoutonXp, TRUE);
    gtk_grid_attach(GTK_GRID(pGrid), pBoutonXp, 2, 1, 1, 1);
    g_signal_connect(G_OBJECT(pBoutonXp), "clicked", G_CALLBACK(CB_Bouton_Xp_Click), (void*)&global);
    g_signal_connect(G_OBJECT(pBoutonXp), "enter", G_CALLBACK(CB_Bouton_X_Affiche_Status), (GtkWidget*)pStatusBar);
    g_signal_connect(G_OBJECT(pBoutonXp), "leave", G_CALLBACK(CB_Bouton_X_Efface_Status), (GtkWidget*)pStatusBar);

    pBoutonXm = gtk_button_new_with_label("X-");
    pLabel = gtk_bin_get_child(GTK_BIN(pBoutonXm));
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_widget_set_hexpand(pBoutonXm, TRUE);
    gtk_widget_set_vexpand(pBoutonXm, TRUE);
    gtk_grid_attach(GTK_GRID(pGrid), pBoutonXm, 0, 1, 1, 1);
    g_signal_connect(G_OBJECT(pBoutonXm), "clicked", G_CALLBACK(CB_Bouton_Xm_Click), (void*)&global);
    g_signal_connect(G_OBJECT(pBoutonXm), "enter", G_CALLBACK(CB_Bouton_X_Affiche_Status), (GtkWidget*)pStatusBar);
    g_signal_connect(G_OBJECT(pBoutonXm), "leave", G_CALLBACK(CB_Bouton_X_Efface_Status), (GtkWidget*)pStatusBar);

    pBoutonYp = gtk_button_new_with_label("Y+");
    pLabel = gtk_bin_get_child(GTK_BIN(pBoutonYp));
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_widget_set_hexpand(pBoutonYp, TRUE);
    gtk_widget_set_vexpand(pBoutonYp, TRUE);
    gtk_grid_attach(GTK_GRID(pGrid), pBoutonYp, 1, 0, 1, 1);
    g_signal_connect(G_OBJECT(pBoutonYp), "clicked", G_CALLBACK(CB_Bouton_Yp_Click), (void*)&global);
    g_signal_connect(G_OBJECT(pBoutonYp), "enter", G_CALLBACK(CB_Bouton_Y_Affiche_Status), (GtkWidget*)pStatusBar);
    g_signal_connect(G_OBJECT(pBoutonYp), "leave", G_CALLBACK(CB_Bouton_Y_Efface_Status), (GtkWidget*)pStatusBar);

    pBoutonYm = gtk_button_new_with_label("Y-");
    pLabel = gtk_bin_get_child(GTK_BIN(pBoutonYm));
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_widget_set_hexpand(pBoutonYm, TRUE);
    gtk_widget_set_vexpand(pBoutonYm, TRUE);
    gtk_grid_attach(GTK_GRID(pGrid), pBoutonYm, 1, 2, 1, 1);
    g_signal_connect(G_OBJECT(pBoutonYm), "clicked", G_CALLBACK(CB_Bouton_Ym_Click), (void*)&global);
    g_signal_connect(G_OBJECT(pBoutonYm), "enter", G_CALLBACK(CB_Bouton_Y_Affiche_Status), (GtkWidget*)pStatusBar);
    g_signal_connect(G_OBJECT(pBoutonYm), "leave", G_CALLBACK(CB_Bouton_Y_Efface_Status), (GtkWidget*)pStatusBar);

    pBoutonZp = gtk_button_new_with_label("Z+");
    pLabel = gtk_bin_get_child(GTK_BIN(pBoutonZp));
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_widget_set_hexpand(pBoutonZp, TRUE);
    gtk_widget_set_vexpand(pBoutonZp, TRUE);
    gtk_grid_attach(GTK_GRID(pGrid), pBoutonZp, 4, 0, 1, 1);
    g_signal_connect(G_OBJECT(pBoutonZp), "clicked", G_CALLBACK(CB_Bouton_Zp_Click), (void*)&global);
    g_signal_connect(G_OBJECT(pBoutonZp), "enter", G_CALLBACK(CB_Bouton_Z_Affiche_Status), (GtkWidget*)pStatusBar);
    g_signal_connect(G_OBJECT(pBoutonZp), "leave", G_CALLBACK(CB_Bouton_Z_Efface_Status), (GtkWidget*)pStatusBar);

    pBoutonZm = gtk_button_new_with_label("Z-");
    pLabel = gtk_bin_get_child(GTK_BIN(pBoutonZm));
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_widget_set_hexpand(pBoutonZm, TRUE);
    gtk_widget_set_vexpand(pBoutonZm, TRUE);
    gtk_grid_attach(GTK_GRID(pGrid), pBoutonZm, 4, 2, 1, 1);
    g_signal_connect(G_OBJECT(pBoutonZm), "clicked", G_CALLBACK(CB_Bouton_Zm_Click), (void*)&global);
    g_signal_connect(G_OBJECT(pBoutonZm), "enter", G_CALLBACK(CB_Bouton_Z_Affiche_Status), (GtkWidget*)pStatusBar);
    g_signal_connect(G_OBJECT(pBoutonZm), "leave", G_CALLBACK(CB_Bouton_Z_Efface_Status), (GtkWidget*)pStatusBar);

    // Création de la ComboBox Déplacements
    global.pComboBoxInc = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxInc), "1 pas");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxInc), "0,1 mm");
    gtk_combo_box_set_active(GTK_COMBO_BOX(global.pComboBoxInc), 1);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxInc), "1 mm");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxInc), "5 mm");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxInc), "10 mm");

    gtk_grid_attach(GTK_GRID(pGrid), global.pComboBoxInc, 0, 3, 5, 1);



    // Création de la ComboBox Vitesses
    global.pComboBoxVit = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVit), "10 mm/min");
    gtk_combo_box_set_active(GTK_COMBO_BOX(global.pComboBoxVit), 0);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVit), "50 mm/min");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVit), "100 mm/min");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVit), "200 mm/min");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVit), "300 mm/min");

    gtk_grid_attach(GTK_GRID(pGrid), global.pComboBoxVit, 0, 4, 5, 1);

    // Création de la ComboBox Vitesses Programme
    global.pComboBoxVitProg = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVitProg), "100 %");
    gtk_combo_box_set_active(GTK_COMBO_BOX(global.pComboBoxVitProg), 0);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVitProg), "90 %");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVitProg), "80 %");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVitProg), "70 %");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVitProg), "60 %");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVitProg), "50 %");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVitProg), "40 %");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVitProg), "30 %");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVitProg), "20 %");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(global.pComboBoxVitProg), "10 %");
    g_signal_connect(G_OBJECT(global.pComboBoxVitProg), "changed", G_CALLBACK(ComboBoxVitProg_on_changed), (void*)&global);

    gtk_grid_attach(GTK_GRID(pGrid), global.pComboBoxVitProg, 0, 5, 5, 1);

    // Creation de la box verticale des 2 repères
    pVBox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(pHBox), pVBox1, TRUE, TRUE, 0);
    gtk_box_set_homogeneous(GTK_BOX(pVBox1), TRUE);

    // Création des labels Affichage du repère pièce
    pEventBox1 = gtk_event_box_new();
    gtk_box_pack_start(GTK_BOX(pVBox1), pEventBox1, TRUE, TRUE, 0);
    gtk_widget_show(pEventBox1);
    g_signal_connect(pEventBox1, "button-press-event", G_CALLBACK(CB_ReperePiece), (void*)&global);
    gtk_widget_realize(pEventBox1);
    pHandCursor = gdk_cursor_new_for_display(gdk_screen_get_display(gdk_screen_get_default()), GDK_HAND2);
    gdk_window_set_cursor(gtk_widget_get_window(pEventBox1), pHandCursor);


    pRepPiece = gtk_frame_new("Repere Piece");
    gtk_frame_set_label_align(GTK_FRAME(pRepPiece), 0.5, 0.5);
    gtk_container_set_border_width(GTK_CONTAINER(pRepPiece), 5);
    gtk_container_add(GTK_CONTAINER(pEventBox1), pRepPiece);

    pVBox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(pRepPiece), pVBox2);

    pHBox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pVBox2), pHBox1, TRUE, TRUE, 0);
    pLabel = gtk_label_new(" X =");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_start(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    pLabel = gtk_label_new("   mm ");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    global.pLabelXmm = gtk_label_new("0,00");
    gtk_label_set_attributes(GTK_LABEL(global.pLabelXmm), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), global.pLabelXmm, FALSE, FALSE, 0);

    pHBox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pVBox2), pHBox1, TRUE, TRUE, 0);
    pLabel = gtk_label_new(" Y =");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_start(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    pLabel = gtk_label_new("   mm ");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    global.pLabelYmm = gtk_label_new("0,00");
    gtk_label_set_attributes(GTK_LABEL(global.pLabelYmm), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), global.pLabelYmm, FALSE, FALSE, 0);

    pHBox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pVBox2), pHBox1, TRUE, TRUE, 0);
    pLabel = gtk_label_new(" Z =");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_start(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    pLabel = gtk_label_new("   mm ");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    global.pLabelZmm = gtk_label_new("0,00");
    gtk_label_set_attributes(GTK_LABEL(global.pLabelZmm), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), global.pLabelZmm, FALSE, FALSE, 0);


    // Création des labels Affichage du repère fraiseuse
    pEventBox2 = gtk_event_box_new();
    gtk_box_pack_start(GTK_BOX(pVBox1), pEventBox2, TRUE, TRUE, 0);
    gtk_widget_show(pEventBox2);
    g_signal_connect(pEventBox2, "button-press-event", G_CALLBACK(CB_RepereFraiseuse), (void*)&global);
    gtk_widget_realize(pEventBox2);
    gdk_window_set_cursor(gtk_widget_get_window(pEventBox2), pHandCursor);

    pRepFraiseuse = gtk_frame_new("Repere Fraiseuse");
    gtk_frame_set_label_align(GTK_FRAME(pRepFraiseuse), 0.5, 0.5);
    gtk_container_set_border_width(GTK_CONTAINER(pRepFraiseuse), 5);
    gtk_container_add(GTK_CONTAINER(pEventBox2), pRepFraiseuse);

    pVBox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(pRepFraiseuse), pVBox2);

    pHBox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pVBox2), pHBox1, TRUE, TRUE, 0);
    pLabel = gtk_label_new(" X =");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_start(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    pLabel = gtk_label_new("     pas ");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    global.pLabelXpas = gtk_label_new("0");
    gtk_label_set_attributes(GTK_LABEL(global.pLabelXpas), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), global.pLabelXpas, FALSE, FALSE, 0);

    pHBox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pVBox2), pHBox1, TRUE, TRUE, 0);
    pLabel = gtk_label_new(" Y =");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_start(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    pLabel = gtk_label_new("     pas ");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    global.pLabelYpas = gtk_label_new("0");
    gtk_label_set_attributes(GTK_LABEL(global.pLabelYpas), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), global.pLabelYpas, FALSE, FALSE, 0);

    pHBox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pVBox2), pHBox1, TRUE, TRUE, 0);
    pLabel = gtk_label_new(" Z =");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_start(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    pLabel = gtk_label_new("     pas ");
    gtk_label_set_attributes(GTK_LABEL(pLabel), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), pLabel, FALSE, FALSE, 0);
    global.pLabelZpas = gtk_label_new("0");
    gtk_label_set_attributes(GTK_LABEL(global.pLabelZpas), attrlist);
    gtk_box_pack_end(GTK_BOX(pHBox1), global.pLabelZpas, FALSE, FALSE, 0);


    // Creation de la zone de texte
    global.pTextView = gtk_text_view_new();
    pTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(global.pTextView));
    gtk_text_buffer_set_text(pTextBuffer, "G0 X0 Y0 Z0", -1);
    GtkCssProvider *provider;
    GtkStyleContext *context;
    provider = gtk_css_provider_new();
    //https://developer.gnome.org/gtk3/stable/chap-css-properties.html Table 3
    const gchar *textformat = "textview { font: 18px Liberation Mono; } text { color: black; }";
    gtk_css_provider_load_from_data(provider, textformat, -1, NULL);
    context = gtk_widget_get_style_context(global.pTextView);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Insertion de la zone de text dans la fenetre
    pScrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(pScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_set_border_width(GTK_CONTAINER(global.pTextView), 5);
    gtk_container_add(GTK_CONTAINER(pScrolledWindow), global.pTextView);
    gtk_box_pack_start(GTK_BOX(pVBox), pScrolledWindow, TRUE, TRUE, 0);

    // Insertion dans la fenetre de la barre de status
    gtk_box_pack_end(GTK_BOX(pVBox), pStatusBar, FALSE, FALSE, 0);



    global.Mutex_LectureStop = g_new(GMutex, 1);
    g_mutex_init(global.Mutex_LectureStop);

    global.Mutex_EnvoiPortSerie = g_new(GMutex, 1);
    g_mutex_init(global.Mutex_EnvoiPortSerie);

    global.Mutex_UpdateLabel = g_new(GMutex, 1);
    g_mutex_init(global.Mutex_UpdateLabel);

    //Lancement des différents threads
    Lance_Thread(&global);



    // Affichage et boucle évènementielle
    gtk_widget_show_all(global.pMainWindow);
    gtk_main();
    printf("gtk_main ...  fin\n");

    //g_thread_join(global.Thread1);
    //g_thread_join(global.Thread2);

    pango_attr_list_unref(attrlist);

    //On attend la fin de toutes les tâches
    g_usleep(100*1000);


    return EXIT_SUCCESS;
}
