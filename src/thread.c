#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>

#include "main.h"
#include "thread.h"
#include "communication.h"
#include "rs232.h"
#include "callback.h"
#include "error.h"


void SurlignageProg(global_t* pGlobal);


void Thread_LectureStop(global_t* pGlobal)
{
    //GError *err = NULL;
    GtkTextBuffer *buffer = NULL;
    GtkTextIter start;
    GtkTextIter iter;
    gchar *buffer_ligne = NULL;
    uint32_t i = 0; //Index de la ligne a envoyer
    //uint32_t nombre_ligne = 0;
    uint8_t CTS_Enable = 0;
    enum EtatProgramme Etat;


    while(pGlobal->quit == 0)
    {
        g_mutex_lock(pGlobal->Mutex_LectureStop);
        Etat = pGlobal->Etat;
        g_mutex_unlock(pGlobal->Mutex_LectureStop);

        switch(Etat)
        {
            case LECTURE:
            case PAUSE:
                //printf("Etat CTS : %d\n", IsCTSEnabled(pGlobal->comport_number));
                CTS_Enable = RS232_IsCTSEnabled(pGlobal->comport_number);
                if((pGlobal->comport_open == 0) && (CTS_Enable == 1))
                {
                    if(pGlobal->nombre_ligne == 0)   //Détermine une seule fois le nombre de lignes du TextView
                    {
                        gtk_text_view_set_editable(GTK_TEXT_VIEW(pGlobal->pTextView), FALSE);
                        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pGlobal->pTextView));
                        pGlobal->nombre_ligne = gtk_text_buffer_get_line_count(buffer);
                        gtk_text_buffer_get_bounds(buffer, &start, &iter);
                        i = 0;
                        g_mutex_lock(pGlobal->Mutex_UpdateLabel);
                        pGlobal->LigneProg = 0;
                        g_mutex_unlock(pGlobal->Mutex_UpdateLabel);
                    }

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
                            g_usleep(10*1000); //On laisse le temps au dsPIC de traiter la ligne pendant 20ms
                        }
                        else    //Sinon, c'est qu'on a atteint la fin du fichier
                        {
                            g_mutex_unlock(pGlobal->Mutex_UpdateLabel);
                            printf("********Fin du programme\n\n");
                            return;
                        }
                    }
                    else
                    {
                        g_mutex_unlock(pGlobal->Mutex_UpdateLabel);
                    }
                    free(buffer_ligne);
                    buffer_ligne = NULL;
                    i++;    //Passage à la ligne suivante
                }
                else if((pGlobal->comport_open == 0) && (CTS_Enable == 0))
                {
                    g_usleep(1*1000); //On attends que le dsPIC finisse l'instruction en cours
                }
                else    //if(pGlobal->comport_open == 1)
                {
                    pGlobal->Etat = STOP;
                    printf("Port COM non ouvert !\n");
                    return;
                }
                break;

            case STOP:
                return;
                break;

            default:
                printf("Erreur etat du programme\n");
                return;
                break;
        }
    }
}





