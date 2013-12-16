#include "dir_index.h"

int
compare_strings(const char *s1, const char *s2)
{
        return strcasecmp(*s2 == '.' ? s2+1 : s2, *s1 == '.' ? s1+1 : s1);
}

/* Lexicographical comparison */
int
l_compar(const FTSENT **ent1, const FTSENT **ent2)
{
        return -compare_strings((*ent1)->fts_name, (*ent2)->fts_name);
}

void
makeIndex( char *dir, char *index ){
    FTS *ftsp;
    FTSENT *p, *chp;
    int fts_options = FTS_NOCHDIR | FTS_PHYSICAL | FTS_SEEDOT;
    int memleft= MAX_DIR_PAGE - (strlen(HTML_HEAD)+strlen(HTML_TAIL));
    int memlist=0;
    char * paths[2] = {dir, NULL};

    memset(index, 0, MAX_DIR_PAGE);
    memcpy(index,HTML_HEAD,strlen(HTML_HEAD));
    if ((ftsp = fts_open(paths, fts_options, l_compar)) == NULL) {
        fprintf(stderr,"fts_open failed");
        return; /*make sure to catch this error*/
    }

    if (NULL != ftsp) {
        if ((p = fts_read(ftsp)) != NULL) {
            chp = fts_children(ftsp,0);

            while (NULL != chp) {
                memlist= strlen(chp->fts_accpath) + 2*(strlen(chp->fts_name));
                if(memleft > memlist+ANCHOR_LEN){
                    char buffer[1024];
                    *buffer='\0';
                    strcat(buffer,"<li><a href=\'");

                    /* Prepend a '/' if not already present */
                    if (chp->fts_accpath[0] != '/')
                        strcat(buffer, "/");

                    strcat(buffer,chp->fts_accpath);
                    if ((chp->fts_accpath)[strlen(chp->fts_accpath) - 1] != '/')
                        strcat(buffer,"/");

                    strcat(buffer,chp->fts_name);
                    strcat(buffer,"\'>");
                    strcat(buffer,chp->fts_name);
                    strcat(buffer,"</a>");
                    memleft -= memlist+ANCHOR_LEN;
                    strncat(index,buffer,strlen(buffer));
                    *buffer='\0';
                }

                (void)fts_set(ftsp, p, FTS_SKIP);
                chp = chp->fts_link;
            }
        }
    }
    fts_close(ftsp);
    strcat(index,HTML_TAIL);
    return;
}
