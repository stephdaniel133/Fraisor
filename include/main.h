#ifndef __MAIN_H
#define __MAIN_H

#include <stdint.h>
#include <gtk/gtk.h>


//#define __windows__
//#define __linux__

#define GTK_THEME_FILE "temprc.txt"

enum EtatProgramme
{
    STOP,
    LECTURE,
    PAUSE
};

typedef struct
{
    GtkWidget *pMainWindow;
    GtkWidget *pStatusBar;
    GtkWidget *pTextView;
    gchar     *chemin;
    gboolean  sauve;
    //Repere Piece et Fraiseuse
    GtkWidget *pLabelXmm;
    GtkWidget *pLabelYmm;
    GtkWidget *pLabelZmm;
    GtkWidget *pLabelXpas;
    GtkWidget *pLabelYpas;
    GtkWidget *pLabelZpas;
    GtkWidget *pComboBoxInc;
    GtkWidget *pComboBoxVit;
    GtkWidget *pComboBoxVitProg;
    //Barre d'outils
    GtkToolItem *pToolItemConnecter;
    GtkToolItem *pToolItemDeconnecter;
    GtkToolItem *pToolItemLecture;
    GtkToolItem *pToolItemStop;
    GtkToolItem *pToolItemPause;
    //Gestion port série
    int         comport_number;
    gboolean    comport_open;
    //Gestion des Threads
    GThread     *Thread1;
    GThread     *Thread2;
    GThread     *Thread3;
    //GThread     *Thread4;
    enum        EtatProgramme Etat;
    GMutex      *Mutex_LectureStop;
    GMutex      *Mutex_EnvoiPortSerie;
    uint8_t     quit;

    GtkWidget *pWindow;
    GtkWidget *pLabelXnew;
    GtkWidget *pLabelYnew;
    GtkWidget *pLabelZnew;
    GtkWidget *pCheckEntryX;
    GtkWidget *pCheckEntryY;
    GtkWidget *pCheckEntryZ;


    GMutex      *Mutex_UpdateLabel;
    char axe1;
    uint32_t Xfraiseuse;
    float Xpiece;
    char axe2;
    uint32_t Yfraiseuse;
    float Ypiece;
    char axe3;
    uint32_t Zfraiseuse;
    float Zpiece;
    uint8_t status;
    char buffer[10];
    uint32_t nombre_ligne;
    uint32_t LigneProg;
} global_t;



#endif  //__MAIN_H
