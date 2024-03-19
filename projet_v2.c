#include <time.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <math.h> 
#include <MLV/MLV_all.h>

// Define 

#define height 1000
#define width 1200
#define MAXPOINT 1000000 

// Structure 

typedef struct  {
    double x ;
    double y ;
}  Point ;


typedef struct _vrtx_{
Point* s; /* un point de l’ensemble */
struct _vrtx_ *prev; /*le vertex précédent*/
struct _vrtx_ *next; /*le vertex suivant*/
} Vertex, *Polygon;


typedef struct{
Polygon pol; /* le polygône */
int curlen; /* la longueur courante */
int maxlen; /* la longueur maximale */
} ConvexHull;


typedef struct listenv{
    ConvexHull C;
    struct listenv *next;
} enveloppe,*listeconvex;

// Fonctions 

/**
    *@brief alloue une enveloppe et l'initialise 
    *
    *@return enveloppe*

*/
enveloppe* alloueEnveloppe(void){
    enveloppe* nouv = (enveloppe*) malloc(sizeof(enveloppe)); // allocation d'une nouvelle enveloppe 
    if(nouv == NULL)
        return NULL; // On renvoie Null si l'allocation se passe mal 
    // initialisation de l'enveloppe
    nouv->next = NULL;
    nouv->C.curlen = 0;
    nouv->C.maxlen = 0;
    nouv->C.pol = NULL;
    return nouv;
}



/**
    *@brief alloue un Vertex (un nouveau sommet) et lui affecte le Point 'p'.
    *
    *@param Point * p 
    *@return Vertex * 

*/
Vertex* alloueVertex(Point* p){
    if (p == NULL) 
        return NULL;
    Vertex* V = (Vertex*) malloc(sizeof(Vertex)); // allocation d'un nouveau sommet 
    if(V == NULL) 
        return NULL; // On renvoie Null si l'allocation se passe mal 
    // affectation du point 'p' dans le vertex 'V' et le renvoie de ce dernier 
    V->s = p;
    V->next = NULL;
    V->prev = NULL;
    return V;
}


/**
    *@brief insere un nouveau sommet dans la structure polygone et renvoie 1,
            renvoie 0 si erreur dans l'allocation.
    *
    *@param Point * p 
    *@param Polygon* pol
    *@return int 1 ou 0 

*/
int inserePolygon(Polygon* pol, Point* p){
    Vertex* V = alloueVertex(p);
    if(V == NULL){
        printf("echec de l'allocation d'un Vertex");
        return 0;
    }
    if(*pol == NULL){ // insertion du point 'p' dans un polygone 'pol' qui ne contient aucun sommet
        *pol = V;
        (*pol)->next = *pol;
        (*pol)->prev = *pol;
        return 1;
    }
    // insertion du point 'p' dans un polygone 'pol' qui contient un sommet ou plus
    V->next = (*pol)->next;
    (*pol)->next->prev = V;
    (*pol)->next = V;
    V->prev = *pol;
    *pol = V;
    return 1;
}


/**
    *@brief calcl et renvoie le produit vectoriel des deux vecteur AB et BC 
    *
    *@param Point A 
    *@param Point B
    *@param Point C 
    *
    *@return double 

*/
double ProduitVectoriel(Point A, Point B, Point C){
    double AB_x =  B.x - A.x;
    double AB_y =  B.y - A.y;
    double AC_x =  C.x - A.x;
    double AC_y =  C.y - A.y;
    return (AB_x * AC_y) - (AC_x * AB_y);
}

/**
    *@brief renvoie le point 'p' avec une perturbation afin d'eviter
            un comportement non desire a l'affichage 
    *
    *@param int x 
    *@param int y 
    *
    *@return Point

*/
Point get_point_on_clic(int x, int y){
    double PERTUB = 0.0001/RAND_MAX;
    Point P;
    P.x = x+(rand()%2?+1.:-1.)*PERTUB*rand();
    P.y = y+(rand()%2?+1.:-1.)*PERTUB*rand();
    return P;
}



