#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <string.h>

#include "main.h"
#include "communication.h"
#include "rs232.h"
#include "callback.h"
#include "error.h"


gboolean Thread_UpdateLabels(gpointer data);
gboolean Thread_Scroll(gpointer data);



gpointer Thread_LectureStop(gpointer data)
{
    global_t* pGlobal = data;
    GtkTextBuffer *buffer = NULL;
    GtkTextIter start;
    GtkTextIter end;
    GtkTextIter iter;
    gchar *buffer_ligne = NULL;
    uint32_t i = 0; //Index de la ligne a envoyer
    uint8_t CTS_Enable = 0;
    enum EtatProgramme Etat;
    char ajout = '\n';  //Caractere saut de ligne
    uint8_t QuitReason = 0;


    printf("Debut de tache LectureStop\n");

    g_mutex_lock(pGlobal->Mutex_UpdateLabel);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(pGlobal->pTextView), FALSE);   //Rend la fenetre de text non modifibale

    //Ajout d'un saut de ligne à la fin du TextView s'il n'y en a pas
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pGlobal->pTextView));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_get_iter_at_line(buffer, &start, gtk_text_buffer_get_line_count(buffer) - 1);
    buffer_ligne = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if(strcmp(buffer_ligne, "") != 0)
    {
        gtk_text_buffer_insert(buffer, &end, &ajout, 1);
    }

    printf("Clique sur Lecture\n");

    pGlobal->nombre_lignes_OK = 0;
    pGlobal->nombre_lignes = gtk_text_buffer_get_line_count(buffer) - 1;
    g_mutex_unlock(pGlobal->Mutex_UpdateLabel);

    g_mutex_lock(pGlobal->Mutex_LectureStop);
    pGlobal->Etat = LECTURE;
    g_mutex_unlock(pGlobal->Mutex_LectureStop);

    gdk_threads_add_idle((GSourceFunc)Thread_Scroll, pGlobal);

    while((pGlobal->quit == 0) && (pGlobal->comport_open == 0) && (QuitReason == 0))
    {
        g_mutex_lock(pGlobal->Mutex_LectureStop);
        Etat = pGlobal->Etat;
        g_mutex_unlock(pGlobal->Mutex_LectureStop);

        switch(Etat)
        {
            case LECTURE:
            case PAUSE:
                CTS_Enable = RS232_IsCTSEnabled(pGlobal->comport_number);
                if(CTS_Enable == 1)
                {
                    g_mutex_lock(pGlobal->Mutex_UpdateLabel);
                    gtk_text_buffer_get_iter_at_line(buffer, &start, i);
                    gtk_text_buffer_get_iter_at_line(buffer, &iter, i+1);
                    buffer_ligne = gtk_text_buffer_get_text(buffer, &start, &iter, FALSE);

                    if((buffer_ligne[0] != '(') && (buffer_ligne[0] != '%') && (buffer_ligne[0] != '#') && (buffer_ligne[0] != ' ') && (buffer_ligne[0] != '\n'))   //Si ce n'est pas une ligne de commentaires ou une ligne vide
                    {
                        if(gtk_text_iter_get_offset(&iter)-gtk_text_iter_get_offset(&start) > 1)   //S'il y a des caractères à envoyer
                        {
                            printf("Ligne %d : start = %d, iter = %d: ", i+1, gtk_text_iter_get_offset(&start), gtk_text_iter_get_offset(&iter));
                            printf("%s\n", buffer_ligne);
                            Envoi_Ligne_Instruction(buffer_ligne, gtk_text_iter_get_offset(&iter)-gtk_text_iter_get_offset(&start), pGlobal);
                            g_mutex_unlock(pGlobal->Mutex_UpdateLabel);
                            g_usleep(10*1000); //On laisse le temps au dsPIC de traiter la ligne pendant 10ms
                        }
                        else    //Sinon, c'est qu'on a atteint la fin du fichier
                        {
                            g_mutex_unlock(pGlobal->Mutex_UpdateLabel);
                            printf("********Fin du programme\n\n");
                            QuitReason = 1;
                        }
                    }
                    else
                    {
                        g_mutex_unlock(pGlobal->Mutex_UpdateLabel);
                    }

                    if(buffer_ligne != NULL)
                    {
                        free(buffer_ligne);
                        buffer_ligne = NULL;
                    }

                    i++;    //Passage à la ligne suivante
                }
                else    //Si CTS == 0, on attends
                {
                    g_usleep(1*1000); //On attends que le dsPIC finisse l'instruction en cours
                }
                break;

            case STOP:
                QuitReason = 2;
                break;

            default:
                printf("Erreur etat du programme\n");
                QuitReason = 3;
                break;
        }
    }

    printf("Fin de tache LectureStop, raison = %u\n", QuitReason);

    return(NULL);
}


