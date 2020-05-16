#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "main.h"
#include "error.h"
#include "rs232.h"
#include "communication.h"

/*******************************************************
* Fonction Envoi_Ligne_Instruction
*
*Formate la trame d'instruction à envoyer à la fraiseuse
*******************************************************/
void Envoi_Ligne_Instruction(gchar* buffer, int nb_caracteres, global_t* pGlobal)
{
    int i = 0;
    unsigned char tab[100] = "\0";

    tab[0] = 1;

    if(pGlobal->comport_open == 0)  //Si le port série est ouvert
    {
        for(i=0 ; i<nb_caracteres ; i++)    //Comptage des octets un par un
        {
            if((buffer[i] != 0x0A) && (buffer[i] != 0x0D))  //Recherche des caractères de fin de ligne
            {
                tab[i+1] = buffer[i];
                tab[0]++;
            }
        }
        g_mutex_lock(pGlobal->Mutex_EnvoiPortSerie);
        RS232_SendBuf(pGlobal->comport_number, tab, tab[0]);
        g_mutex_unlock(pGlobal->Mutex_EnvoiPortSerie);
    }
    else
    {
        print_warning(pGlobal, "Port COM non ouvert !\n");
    }
}


/*******************************************************
* Fonction Envoi_Deplacement
*
*Formate la trame de déplacement à envoyer à la fraiseuse
*******************************************************/
void Envoi_Deplacement(char axe, char posneg, global_t* pGlobal)
{
    char buffer[50] = "\0";
    float valeurfX = 0;
    float valeurfY = 0;
    float valeurfZ = 0;
    gchar* increment = NULL;
    gchar* vitesse = NULL;
    float incrementf = 0;
    float vitessef = 0;

    g_mutex_lock(pGlobal->Mutex_UpdateLabel);

    valeurfX = pGlobal->Xpiece;
    valeurfY = pGlobal->Ypiece;
    valeurfZ = pGlobal->Zpiece;

    g_mutex_unlock(pGlobal->Mutex_UpdateLabel);

    increment = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(pGlobal->pComboBoxInc));
    if(strcmp(increment, "1 pas") == 0)
    {
        incrementf = RESOLUTION_FRAISEUSE;
    }
    else
    {
        sscanf(increment, "%f", &incrementf);
    }
    free(increment);

    vitesse = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(pGlobal->pComboBoxVit));
    sscanf(vitesse, "%f", &vitessef);
    free(vitesse);

    if((axe == 'X') && (posneg == 1))
    {
        valeurfX += incrementf;
    }
    else if((axe == 'X') && (posneg == 0))
    {
        valeurfX -= incrementf;
    }
    else if((axe == 'Y') && (posneg == 1))
    {
        valeurfY += incrementf;
    }
    else if((axe == 'Y') && (posneg == 0))
    {
        valeurfY -= incrementf;
    }
    else if((axe == 'Z') && (posneg == 1))
    {
        valeurfZ += incrementf;
    }
    else if((axe == 'Z') && (posneg == 0))
    {
        valeurfZ -= incrementf;
    }

    //sprintf(buffer, "G1 F%0.4f X%0.5f Y%0.5f Z%0.5f\n", vitessef, valeurfX, valeurfY, valeurfZ);
    sprintf(buffer, "G1 F%0.4f X%f Y%f Z%f\n", vitessef, valeurfX, valeurfY, valeurfZ);

    printf("Chaine construite : %s\n", buffer);
    Envoi_Ligne_Instruction(buffer, strlen(buffer), pGlobal);
}

/*******************************************************
* Fonction Envoi_Reset_Axe_Fraiseuse
*
*Envoi une commande pour mettre à 0 l'axe correspondant
* de la fraiseuse
*******************************************************/
void Envoi_Reset_Axe_Fraiseuse(char Axe, global_t* pGlobal)
{
    //Envoi des octets 0x00 puis l'axe en ASCII
    unsigned char buffer[3] = "\0";

    buffer[0] = 3;
    buffer[1] = 0;
    buffer[2] = Axe;

    printf("Envoi reset repere fraiseuse\t : %c\n", Axe);

    if(pGlobal->comport_open == 0)  //Si le port série est ouvert
    {
        g_mutex_lock(pGlobal->Mutex_EnvoiPortSerie);
        RS232_SendBuf(pGlobal->comport_number, buffer, buffer[0]);
        g_usleep(100000);
        g_mutex_unlock(pGlobal->Mutex_EnvoiPortSerie);
    }
    else
    {
        print_warning(pGlobal, "Port COM non ouvert !\n");
    }
}