/**
    *@brief Dessine la liste des enveloppe et leur sommets 
    *
    *@param listeconvex l

*/
void dessineConvex(listeconvex l){
    int color[6][3] = {{0, 0, 255}, {255, 0, 0}, {255, 255, 0}, {0, 255, 0}, {165, 42, 42}, {160, 32, 240}};
    int tabx[1000], taby[1000], cpt=0;

    for(; l; l = l->next){
        if(l->C.curlen < 3){
            for(int i = 0; i < l->C.curlen; ++i){
                // dessin des sommets des enveloppes ayant moins de 3 sommets 
                MLV_draw_filled_circle(l->C.pol->s->x, l->C.pol->s->y, 2, MLV_COLOR_RED);
                l->C.pol = l->C.pol->next;
            }    
        }
        else{
            for(int i = 0; i < l->C.curlen; ++i){
                tabx[i] = l->C.pol->s->x;
                taby[i] = l->C.pol->s->y;
                // dessin des sommets des enveloppes ayant plus de 3 sommets 
                MLV_draw_filled_circle(l->C.pol->s->x, l->C.pol->s->y, 2, MLV_rgba(color[cpt%6][0], color[cpt%6][1], color[cpt%6][2], 255));
                l->C.pol = l->C.pol->next;
            }
            // dessine les enveloppes sous forme de polygone remplie
            MLV_draw_filled_polygon(tabx, taby, l->C.curlen, MLV_rgba(color[cpt%6][0], color[cpt%6][1], color[cpt%6][2], 50));
            
        }
        cpt++;
    }
}


/**
    *@brief test si le point 'p' est en dehors de l'enveloppe
            renvoie 1 apres insertion de ce dernier comme nouveau sommet,
            sinon le point est a l'interieur de l'enveloppe et on renvoie 0 
    *
    *@param ConvexHull* C
    *@param Point* p
    *
    *@return int 1 ou 0 

*/
int testdirect(ConvexHull* C, Point* p){
    for(int i = 0; i < C->curlen; ++i){
        if(ProduitVectoriel(*p, *(C->pol->s), *(C->pol->next->s)) < 0.){
            // cas ou le point est a l'exterieur de l'enveloppe 
            inserePolygon(&(C->pol), p);
            C->curlen++;
            if(C->curlen > C->maxlen)
                C->maxlen = C->curlen;
            return 1;
        }
        C->pol = C->pol->next;
    }
    return 0;
}


void nettoyage(listeconvex* l);


/**
    *@brief teste si le point est a l interieur d une enveloppe , 
            si c est les cas avec la recursivité elle teste pour l enveloppe d apres . 
            sinon elle l ajoute et fait le nettoyage
    *
    *@param listeconvex *l
    *@param Point *p 

*/
void direct(listeconvex *l, Point *p){
    if(!testdirect(&((*l)->C), p)){
        if((*l)->next == NULL){
            (*l)->next = alloueEnveloppe();
        }
        if((*l)->next->C.curlen < 3){
            inserePolygon(&((*l)->next->C.pol), p);
            (*l)->next->C.curlen++;
            return;
        }
        if((*l)->next->C.curlen == 3){
            if(ProduitVectoriel(*((*l)->next->C.pol->s), *((*l)->next->C.pol->next->s), *((*l)->next->C.pol->next->next->s)) < 0.){
                Polygon tmp = (*l)->next->C.pol->next;
                (*l)->next->C.pol->next = (*l)->next->C.pol->next->next;
                (*l)->next->C.pol->prev = tmp;
                (*l)->next->C.pol->next->prev = (*l)->next->C.pol;
                (*l)->next->C.pol->next->next = tmp;
                tmp->next = (*l)->next->C.pol;
                tmp->prev = (*l)->next->C.pol->next;
            }
        }
        direct(&((*l)->next), p);
    }
    nettoyage(l);
}

/**
    *@brief nettoyage des enveloppe convex qui constituent 'l'
            avant et arriere .
    *
    *@param listeconvex* l

*/
void nettoyage(listeconvex* l){
    Polygon tmp = NULL;

     // nettoyage de l'enveloppe convex vers l'avant 
    while(ProduitVectoriel(*((*l)->C.pol->s),*((*l)->C.pol->next->s),*((*l)->C.pol->next->next->s)) < 0.){
        tmp = (*l)->C.pol->next;
        (*l)->C.pol->next = (*l)->C.pol->next->next;
        (*l)->C.pol->next->prev = (*l)->C.pol;
        (*l)->C.curlen--;
        direct(l, tmp->s); 
        free(tmp);
    }

    // nettoyage de l'enveloppe convex vers l'arriere 
    while(ProduitVectoriel(*((*l)->C.pol->s),*((*l)->C.pol->prev->prev->s),*((*l)->C.pol->prev->s)) < 0.){
        tmp = (*l)->C.pol->prev;
        (*l)->C.pol->prev = (*l)->C.pol->prev->prev;
        (*l)->C.pol->prev->next = (*l)->C.pol;
        (*l)->C.curlen--;
        direct(l, tmp->s);
        free(tmp);
    }
}