void UpdateLabels(global_t* pGlobal)
{
    static uint8_t status = 0;
    GtkTextBuffer *buffer = NULL;
    uint32_t LigneProg = 0;
    uint32_t OldLigneProg = 0;
    GtkTextIter startExec;
    GtkTextIter endExec;


    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pGlobal->pTextView));

    if(gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer), "highlight") == NULL)
    {
        gtk_text_buffer_create_tag(buffer, "highlight", "weight", PANGO_WEIGHT_BOLD, "foreground", "blue", NULL);
    }

    while(pGlobal->quit == 0)
    {
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


        if((0x01 == (pGlobal->status & 0x01)) && (status != pGlobal->status))
        {
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemStop), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemPause), TRUE);
        }
        if((0x00 == (pGlobal->status & 0x01)) && (status != pGlobal->status))
        {
            pGlobal->Etat = STOP;
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemStop), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemPause), FALSE);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(pGlobal->pTextView), TRUE);
        }


        if((0x02 == (pGlobal->status & 0x02)) && (status != pGlobal->status))
        {
            pGlobal->Etat = STOP;
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemStop), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemPause), FALSE);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(pGlobal->pTextView), TRUE);
            print_warning(pGlobal, "%s", "Butee X min atteinte !\n");
        }

        if((0x04 == (pGlobal->status & 0x04)) && (status != pGlobal->status))
        {
            pGlobal->Etat = STOP;
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemStop), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemPause), FALSE);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(pGlobal->pTextView), TRUE);
            print_warning(pGlobal, "%s", "Butee X max atteinte !\n");
        }


        if((0x08 == (pGlobal->status & 0x08)) && (status != pGlobal->status))
        {
            pGlobal->Etat = STOP;
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemStop), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemPause), FALSE);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(pGlobal->pTextView), TRUE);
            print_warning(pGlobal, "%s", "Butee Y min atteinte !\n");
        }

        if((0x10 == (pGlobal->status & 0x10)) && (status != pGlobal->status))
        {
            pGlobal->Etat = STOP;
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemStop), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemPause), FALSE);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(pGlobal->pTextView), TRUE);
            print_warning(pGlobal, "%s", "Butee Y max atteinte !\n");
        }


        if((0x20 == (pGlobal->status & 0x20)) && (status != pGlobal->status))
        {
            pGlobal->Etat = STOP;
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemStop), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemPause), FALSE);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(pGlobal->pTextView), TRUE);
            print_warning(pGlobal, "%s", "Butee Z min atteinte !\n");
        }

        if((0x40 == (pGlobal->status & 0x40)) && (status != pGlobal->status))
        {
            pGlobal->Etat = STOP;
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemLecture), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemStop), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(pGlobal->pToolItemPause), FALSE);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(pGlobal->pTextView), TRUE);
            print_warning(pGlobal, "%s", "Butee Z max atteinte !\n");
        }

        status = pGlobal->status;



            //Surlignage de la ligne en cours d'execution
        if(pGlobal->Etat == LECTURE)
        {
            LigneProg = pGlobal->LigneProg;

            if(LigneProg != OldLigneProg)
            {
                gtk_text_buffer_get_iter_at_line(buffer, &startExec, OldLigneProg);
                gtk_text_buffer_get_iter_at_line(buffer, &endExec, LigneProg + 1);

                OldLigneProg = LigneProg;
                //buffer_ligne = gtk_text_buffer_get_text(buffer, &startExec, &endExec, FALSE);
                //printf(".....Application du Tag2 sur ligne %u :%s\n", LigneProg, buffer_ligne);

                //gtk_text_buffer_apply_tag_by_name(buffer, "highlight", &startExec, &endExec);

                if((LigneProg % 8) == 0)
                {
                    //gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(pGlobal->pTextView), &endExec, 0, TRUE, 0, 0.3);
                }
            }
        }
        if(pGlobal->Etat == STOP)
        {
            gtk_text_buffer_get_bounds(buffer, &startExec, &endExec);
            gtk_text_buffer_remove_all_tags(buffer, &startExec, &endExec);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(pGlobal->pTextView), TRUE);
            OldLigneProg = 1;
        }



        g_mutex_unlock(pGlobal->Mutex_UpdateLabel);

        while(gtk_events_pending())
        {
            gtk_main_iteration();
        }

        g_usleep(10*1000);
    }
}