gboolean Thread_UpdateLabels(gpointer data)
{
    global_t* pGlobal = data;
    static uint8_t status = 0;


    g_mutex_lock(pGlobal->Mutex_UpdateLabel);

    sprintf(pGlobal->buffer, "%f", pGlobal->Xpiece);
    gtk_label_set_text(GTK_LABEL(pGlobal->pLabelXmm), pGlobal->buffer);

    sprintf(pGlobal->buffer, "%f", pGlobal->Ypiece);
    gtk_label_set_text(GTK_LABEL(pGlobal->pLabelYmm), pGlobal->buffer);

    sprintf(pGlobal->buffer, "%f", pGlobal->Zpiece);
    gtk_label_set_text(GTK_LABEL(pGlobal->pLabelZmm), pGlobal->buffer);

    sprintf(pGlobal->buffer, "%d", pGlobal->Xfraiseuse);
    gtk_label_set_text(GTK_LABEL(pGlobal->pLabelXpas), pGlobal->buffer);

    sprintf(pGlobal->buffer, "%d", pGlobal->Yfraiseuse);
    gtk_label_set_text(GTK_LABEL(pGlobal->pLabelYpas), pGlobal->buffer);

    sprintf(pGlobal->buffer, "%d", pGlobal->Zfraiseuse);
    gtk_label_set_text(GTK_LABEL(pGlobal->pLabelZpas), pGlobal->buffer);


    if((0x02 == (pGlobal->status & 0x02)) && (status != pGlobal->status))
    {
        pGlobal->Etat = STOP;
        print_warning(pGlobal, "%s", "Butee X min atteinte !\n");
    }

    if((0x04 == (pGlobal->status & 0x04)) && (status != pGlobal->status))
    {
        pGlobal->Etat = STOP;
        print_warning(pGlobal, "%s", "Butee X max atteinte !\n");
    }


    if((0x08 == (pGlobal->status & 0x08)) && (status != pGlobal->status))
    {
        pGlobal->Etat = STOP;
        print_warning(pGlobal, "%s", "Butee Y min atteinte !\n");
    }

    if((0x10 == (pGlobal->status & 0x10)) && (status != pGlobal->status))
    {
        pGlobal->Etat = STOP;
        print_warning(pGlobal, "%s", "Butee Y max atteinte !\n");
    }


    if((0x20 == (pGlobal->status & 0x20)) && (status != pGlobal->status))
    {
        pGlobal->Etat = STOP;
        print_warning(pGlobal, "%s", "Butee Z min atteinte !\n");
    }

    if((0x40 == (pGlobal->status & 0x40)) && (status != pGlobal->status))
    {
        pGlobal->Etat = STOP;
        print_warning(pGlobal, "%s", "Butee Z max atteinte !\n");
    }

    if(0x01 == (pGlobal->status & 0x01))    //FRAISEUSE_EN_DEPLACEMENT
    {
        gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemStop), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemPause), TRUE);
    }
    else                                    //FRAISEUSE_STOP
    {
        gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemStop), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemPause), FALSE);
    }

    status = pGlobal->status;

    g_mutex_unlock(pGlobal->Mutex_UpdateLabel);

    return(false);
}