/**
    *@brief libere la memoire occuper par les sommets du polygone 'p' 
            (free tout les Vertex qui constituent le polygone passee en parametre )
    *
    *@param Polygon * p

*/
void libere(Polygon* p){
    if(*p == NULL)
        return;
    Polygon next;
    for(Polygon tmp = (*p)->next; tmp != *p; tmp = next){
        next = tmp->next;
        free(tmp);
    }
    free(*p);
    *p = NULL;
}

/**
    *@brief libere la memoire occuper par les enveloppe de la listeconvex 'l' 
            (free tout les enveloppe convex de l  )
    *
    *@param listeconvex* l

*/
void libereListC(listeconvex* l){
    listeconvex supp = NULL;
    while((supp = *l)){ 
        libere(&((*l)->C.pol));
        *l = (*l)->next;
        free(supp);
    }
}




/**
    *@brief Initialise le Polygone avec les trois premier point dans l'ordre 
            formant un triangle dans le sens droit (le sens des aiguilles d'une montre)
    *
    *@param ConvexHull* convex
    *@param  Point* tab

*/
void initEnveloppe(listeconvex* l, Point* tab){
    if(ProduitVectoriel(tab[0], tab[1], tab[2]) >= 0.){
        inserePolygon(&((*l)->C.pol), &tab[0]);
        inserePolygon(&((*l)->C.pol), &tab[1]);
        inserePolygon(&((*l)->C.pol), &tab[2]);
    }
    else{
    inserePolygon(&((*l)->C.pol), &tab[0]);
    inserePolygon(&((*l)->C.pol), &tab[2]);
    inserePolygon(&((*l)->C.pol), &tab[1]);
    }
    (*l)->C.curlen = 3;
    (*l)->C.maxlen = 3;
}

/**
    *@brief la creation a la main (souris) du nuage  d'enveloppes convex 
    *
    *@param listeconvex* l
    *@param Point* tab
    *
    *@return int 2 ou 1
*/
int convexMouse(listeconvex* l, Point* tab){
    int cpt = 0;
    int x, y;
    int key;
    if((*l)){
        free(l);
        *l = NULL;
    }
    *l = alloueEnveloppe();
    while(1){

        MLV_Event event = MLV_wait_keyboard_or_mouse(NULL,NULL,&key,&x,&y);
        if(event == MLV_MOUSE_BUTTON)// recuperation des points 
            tab[cpt] = get_point_on_clic(x, y);
            
        MLV_clear_window(MLV_COLOR_BLACK);

        if(event == MLV_KEY){//En cliquant sur une touche du clavier on quitte et 
            // on libere la memoire 
            libereListC(l);
            if(key == MLV_KEYBOARD_r){// si on clique sur r on recommence 
                MLV_update_window();
                return 2;
            }
            return 1;
        }


        if(cpt < 2){// dessiner les deux premier point 
            MLV_draw_filled_circle(tab[0].x, tab[0].y, 2, MLV_COLOR_RED);
            MLV_draw_filled_circle(tab[cpt].x, tab[cpt].y, 2, MLV_COLOR_RED);
        }
        if(cpt == 2){// initialise l'enveloppe et la dessine
            initEnveloppe(l,tab);
            dessineConvex(*l);
        }
        
        if(cpt > 2){// traitement des points 
            direct(l, &tab[cpt]);
            dessineConvex(*l);
        }
            
        MLV_update_window();
        cpt++;
        }
    libereListC(l);
    return 1;
}

