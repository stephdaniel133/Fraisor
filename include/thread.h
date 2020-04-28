#ifndef __THREAD_H
#define __THREAD_H

/*union var_type             //Utilisé pour découper un float ou un int32 plus facilement
{
    float u_f;
    int u_i;
    char u_o[4];
};
*/


struct buffer_position
{
    char axe1;
    unsigned int Xfraiseuse;
    float Xpiece;
    char axe2;
    unsigned int Yfraiseuse;
    float Ypiece;
    char axe3;
    unsigned int Zfraiseuse;
    float Zpiece;
};


void Thread_LectureStop(global_t* pGlobal);
void Thread_Scroll(global_t* pGlobal);
void Lance_Thread(volatile global_t *pGlobal);

#endif //__THREAD_H