gboolean Thread_Scroll(gpointer data)
{
    global_t* pGlobal = data;
    GtkTextBuffer *buffer = NULL;
    GtkTextIter startExec;
    GtkTextIter endExec;
    GtkTextIter start, end;
    float fracProg = 0;

    //Surligne la ligne programme en cours d'execution
    g_mutex_lock(pGlobal->Mutex_UpdateLabel);

    gtk_text_buffer_get_iter_at_line(pGlobal->pTextBuffer, &start, pGlobal->nombre_lignes_OK);
    gtk_text_buffer_get_iter_at_line(pGlobal->pTextBuffer, &end, pGlobal->nombre_lignes_OK + 1);

    g_mutex_unlock(pGlobal->Mutex_UpdateLabel);

    gtk_text_buffer_apply_tag_by_name(pGlobal->pTextBuffer, "highlight", &start, &end);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(pGlobal->pTextView), &end, 0, TRUE, 0, 0.3);

    if(0x01 != (pGlobal->status & 0x01))
    {
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pGlobal->pTextView));
        gtk_text_buffer_get_bounds(buffer, &startExec, &endExec);
        gtk_text_buffer_remove_all_tags(buffer, &startExec, &endExec);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(pGlobal->pTextView), TRUE);
    }

    //Met à jour la Progress Bar
    g_mutex_lock(pGlobal->Mutex_UpdateLabel);

    if(pGlobal->nombre_lignes == 0)
    {
        fracProg = 0.0;
    }
    else
    {
        fracProg = (float)pGlobal->nombre_lignes_OK / (float)pGlobal->nombre_lignes;
    }

    g_mutex_unlock(pGlobal->Mutex_UpdateLabel);

    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pGlobal->pProgressBar), fracProg);

    return(false);
}


gpointer Thread_Reception(gpointer data)
{
    global_t* pGlobal = data;
    uint16_t taille = 0;
    char temp[10];
    int32_t LigneProg = 0;


    printf("Debut de tache Reception\n");


    while((pGlobal->quit == 0) && (pGlobal->comport_open == 0))
    {
        taille = Lit_Octet(pGlobal);
        if(taille == 0)
        {
            g_usleep(100*1000); //Si on n'a rien reçu, on s'endort
        }
        else if(taille == 0x1D) //On a reçu une trame indiquant les coordonnées de la fraiseuse
        {
            g_mutex_lock(pGlobal->Mutex_UpdateLabel);
            pGlobal->axe1 = Lit_Octet(pGlobal);
            pGlobal->Xfraiseuse = Lit_Int32(pGlobal);
            pGlobal->Xpiece = Lit_Float(pGlobal);
            pGlobal->axe2 = Lit_Octet(pGlobal);
            pGlobal->Yfraiseuse = Lit_Int32(pGlobal);
            pGlobal->Ypiece = Lit_Float(pGlobal);
            pGlobal->axe3 = Lit_Octet(pGlobal);
            pGlobal->Zfraiseuse = Lit_Int32(pGlobal);
            pGlobal->Zpiece = Lit_Float(pGlobal);
            pGlobal->status = Lit_Octet(pGlobal);
            g_mutex_unlock(pGlobal->Mutex_UpdateLabel);

            //On met à jour l'interface
            gdk_threads_add_idle((GSourceFunc)Thread_UpdateLabels, pGlobal);
        }
        else if(taille == 0x05)  //On a reçu une trame indiquant qu'une ligne de programme est terminée
        {
            temp[0] = Lit_Octet(pGlobal);
            temp[1] = Lit_Octet(pGlobal);
            temp[2] = Lit_Octet(pGlobal);
            temp[3] = Lit_Octet(pGlobal);

            if((temp[0] == 'O') && (temp[1]=='K'))
            {
                g_mutex_lock(pGlobal->Mutex_UpdateLabel);
                pGlobal->nombre_lignes_OK++;
                LigneProg = pGlobal->nombre_lignes_OK;
                g_mutex_unlock(pGlobal->Mutex_UpdateLabel);
                printf("Ligne Executee : %d, instruction : %u, %u\n", LigneProg, temp[2], temp[3]);
            }

            //On met à jour l'interface
            if(pGlobal->nombre_lignes_OK >= 0)  //Si ce n'est pas un deplacement manuel
            {
                gdk_threads_add_idle((GSourceFunc)Thread_Scroll, pGlobal);
            }
        }
        else
        {
            Lit_Octet(pGlobal); //Si l'octet ne correspond pas à une taille connue, on dépile le buffer
        }
    }

    printf("Fin de tache Reception\n");

    return(NULL);
}