/**
    *@brief Generation d'un points aleatoire dans une zone precise delemite par 
            le rayon r et le centre passee en parametre 
    *
    *@param int r
    *@param Point centre
    *
    *@return Point

*/
Point fillSquare(int r, Point centre){
    Point p;
    double PERTUB = 0.0001/RAND_MAX;
    /*(rand() % (MAX - MIN +1)) + MIN formule pour avoir un nombre entre le min et le max*/
    p.x = centre.x + ((rand()%(r + r +1))- r )+ (rand()%2?+1.:-1.) * PERTUB*rand();
    p.y = centre.y + ((rand()%(r + r +1))- r )+ (rand()%2?+1.:-1.) * PERTUB*rand();
    return p;
}


/**
    *@brief Generation d'un points aleatoire dans une zone precise delemite par 
            le rayon r et le centre passee en parametre 
    *
    *@param int r
    *@param Point centre
    *
    *@return Point

*/
Point fillCircle(int r, Point centre){
    Point p;
    double PERTUB = 0.0001/RAND_MAX;
    
    do{
    /*(rand() % (MAX - MIN +1)) + MIN formule pour avoir un nombre entre le min et le max*/
    p.x = centre.x + ((rand()%(r + r +1))- r )+ (rand()%2?+1.:-1.) * PERTUB*rand();
    p.y = centre.y + ((rand()%(r + r +1))- r )+ (rand()%2?+1.:-1.) * PERTUB*rand();
    }while(sqrt((p.x-centre.x)*(p.x-centre.x) + (p.y-centre.y)*(p.y-centre.y)) > r);

    return p;
}


/**
    *@brief la creation automatique du nuage d'enveloppe convex sous la forme d'un carre 
            ou sous la forme d'un cercle 
    *
    *@param listeconvex *l
    *@param Point* tab
    *@param int nbpoint
    *@param int rayon
    *@param int forme
    *@param int affichage
    *
    *@return int 2 si l'utilisateur souhaite relancer le programme avec 
             les meme options 1 si le programme est terminer 

*/
int convexForme(listeconvex *l, Point* tab, int nbpoint, int rayon, int forme, int affichage){
    float rSpiral = 1;
    int d = nbpoint/rayon;
    int x,y,key;

    if((*l)){
        free(l);
        *l = NULL;
    }
    *l = alloueEnveloppe();
    // recuperation du point 'centre' et gestion d'erreur 
    printf("Cliquez pour choisir un point comme centre\n");
    MLV_wait_mouse(&x, &y);
    while(x + rayon >= width || x - rayon <= 0 || y + rayon >= height || y - rayon <= 0){
        printf("Centre trop pres du bord avec un rayon de %d l'enveloppe va sortir de la fenetre\n", rayon);
        printf("Cliquez pour choisir un point comme centre\n");
        MLV_wait_mouse(&x, &y);
    }
    Point c = get_point_on_clic(x,y);
    // generation des points selon la forme souhaite 
    for(int i = 0; i < nbpoint; ++i){
        if(i%d == 0)
            rSpiral+=d/((float) nbpoint/ (float) rayon);
        
        if(forme == 0)
            tab[i] = fillCircle(rSpiral, c);
        if(forme == 1)
            tab[i] = fillSquare(rSpiral, c);
    }

    for(int i = 0; i < nbpoint; ++i){
        
        if(i == 2){ // initialisation de l'enveloppe a partir de 3 point 
            initEnveloppe(l,tab);
        }
        if(i > 2){  // traitement des points 
            direct(l, &tab[i]);
            if(!affichage){
                MLV_clear_window(MLV_COLOR_BLACK);
                dessineConvex(*l);
                MLV_update_window();
            }
        }
    }
    // affichage 
    if(affichage){
        MLV_clear_window(MLV_COLOR_BLACK);
        dessineConvex(*l);
        MLV_update_window();
    }

    printf("appuyez sur 'r' pour recommencer\n");
    MLV_Event event = MLV_wait_keyboard_or_mouse(NULL,NULL,&key,NULL, NULL);
    libereListC(l);
    if(event == MLV_KEY)
        if(key == MLV_KEYBOARD_r){
            MLV_clear_window(MLV_COLOR_BLACK);
            MLV_update_window();
            return 2;
        }
    return 1;
}

/**
    *@brief test si un char * c est un nombre ou pas 
            renvoie 0 si il ne s'agit pas d'un nombre 
            et 1 dans le cas contraire 
    *
    *@param char* c
    *
    *@return int 1 ou 0

*/
int isnumber(char* c){
    size_t len = strlen(c);
    for(int i = 0; i < len; ++i){
        if(!isdigit(c[i]))
            return 0;
    }
    return 1;
}

