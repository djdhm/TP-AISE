#ifndef MYMALLOC_LIBRARY_H
#define MYMALLOC_LIBRARY_H


#define  TAILLE_PAGE_SYSTEME  sysconf(_SC_PAGE_SIZE)

#define ALIGNEMENT 8
#define ALIGNER(taille) (taille + (ALIGNEMENT - 1)) & -ALIGNEMENT  // aligner l$



typedef struct block
{
    struct block *precedent;
    struct block *suivant;
    int libre;
    size_t taille;
} block_header;

typedef struct page
{
    struct page *precedente;
    struct page *suivante;
    struct block *debut ;

    /*
    ** si   page->taille==page->debut->taille donc la page est vide
    */
    size_t taille;
} page_header;
static const size_t TAILL_ENTETE_PAGE = sizeof(struct page);
static const size_t TAILLE_ENTETE_BLOC = sizeof(struct block);


#endif //MYMALLOC_LIBRARY_H