/*******************************************************
* Fonction Envoi_Changement_Repere_Piece
*
*Envoi une commande pour changer les coordonnées pieces
* de la fraiseuse
*******************************************************/
void Envoi_Changement_Repere_Piece(char Axe, float valeur, global_t* pGlobal)
{
    //Envoi des octets 0x01, l'axe en ASCII et la valeur en float
    union             //Utilisé pour découper un float ou un int32 plus facilement
    {
        float u_f;
        char u_o[4];
    } var_type;
    unsigned char buffer[7] = "\0";

    buffer[0] = 7;
    buffer[1] = 0x01;
    buffer[2] = Axe;

    var_type.u_f = valeur;
    buffer[3] = var_type.u_o[0];
    buffer[4] = var_type.u_o[1];
    buffer[5] = var_type.u_o[2];
    buffer[6] = var_type.u_o[3];

    printf("Envoi changement repere piece\t : %c, %f\n", Axe, var_type.u_f);

    if(pGlobal->comport_open == 0)  //Si le port série est ouvert
    {
        g_mutex_lock(pGlobal->Mutex_EnvoiPortSerie);
        RS232_SendBuf(pGlobal->comport_number, buffer, buffer[0]);
        g_usleep(100000);
        g_mutex_unlock(pGlobal->Mutex_EnvoiPortSerie);
    }
    else
    {
        print_warning(pGlobal, "Port COM non ouvert !\n");
    }
}

/*******************************************************
* Fonction Envoi_VitesseProgramme
*
*Envoi une commande pour changer la vitesse d'execution
* du programme par la fraiseuse
*******************************************************/
void Envoi_VitesseProgramme(global_t* pGlobal)
{
    gchar* valeur = NULL;
    float valeurfV = 0;

    //Envoi des octets 0x05 et la valeur en float
    union             //Utilisé pour découper un float ou un int32 plus facilement
    {
        float u_f;
        char u_o[4];
    } var_type;
    unsigned char buffer[7] = "\0";

    buffer[0] = 6;
    buffer[1] = 0x05;

    valeur = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(pGlobal->pComboBoxVitProg));
    sscanf(valeur, "%f", &valeurfV);

    var_type.u_f = valeurfV / (float)100;
    buffer[2] = var_type.u_o[0];
    buffer[3] = var_type.u_o[1];
    buffer[4] = var_type.u_o[2];
    buffer[5] = var_type.u_o[3];

    printf("Envoi Vitesse Programme\t : %0.0f %c\n", var_type.u_f * 100, '%');

    if(pGlobal->comport_open == 0)  //Si le port série est ouvert
    {
        g_mutex_lock(pGlobal->Mutex_EnvoiPortSerie);
        RS232_SendBuf(pGlobal->comport_number, buffer, buffer[0]);
        g_mutex_unlock(pGlobal->Mutex_EnvoiPortSerie);
        g_usleep(500000);   //ajout d'un petit delais pour ne pas envoyer des changements trop rapidement lorsqu'on tourne la molette de la souris
    }
    else
    {
        print_warning(pGlobal, "Port COM non ouvert !\n");
    }
}

/*******************************************************
* Fonction Attente_Connexion
*
*Envoi un caractere 'U' (0x55) pour que la fraiseuse
*détermine elle même la vitesse de connexion
*******************************************************/
int Attente_Connexion(global_t* pGlobal)
{
    unsigned char buf[2000];
    const char* buffer_comp = "Fraisor\n";
    unsigned char caractere = 'U';
    int ret = 0;

    memset(buf, 0, sizeof(buf));

    RS232_flushRX(pGlobal->comport_number);

    g_mutex_lock(pGlobal->Mutex_EnvoiPortSerie);
    RS232_SendBuf(pGlobal->comport_number, &caractere, 1);
    g_mutex_unlock(pGlobal->Mutex_EnvoiPortSerie);

    g_usleep(100*1000);
    RS232_flushRX(pGlobal->comport_number);

    g_usleep(100*1000);
    RS232_PollComport(pGlobal->comport_number, buf, 2000/*sizeof(buffer_comp)+6*/);

    printf("string recue : %s\n", buf);

    if(NULL == strstr((const char*)buf, buffer_comp))
    {
        ret = 0;   //Si connexion échouée
    }
    else
    {
        ret = 1;   //Si connexion réussie
    }

    return ret;
}