/**
    *@brief text expliquant la maniere d'utiliser les options a la compilation 

*/
void help(void){
    
    printf("pour les options :\n"
            "si vous lancez le programme sans option vous allez jouer sur la version a la souri\n"
            "sinon vous pouvez utiliser:\n"
            "-s nb r  ou s permet de choisir la forme carre et nb et r sont des entiers pour designer le nombres de points et la taille du rayon\n"
            "-c nb r  meme chose que pour le carre mais pour la forme 'cercle'\n"
            "-t pour avoir l'affichage terminal(cad seulement l'affichage final)\n"
            "-d pour avoir l'affichage dynamique\n");
}




int main(int argc, char* argv[]){
    srand(time(NULL));
    int modeSouris = 1;// 0 -> pas de mode souris donc lance aleatoire controlé
    int affichage = 0; // 0 -> affichage dynamique & 1 -> affichage terminal
    int rayon, nbpoint = MAXPOINT;//option si on est en mode aleatoire controlé
    int forme; // 0 -> cercle & 1 -> carre
    listeconvex liste = NULL;
    Point *tab;


    for(int i = 1; i < argc; ++i){ // gestion des options a la compilation 
        if(argv[i][0] == '-'){
            if(argv[i][2] != '\0'){
                printf("mauvaise commande mettez 'help' en option pour voir les option disponible\n");
                return 0;
            }
            if (argv[i][1] == 's'){
                if(argc-i > 2 && isnumber(argv[i+1]) && isnumber(argv[i+2])){
                    forme = 1;
                    nbpoint = atoi(argv[i+1]);
                    rayon = atoi(argv[i+2]);
                    modeSouris = 0;
                    if(nbpoint <= 0 || rayon <= 0){
                        printf("Apres le choix de la forme ,veuillez entrer sous forme d'entier le nombres de points voulu puis la taille du rayon voulu\n");
                        return 0;
                    }
                    if(rayon > nbpoint){
                        printf("il doit y avoir plus ou le meme nombre de points que la taille du rayon\n");
                        return 0;
                    }
                    i+=2;
                }
                else{
                    printf("Apres le choix de la forme ,veuillez entrer sous forme d'entier le nombres de points voulu puis la taille du rayon voulu\n");
                    return 0;
                }
            }
            else if(argv[i][1] == 'c'){
                if(argc-i > 2 && isnumber(argv[i+1]) && isnumber(argv[i+2])){
                    forme = 0;
                    nbpoint = atoi(argv[i+1]);
                    rayon = atoi(argv[i+2]);
                    modeSouris = 0;
                    if(nbpoint <= 0 || rayon <= 0){
                        printf("Apres le choix de la forme ,veuillez entrer sous forme d'entier le nombres de points voulu puis la taille du rayon voulu\n");
                        return 0;
                    }
                    if(rayon > nbpoint){
                        printf("il doit y avoir plus ou le meme nombre de points que la taille du rayon\n");
                        return 0;
                    }
                    i+=2;
                }
                else{
                    printf("Apres le choix de la forme ,veuillez entrer sous forme d'entier le nombres de points voulu puis la taille du rayon voulu\n");
                    return 0;
                }
            }
            else if(argv[i][1] == 'd')
                affichage = 0;
            else if(argv[i][1] == 't')
                affichage = 1;
            else{
                printf("mauvaise commande mettez 'help' en option pour voir les option disponible\n");
                return 0;
            }
        }
        else if(!strcmp(argv[i], "help")){
            help();
            return 0;
        }
        else{
            printf("mauvaise commande mettez 'help' en option pour voir les option disponible\n");
            return 0;
            }
    }


    if(!(tab = (Point*) calloc(nbpoint, sizeof(Point)))){
        printf("echec de l'allocation du tableau de points");
        exit(0);
    }

    MLV_create_window("convex", "convex", width, height);

    if(modeSouris)
        while(convexMouse(&liste, tab) == 2);
    else{
        while(convexForme(&liste, tab, nbpoint, rayon, forme, affichage) == 2);
    }

    MLV_free_window();
    free(tab);

}