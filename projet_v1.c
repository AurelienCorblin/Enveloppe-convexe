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

// Fonctions 


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
    *@brief nettoyage de l'enveloppe convex avant et arriere .
    *
    *@param ConvexHull* C

*/
void nettoyage(ConvexHull* C, Polygon* supp){
    Polygon tmp = NULL;

    // nettoyage de l'enveloppe convex vers l'avant 
    while(ProduitVectoriel(*(C->pol->s),*(C->pol->next->s),*(C->pol->next->next->s)) < 0.){
        tmp = C->pol->next;
        C->pol->next = C->pol->next->next;
        C->pol->next->prev = C->pol;
        inserePolygon(supp, tmp->s); 
        free(tmp);
        C->curlen--;
    }
    // nettoyage de l'enveloppe convex vers l'arriere 
    while(ProduitVectoriel(*(C->pol->s),*(C->pol->prev->prev->s),*(C->pol->prev->s)) < 0.){
        tmp = C->pol->prev;
        C->pol->prev = C->pol->prev->prev;
        C->pol->prev->next = C->pol;
        inserePolygon(supp, tmp->s);
        free(tmp);
        C->curlen--;
    }
}


/**
    *@brief test si le point 'p' est en dehors de l'enveloppe
            renvoie 1 apres insertion de ce dernier comme nouveau sommet,
            sinon le point est a l'interieur de l'enveloppe et on renvoie 0 
    *
    *@param ConvexHull* C
    *@param Point* p
    *@param Polygon* in
    *
    *@return int 1 ou 0 

*/
int testdirect(ConvexHull* C, Point* p, Polygon* in){
    
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
    // cas ou le point 'p' est l'interieur de l'enveloppe 
    inserePolygon(in, p);
    return 0;
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
    *@brief Dessine le Polygon 'p' en reliant les differents sommets les uns apres
            les autres par des trait rouge 
    *
    *@param ConvexHull C

*/
void dessineConvex(ConvexHull C){  
    for(int i = 0; i < C.curlen; ++i){
        MLV_draw_line(C.pol->s->x, C.pol->s->y, C.pol->next->s->x, C.pol->next->s->y, MLV_COLOR_RED);
        C.pol = C.pol->next;
    }
}


/**
    *@brief Dessine les sommets du Polygon 'p' sous forme de cercle avec a couleur 
            color passe en parametre 
    *
    *@param MLV_Color color
    *@param Polygon p

*/
void dessinePoint(Polygon p, MLV_Color color){
    if(p == NULL)
        return;
    for(Polygon tmp = p->next; tmp != p; tmp = tmp->next){
        MLV_draw_filled_circle(tmp->s->x, tmp->s->y, 2, color);
    }
    MLV_draw_filled_circle(p->s->x, p->s->y, 2, color);
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
    *@brief Initialise le Polygone avec les trois premier point dans l'ordre 
            formant un triangle dans le sens droit (le sens des aiguilles d'une montre)
    *
    *@param ConvexHull* convex
    *@param  Point* tab

*/
void initEnveloppe(ConvexHull* convex, Point* tab){
    if(ProduitVectoriel(tab[0], tab[1], tab[2]) > 0.){
            inserePolygon(&(convex->pol), &tab[0]);
            inserePolygon(&(convex->pol), &tab[1]);
            inserePolygon(&(convex->pol), &tab[2]);
            }
    else{
        inserePolygon(&(convex->pol), &tab[0]);
        inserePolygon(&(convex->pol), &tab[2]);
        inserePolygon(&(convex->pol), &tab[1]);
        }
    convex->curlen = 3;
    convex->maxlen = 3;

}



/**
    *@brief dessine tout les points sommets ayant appartenue  a l'enveloppe 
            Ainsi que ceux etant initialement a l'interieur de l'enveloppe 
    *
    *@param ConvexHull* convex
    *@param Polygon* supp
    *@param Polygon* in
*/
void dessineLesPoints(ConvexHull* convex, Polygon* supp, Polygon* in){
    dessinePoint(convex->pol, MLV_COLOR_GREEN);
    dessinePoint(*supp, MLV_COLOR_ORANGE);
    dessinePoint(*in, MLV_COLOR_BLUE);
    MLV_update_window();
}



/**
    *@brief la creation a la main (souris) de l'enveloppe convex 
    *
    *@param ConvexHull* convex
    *@param Point* tab
    *@param Polygon* supp
    *@param Polygon* in
    *
    *@return int 2 ou 1
*/
int convexMouse(ConvexHull* convex, Point* tab, Polygon* supp, Polygon* in){
    int cpt = 0;
    int x, y;
    int key;
    while(1){

        MLV_Event event = MLV_wait_keyboard_or_mouse(NULL,NULL,&key,&x,&y);
        if(event == MLV_MOUSE_BUTTON)// recuperation des points 
            tab[cpt] = get_point_on_clic(x, y);

        MLV_clear_window(MLV_COLOR_BLACK);
        
        if(event == MLV_KEY){//En cliquant sur une touche du clavier on quitte et 
            // on libere la memoire 
            libere(&(convex->pol));
            libere(supp);
            libere(in);
            printf("la longueur maximale que l'enveloppe a atteinte est %d\n", convex->maxlen);
            if(key == MLV_KEYBOARD_r){// si on clique sur r on recommence 
                MLV_update_window();
                return 2;
            }
            return 1;
        }
        if(cpt < 2){
            // dessiner les deux premier point 
            MLV_draw_filled_circle(tab[0].x, tab[0].y, 2, MLV_COLOR_GREEN);
            MLV_draw_filled_circle(tab[cpt].x, tab[cpt].y, 2, MLV_COLOR_GREEN);
        }
        if(cpt == 2){
            // initialise l'enveloppe et la dessine
            initEnveloppe(convex,tab);
            dessineConvex(*convex);
        }
        
        if(cpt > 2){
            // traitement des points 
            if(testdirect(convex, &tab[cpt], in) == 1){
                // le point tab[cpt] est un nouveau sommet faut donc nettoyer 
                nettoyage(convex, supp);
            }
            dessineConvex(*convex);
            MLV_draw_text(width-160, 10, "longueur courante: %d", MLV_COLOR_BLUE, convex->curlen);
        }
        dessineLesPoints(convex,supp,in);
        cpt++;
        }
    printf("la longueur maximale que l'enveloppe a atteinte est %d\n", convex->maxlen);
    return 1;
}

/**
    *@brief la creation automatique de l'enveloppe convex sous la forme d'un carre 
            ou sous la forme d'un cercle 
    *
    *@param ConvexHull* convex
    *@param Point* tab
    *@param Polygon* supp
    *@param Polygon* in
    *
    *@return int 2 si l'utilisateur souhaite relancer le programme avec 
             les meme options 1 si le programme est terminer 

*/
int convexForme(ConvexHull* convex, Point* tab, Polygon* supp, Polygon* in, int nbpoint, int rayon, int forme, int affichage){
    float rSpiral = 1; 
    int d = nbpoint/rayon;
    int x,y,key;
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
            rSpiral+= d/((float) nbpoint/ (float) rayon);
        
        if(forme == 0)
            tab[i] = fillCircle(rSpiral, c);
        if(forme == 1)
            tab[i] = fillSquare(rSpiral, c);
    }

    for(int i = 0; i < nbpoint; ++i){
        
        if(i == 2){// initialisation de l'enveloppe a partir de 3 point 
            initEnveloppe(convex,tab);
            if(!affichage){
                MLV_clear_window(MLV_COLOR_BLACK);
                dessineConvex(*convex);}
        }
        
        if(i > 2){ // traitement des points 
            if(testdirect(convex, &tab[i], in) == 1){
                nettoyage(convex, supp);
            }    
            if(!affichage){
                MLV_clear_window(MLV_COLOR_BLACK);
                dessineConvex(*convex);}
        }
        if(!affichage){ //  affichage dynamique  
            MLV_draw_text(width-160, 10, "longueur courante: %d", MLV_COLOR_BLUE, convex->curlen);
            dessineLesPoints(convex,supp,in);
        }
    }
    if(affichage){ // affichage terminal 
        MLV_clear_window(MLV_COLOR_BLACK);
        dessineConvex(*convex);
        dessineLesPoints(convex,supp,in);
    }

    printf("la longueur maximale que l'enveloppe a atteinte est %d\n", convex->maxlen);


    printf("appuyez sur 'r' pour recommencer\n");
    MLV_Event event = MLV_wait_keyboard_or_mouse(NULL,NULL,&key,NULL, NULL);
    libere(&(convex->pol)); // liberation de la memoire
    libere(supp);
    libere(in);
    if(event == MLV_KEY)
        if(key == MLV_KEYBOARD_r){
            MLV_clear_window(MLV_COLOR_BLACK);
            MLV_update_window();
            return 2; // on recommence 
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
    ConvexHull convex;
    convex.pol = NULL;
    convex.maxlen = 0;
    Polygon supp = NULL;
    Polygon in = NULL;
    Point* tab;
  
    for(int i = 1; i < argc; ++i){// Gestion des option entre sur la ligne de commande 
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
        while(convexMouse(&convex, tab, &supp, &in) == 2);
    else{
        while(convexForme(&convex, tab, &supp, &in, nbpoint, rayon, forme, affichage) == 2);
    }

    MLV_free_window();
    free(tab);
}