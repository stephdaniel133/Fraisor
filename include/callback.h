#ifndef __CALLBACK_H
#define __CALLBACK_H


//Prototypes
gboolean CB_Menu_Nouveau_Status(GtkWidget* widget, GdkEventCrossing* event, gpointer user_data);
void CB_Menu_Nouveau(GtkWidget* widget, global_t* pGlobal);
gboolean CB_Menu_Ouvrir_Status(GtkWidget* widget, GdkEventCrossing* event, gpointer user_data);
void CB_Menu_Ouvrir(GtkWidget* widget, global_t* pGlobal);
gboolean CB_Menu_Sauvegarder_Status(GtkWidget* widget, GdkEventCrossing* event, gpointer user_data);
void CB_Menu_Sauvegarder(GtkWidget* widget, global_t* pGlobal);
gboolean CB_Menu_SauvegarderSous_Status(GtkWidget* widget, GdkEventCrossing* event, gpointer user_data);
void CB_Menu_SauvegarderSous(GtkWidget* widget, global_t* pGlobal);
gboolean CB_Menu_Quitter_Status(GtkWidget* widget, GdkEventCrossing* event, gpointer user_data);
gint CB_Delete_Event(GtkWidget* widget, GdkEvent* event, global_t* pGlobal);
void CB_Menu_Quitter(GtkWidget* widget, global_t* pGlobal);

void CB_OnRadio(GtkWidget* widget, global_t* pGlobal);

void CB_APropos(GtkWidget* widget, global_t* pGlobal);


void CB_Connecter(GtkWidget* widget, global_t* pGlobal);
void CB_Deconnecter(GtkWidget* widget, global_t* pGlobal);
void CB_Lecture(GtkWidget* widget, global_t* pGlobal);
void CB_Stop(GtkWidget* widget, global_t* pGlobal);
void CB_Pause(GtkWidget* pWidget, global_t* pGlobal);


void CB_Bouton_Xp_Click(GtkWidget* widget, global_t* pGlobal);
void CB_Bouton_Xm_Click(GtkWidget* widget, global_t* pGlobal);
void CB_Bouton_X_Affiche_Status(GtkWidget* widget, gpointer iContextId);
void CB_Bouton_X_Efface_Status(GtkWidget* widget, gpointer iContextId);
void CB_Bouton_Yp_Click(GtkWidget* widget, global_t* pGlobal);
void CB_Bouton_Ym_Click(GtkWidget* widget, global_t* pGlobal);
void CB_Bouton_Y_Affiche_Status(GtkWidget* widget, gpointer iContextId);
void CB_Bouton_Y_Efface_Status(GtkWidget* widget, gpointer iContextId);
void CB_Bouton_Zp_Click(GtkWidget* widget, global_t* pGlobal);
void CB_Bouton_Zm_Click(GtkWidget* widget, global_t* pGlobal);
void CB_Bouton_Z_Affiche_Status(GtkWidget* widget, gpointer iContextId);
void CB_Bouton_Z_Efface_Status(GtkWidget* widget, gpointer iContextId);

void ComboBoxVitProg_on_changed(GtkWidget* pWidget, global_t* pGlobal);

void CB_ReperePiece(GtkWidget* widget, GdkEvent* event, global_t* pGlobal);
void CB_RepereFraiseuse(GtkWidget* widget, GdkEvent* event, global_t* pGlobal);


void CB_TextView_Modifie(GtkWidget* widget, global_t* pGlobal);


#endif  //__CALLBACK_H