void Thread_Reception(global_t* pGlobal)
{
    uint16_t taille = 0;
    char temp[10];
    uint32_t LigneProg = 0;

    gdk_threads_add_idle((GSourceFunc)UpdateLabels, pGlobal);

    while(pGlobal->quit == 0)
    {
        if(pGlobal->comport_open == 0)  //Si le port série est ouvert
        {
            //printf("Lecture Serie\n");
            taille = Lit_Octet(pGlobal);
            if(taille == 0)
            {
                g_usleep(0.1*1000); //Si on n'a rien reçu, on s'endort
            }
            else if(taille == 0x1D)
            {
                /*printf("\nTaille recue = %x\n", taille);
                printf("Reception donnees de positionnement : \n");
                */
                //g_usleep(1*1000);
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
            }
            else if(taille == 0x03)  //Surligne la ligne en cours d'execution dans la textbox
            {
                //g_usleep(1*1000);
                temp[0] = Lit_Octet(pGlobal);
                temp[1] = Lit_Octet(pGlobal);
                temp[2] = '\n';

                if((temp[0] == 'O') && (temp[1]=='K'))
                {
                    g_mutex_lock(pGlobal->Mutex_UpdateLabel);
                    pGlobal->LigneProg++;
                    LigneProg = pGlobal->LigneProg;
                    g_mutex_unlock(pGlobal->Mutex_UpdateLabel);
                    printf("Ligne Executee : %u\n", LigneProg);
                }
            }
            else
            {
                //g_usleep(1*1000);
                Lit_Octet(pGlobal); //Si l'octet ne correspond pas à une taille connue, on dépile le buffer
            }
        }
        else    //si port série non ouvert
        {
            g_usleep(100*1000); //Le port n'est pas ouvert, on s'endort
        }
    }

    //On attend la fin de la tache UpdateLabels
    g_usleep(10*1000);
}


//void Thread_Scroll(global_t* pGlobal)
//{
//    //Essai surlignage dans textbox
//    GtkTextBuffer *p_text_buffer;
//    GtkTextIter start, end;
//    GtkTextTag *tag1, *tag2;
//
//    p_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pGlobal->pTextView));
//    gtk_text_buffer_set_text (p_text_buffer, "H\ne\nl\nl\no\n,\n \nt\nh\ni\ns\n \ni\ns\n \ns\no\nm\ne\n \nt\ne\nx\nt\n \nS\nt\ne\np\nh\na\nn\ne", -1);
//    tag1 = gtk_text_buffer_create_tag (p_text_buffer, "highlight", "weight", PANGO_WEIGHT_BOLD, "foreground", "blue", NULL);
//    tag2 = gtk_text_buffer_create_tag (p_text_buffer, "normal", "weight", PANGO_WEIGHT_BOLD, "foreground", "black", NULL);
//    gtk_text_buffer_get_iter_at_offset (p_text_buffer, &start, 7);
//    gtk_text_buffer_get_iter_at_offset (p_text_buffer, &end, 12);
//    gtk_text_buffer_apply_tag (p_text_buffer, tag1, &start, &end);
//    gtk_text_buffer_get_iter_at_offset (p_text_buffer, &start, 13);
//    gtk_text_buffer_get_iter_at_offset (p_text_buffer, &end, 20);
//    gtk_text_buffer_apply_tag (p_text_buffer, tag2, &start, &end);
//    p_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pGlobal->pTextView));
//    gtk_text_buffer_get_line_count(p_text_buffer);
//    //gtk_text_buffer_get_iter_at_offset (p_text_buffer, &end, 31);
//    gtk_text_buffer_get_iter_at_line(p_text_buffer, &end, 16);
//    g_usleep(2000*1000); //Le port n'est pas ouvert, on s'endort
//    //gtk_text_buffer_place_cursor (p_text_buffer, &end);
//    gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW(pGlobal->pTextView), &end, 0, TRUE, 0, 0.3);
//}

void Lance_Thread(volatile global_t* pGlobal)
{
    GError *err = NULL;

//    if( (pGlobal->Thread1 = g_thread_try_new("Thread_LectureStop", (GThreadFunc)Thread_LectureStop, (global_t *)pGlobal, &err)) == NULL)
//    {
//        g_error("Thread LectureStop create failed: %s!!\n", err->message );
//        g_error_free(err);
//    }

    if( (pGlobal->Thread2 = g_thread_try_new("Thread Reception", (GThreadFunc)Thread_Reception, (global_t *)pGlobal, &err)) == NULL)
    {
        g_error("Thread Reception create failed: %s!!\n", err->message );
        g_error_free(err);
    }


//    if( (pGlobal->Thread4 = g_thread_try_new("Thread Scroll", (GThreadFunc)Thread_Scroll, (global_t *)pGlobal, &err)) == NULL)
//    {
//        g_error("Thread Scroll create failed: %s!!\n", err->message );
//        g_error_free(err);
//    }
}