/*******************************************************
* Fonction Lit_Octet
*
*Lit un octet dans le buffer de reception serie
*******************************************************/
unsigned char Lit_Octet(global_t* pGlobal)
{
    unsigned char temp = 0;
    unsigned char ret = 0;

    if(pGlobal->comport_open == 0)
    {
        while(RS232_PollComport(pGlobal->comport_number, &temp, 1) <= 0)
        {
            g_usleep(1*1000);
        }

        ret = temp;
    }
    else
    {
        ret  = 0;
    }

    return ret;
}

/*******************************************************
* Fonction Lit_Int32
*
*Lit un entier 32 bits dans le buffer de reception serie
*******************************************************/
unsigned int Lit_Int32(global_t* pGlobal)
{
    union             //Utilisé pour découper un int32 plus facilement
    {
        int u_i;
        unsigned char u_o[4];
    } var_type;
    unsigned int ret = 0;

    if(pGlobal->comport_open == 0)
    {
        var_type.u_o[0] = Lit_Octet(pGlobal);
        var_type.u_o[1] = Lit_Octet(pGlobal);
        var_type.u_o[2] = Lit_Octet(pGlobal);
        var_type.u_o[3] = Lit_Octet(pGlobal);

        ret = var_type.u_i;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

/*******************************************************
* Fonction Lit_Float
*
*Lit un float 32 bits dans le buffer de reception serie
*******************************************************/
float Lit_Float(global_t* pGlobal)
{
    union             //Utilisé pour découper un float plus facilement
    {
        float u_f;
        unsigned char u_o[4];
    } var_type;
    float ret = 0;

    if(pGlobal->comport_open == 0)
    {
        var_type.u_o[0] = Lit_Octet(pGlobal);
        var_type.u_o[1] = Lit_Octet(pGlobal);
        var_type.u_o[2] = Lit_Octet(pGlobal);
        var_type.u_o[3] = Lit_Octet(pGlobal);

        ret = var_type.u_f;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

/*******************************************************
* Fonction Envoi_Reset_Fraiseuse
*
*Envoi une commande pour reseter le dsPIC de la
* fraiseuse
*******************************************************/
void Envoi_Reset_Fraiseuse(global_t* pGlobal)
{

    unsigned char buffer[2] = "\0";

    buffer[0] = 2;
    buffer[1] = 0x02;

    printf("Envoi reset fraiseuse\n");

    if(pGlobal->comport_open == 0)  //Si le port série est ouvert
    {
        g_mutex_lock(pGlobal->Mutex_EnvoiPortSerie);
        RS232_SendBuf(pGlobal->comport_number, buffer, buffer[0]);
        g_mutex_unlock(pGlobal->Mutex_EnvoiPortSerie);
    }
    else
    {
        print_warning(pGlobal, "Port COM non ouvert !\n");
    }
}

/*******************************************************
* Fonction Envoi_Arrêt_Programme
*
*Envoi une commande pour arrêter le programme en cours
*******************************************************/
void Envoi_Arret_Programme(global_t* pGlobal)
{
    //Envoi de l'octet 0x03
    unsigned char buffer[2] = "\0";

    buffer[0] = 2;
    buffer[1] = 3;

    printf("Envoi arret programme\n");

    if(pGlobal->comport_open == 0)  //Si le port série est ouvert
    {
        g_mutex_lock(pGlobal->Mutex_EnvoiPortSerie);
        RS232_SendBuf(pGlobal->comport_number, buffer, buffer[0]);
        g_mutex_unlock(pGlobal->Mutex_EnvoiPortSerie);
    }
    else
    {
        print_warning(pGlobal, "Port COM non ouvert !\n");
    }
}

/*******************************************************
* Fonction Envoi_Pause_Programme
*
*Envoi une commande pour mettre en pause le programme
*en cours
*******************************************************/
void Envoi_Pause_Programme(global_t* pGlobal)
{
    //Envoi de l'octet 0x04
    unsigned char buffer[2] = "\0";

    buffer[0] = 2;
    buffer[1] = 4;

    printf("Envoi pause programme\n");

    if(pGlobal->comport_open == 0)  //Si le port série est ouvert
    {
        g_mutex_lock(pGlobal->Mutex_EnvoiPortSerie);
        RS232_SendBuf(pGlobal->comport_number, buffer, buffer[0]);
        g_mutex_unlock(pGlobal->Mutex_EnvoiPortSerie);
    }
    else
    {
        print_warning(pGlobal, "Port COM non ouvert !\n");
    }
}
