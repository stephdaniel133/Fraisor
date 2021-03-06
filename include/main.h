#ifndef __MAIN_H
#define __MAIN_H

#include <stdint.h>
#include <gtk/gtk.h>


//#define __windows__
//#define __linux__

#define GTK_THEME_FILE "temprc.txt"

#define RESOLUTION_FRAISEUSE    (float)1.0/72.0
#define BUTEE_MAX_X             (int32_t)14320
#define BUTEE_MAX_Y             (int32_t)7160
#define BUTEE_MAX_Z             (int32_t)14320


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
    GtkTextBuffer *pTextBuffer;
    GtkWidget *pProgressBar;
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
    //Gestion port s�rie
    int         comport_number;
    gboolean    comport_open;
    //Gestion des Threads
    GThread     *pThreadLectureStop;
    GThread     *pThreadReception;

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


    GMutex    *Mutex_UpdateLabel;
    uint32_t Xfraiseuse;
    float Xpiece;
    uint32_t Yfraiseuse;
    float Ypiece;
    uint32_t Zfraiseuse;
    float Zpiece;
    uint8_t status;
    char ThreadReceptionStatut;
    char buffer[20];
    int32_t nombre_lignes;
    int32_t nombre_lignes_OK;
} global_t;



#endif  //__MAIN_H
