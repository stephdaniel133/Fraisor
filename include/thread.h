#ifndef __THREAD_H
#define __THREAD_H

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


gboolean Thread_LectureStop(gpointer data);
gboolean Thread_Reception(gpointer data);
gboolean Thread_UpdateLabels(gpointer data);

#endif //__THREAD_H
