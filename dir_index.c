#include "dir_index.h"

void
makeIndex( char *dir, char *index ){
    FTS *ftsp;
    FTSENT *p, *chp;
    int fts_options = FTS_NOCHDIR | FTS_PHYSICAL;
    int memleft= MAX_DIR_PAGE - (strlen(HTML_HEAD)+strlen(HTML_TAIL));
    int memlist=0;
    char * paths[2] = {dir, NULL};

    memcpy(index,HTML_HEAD,strlen(HTML_HEAD));
    if ((ftsp = fts_open(paths, fts_options, NULL)) == NULL) {
            fprintf(stderr,"fts_open failed");
            return; /*make sure to catch this error*/
        }
    chp = fts_children(ftsp, 0);
        if (chp == NULL) {
            return;               /* no files to traverse */
        }
    if (NULL != ftsp) {
        if ((p = fts_read(ftsp)) != NULL) {
            chp = fts_children(ftsp,0);

            if (chp->fts_errno != 0) {
               // fprintf(stderr,"fts_children");
            }
            while (NULL != chp) {
                memlist= strlen(chp->fts_accpath) + 2*(strlen(chp->fts_name));
                if(memleft > memlist+ANCHOR_LEN){
                    char buffer[1024];
                    *buffer='\0';
                    strcat(buffer,"<a href=\'/");
                    strcat(buffer,chp->fts_accpath);
                    if ((chp->fts_accpath)[strlen(chp->fts_accpath) - 1] != '/')
                        strcat(buffer,"/");
                    strcat(buffer,chp->fts_name);
                    strcat(buffer,"\'>");
                    strcat(buffer,chp->fts_name);
                    strcat(buffer,"</a><br />");
                    memleft -= memlist+ANCHOR_LEN;
                   strncat(index,buffer,strlen(buffer));
                   *buffer='\0';
                }
                if (chp != NULL)
                    (void)fts_set(ftsp, p, FTS_SKIP);

                chp = chp->fts_link;
            }
        }
    }
    fts_close(ftsp);
    strcat(index,HTML_TAIL);
    return;
}
