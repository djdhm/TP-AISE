#define _DEFAULT_SOURCE


#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "library.h"
#include <string.h>
#include <pthread.h>

// Contient la premiere page
static page_header *debutPage = NULL;
static page_header * page_petite = NULL;
static page_header * page_moyenne = NULL;
static page_header * page_large = NULL;
static page_header * page_extra_large = NULL;

static nombreMapping = 0;
static nombreUnMapping = 0;

void __attribute__((constructor)) calledFirst();

void __attribute__((destructor)) calledLast();
void calledFirst(){
    printf("I am initiating memory\m");
}
void calledLast(){
    printf("I have finished \n");
    printf("Nombre d'allocation memoire = %d\n",nombreMapping);
    printf("nombre de liberation memoire = %d\n",nombreUnMapping);
}

//page = page header
void *allouerPage(page_header *pagePrecedente, size_t taille)
{
    size_t tailleNouvellePage = TAILLE_PAGE_SYSTEME;
    if (taille >= TAILLE_PAGE_SYSTEME + TAILL_ENTETE_PAGE + TAILLE_ENTETE_BLOC) tailleNouvellePage = (taille / tailleNouvellePage + 1) * tailleNouvellePage;

    // Demander a l'os Une nouvelle page memoire avec la taille calculer
    void* nouvellePage =  mmap(NULL, tailleNouvellePage, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // En cas d'erreur de mappings de memoire on return null
    if (nouvellePage == NULL)
    {
        perror("Erreur de mapping memoire\n");
        return NULL;
    }

    // Initialisation de premier block de la nouvelle page allouee
    block_header *block = (char*)(nouvellePage) + TAILL_ENTETE_PAGE;
    block->libre = 1;
    block->taille = tailleNouvellePage - TAILLE_ENTETE_BLOC - TAILL_ENTETE_PAGE;
    block->precedent = NULL;
    block->suivant = NULL;

    //


    // Ajout de la nouvelle page a l'ensemble des pages deja alloues
    page_header *page = nouvellePage;
    page->precedente = pagePrecedente;
    page->suivante = NULL;
    page->debut = block;
    page->taille = block->taille;

    //

    // Si la page est la premiere a etre alloue par notre bib
    // on l'affecte a la variable globale debutPage
    if (pagePrecedente) pagePrecedente->suivante = page;
    else debutPage = page;
    nombreMapping++;

    return page;
}

// Decouper un block de memoire en deux pour optimiser l'utilisation de la memoire
// et eviter les fragmentations
void decouperBlock(block_header *block, size_t taille)
{
    // Adresse de nouveau block
    block_header *nouveauBlock = (char*)(block) + (taille + TAILLE_ENTETE_BLOC);

    nouveauBlock->precedent = block;
    nouveauBlock->suivant = block->suivant;
    if (nouveauBlock->suivant)
        nouveauBlock->suivant->precedent = nouveauBlock;
    // Nouveau bloc libre et taille = tailleAncien- tailleUtilise-tailleEntete
    nouveauBlock->libre = 1;
    nouveauBlock->taille = block->taille - TAILLE_ENTETE_BLOC - taille;
    block->suivant = nouveauBlock;
    block->taille = taille;
}


__attribute__ (( __visibility__("default")))
void *malloc(size_t taille)
{


    if (taille == 0)
        return NULL;
    // Si le
    else if (debutPage==NULL){
        debutPage= allouerPage(NULL,taille);
        if(debutPage==NULL){
            return NULL;
        }
    }
    page_header * page= debutPage;




    // Finding libre block with enough space
    page_header *pagePrecedente = NULL;
    block_header *block = NULL;
    int trouve = 0;
    // Recherche d un block libre avec une taille >= taille demande
    while (page && !trouve)
    {
        block = page->debut;
        while (block && (block->libre != 1 || block->taille < taille))
            block = block->suivant;
        // Si aucun block n a ete trouve on passe a la prochaine page
        if (block == NULL)
        {
            pagePrecedente = page;
            page = page->suivante;

        }
        // Sinon on arrete la boucle
        else
            trouve = 1;
    }
    // Si tout les espace memoire ne satisfait pas la condition on alloue une nouvelle page memoire
    if (page == NULL)
    {
        page = allouerPage(pagePrecedente, taille);
        // Si le mapping n a pas reussi on return null
        if (page == NULL)
        {
            return NULL;
        }
        // Sinon on retourne le block debut de la page alloue
        block = page->debut;
    }

    block->libre = 0;

    // Si le bloc alloue un plus que la moitie libre
    // on le decoupe en deux block
    // pour eviter la fragmentation en petits morceaux inutile et assurer l'utilisation de la mem le plus possible
    if (block->taille > 2 * taille + TAILLE_ENTETE_BLOC) decouperBlock(block, taille);


    return block + 1;
}

void verifierVide()
{
    page_header *page = debutPage;
    page_header *temp = NULL;
    while (page)
    {
        temp = page;
        page = page->suivante;
        if (temp->debut->libre == 1 && temp->taille == temp->debut->taille)
        {
            if (temp->precedente)
            {
                if (temp->suivante)
                {
                    temp->suivante->precedente = temp->precedente;
                }
                temp->precedente->suivante = temp->suivante;
            }
            else if (temp->suivante)
            {
                debutPage = temp->suivante;
                debutPage->precedente = NULL;
            }
            if (temp->precedente || temp->suivante)
            {
                nombreUnMapping++;
                munmap(temp, temp->taille + TAILLE_ENTETE_BLOC + TAILL_ENTETE_PAGE);
            }
        }
    }
}

__attribute__ (( __visibility__("default")))
void free(void *ptr)
{
    if (ptr == NULL)
        return ;
    block_header *m = (char *)(ptr)-TAILLE_ENTETE_BLOC;
    if (m->libre != 0)
        return ;
    if (m->precedent && m->precedent->libre == 1)
    {
        m->libre = -1;
        if (m->suivant && m->suivant->libre == 1)
        {
            m->suivant->libre = -1;
            m->precedent->taille += m->taille + m->suivant->taille + 2 * TAILLE_ENTETE_BLOC;
            if (m->suivant->suivant)
            {
                m->precedent->suivant = m->suivant->suivant;
                m->suivant->suivant->precedent = m->precedent;
            }
            else
            {
                m->precedent->suivant = NULL;
            }
        }
        else
        {
            m->precedent->taille += m->taille + TAILLE_ENTETE_BLOC;
            m->precedent->suivant = m->suivant;
            if (m->suivant)
                m->suivant->precedent = m->precedent;
        }
    }
    else if (m->suivant && m->suivant->libre == 1)
    {
        m->libre = 1;
        m->suivant->libre = -1;
        m->taille += m->suivant->taille + TAILLE_ENTETE_BLOC;
        m->suivant = m->suivant->suivant;
        if (m->suivant)
            m->suivant->precedent = m;
    }
    else
    {
        m->libre = 1;
    }
    verifierVide();
}

__attribute__ (( __visibility__("default")))
void *calloc(size_t nombreElements, size_t tailleElement)
{
    size_t s = nombreElements * tailleElement;
    char *ptr = malloc(s);
    for (size_t i = 0; i < s; i++)
    {
        ptr[i] = 0;
    }
    return ptr;
}

__attribute__ (( __visibility__("default")))
void *realloc(void *pointeur, size_t taille)
{
    if (pointeur == NULL)
        return malloc(taille);
    else if (taille == 0)
    {
        free(pointeur);
        return NULL;
    }
    block_header *block = (char*)pointeur - TAILLE_ENTETE_BLOC;
    if (block->taille == taille)
    {
        return pointeur;
    }
    // Si on peut etendre la taille de la zone allouee au sein du mm block
    // Pas besoin de refaire des allocation on modifie seulement la taille
    else if (block->taille > taille + TAILLE_ENTETE_BLOC)
    {

        // Si la moitie de bloc sera non utilise on decoupe ce bloc pour gagner un nouveau espace mem a utiliser
        if(block->taille > 2 * taille + TAILLE_ENTETE_BLOC) decouperBlock(block, taille);
        return pointeur;
    }
    // Expansion de la taille de pointeur
    block_header *blockSuivant = block->suivant;
    if (blockSuivant && blockSuivant->libre == 1 && blockSuivant->taille >= TAILLE_ENTETE_BLOC
        && block->taille + block->suivant->taille + TAILLE_ENTETE_BLOC > taille)
    {
        decouperBlock(blockSuivant, taille - block->taille - TAILLE_ENTETE_BLOC);
        block->suivant = blockSuivant->suivant;
        blockSuivant->suivant->precedent = block;
        block->taille = taille;
        return pointeur;
    }
    // Reallocation d un nouveau pointeur pour la nouvelle taille demande
    char *nouveauPointeur = malloc(taille);
    if (!nouveauPointeur)
        return pointeur;
    // Recopier les donnees  dans le nouveau pointeur
    char *char_ptr = pointeur;
    memcpy(nouveauPointeur,pointeur,block->taille);

    // Liberer l ancien block
    free(pointeur);
    return nouveauPointeur;
}