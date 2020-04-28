#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H



void Envoi_Ligne_Instruction(gchar* buffer, int nb_caracteres, global_t* pGlobal);
void Envoi_Deplacement(char axe, char posneg, global_t* pGlobal);
void Envoi_Reset_Axe_Fraiseuse(char Axe, global_t* pGlobal);
void Envoi_Changement_Repere_Piece(char Axe, float valeur, global_t* pGlobal);
void Envoi_VitesseProgramme(global_t* pGlobal);
int Attente_Connexion(global_t* pGlobal);

unsigned char Lit_Octet(global_t* pGlobal);
unsigned int Lit_Int32(global_t* pGlobal);
float Lit_Float(global_t* pGlobal);
void Envoi_Reset_Fraiseuse(global_t* pGlobal);
void Envoi_Arret_Programme(global_t* pGlobal);
void Envoi_Pause_Programme(global_t* pGlobal);

#endif //__COMMUNICATION_H
