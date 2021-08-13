/**
 *
 * Kostra programu pro 3. projekt IZP 2017/18
 *
 * Jednoducha shlukova analyza
 * Unweighted pair-group average
 * https://is.muni.cz/th/172767/fi_b/5739129/web/web/usrov.html
 *
 * Branislav Mateas login: xmatea00
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX
#include <ctype.h>

/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
/ dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */
/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);
    //inicializacia clustra
    c->size = 0;
    c->capacity = 0;
    c->obj = NULL;
    if (cap > 0)
    {
	if ((c->obj = malloc(cap * sizeof(struct obj_t))))
	{
	     c->capacity = cap;
	}
    }
}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
    if(c->capacity)
	free(c->obj);
    c->obj = NULL;
    c->capacity = 0;
    c->size = 0;	
}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t*)arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    if (c->size == c->capacity)
        resize_cluster(c, c->capacity + CLUSTER_CHUNK);

    if(c->size < c->capacity)
    {
	c->obj[c->size] = obj;
	c->size++;
    }
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);

    for (int i = 0; i < c2->size; i++)
    {
        append_cluster(c1, c2->obj[i]);
    }
    sort_cluster(c1);
}

/**********************************************************************/
/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);

    clear_cluster(&carr[idx]);

    for (int i = idx; i < narr - 1; i++)
    {
        carr[i] = carr[i + 1];
    }
    narr -= 1;
    return narr;
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);
    float x = o1->x - o2->x;
    float y = o1->y - o2->y;
    return sqrtf((x*x)+(y*y));
}

/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);
	
    //pocitam body v clustroch kazdy s kazdym, vzdialenost ukladam do premennej avg 
    //a po ukonceni cyklu vydelim premennou cnt a ziskam priemernu vzdialenost
    float vzdialenost, avg = 0;
    int cnt = 0;
    for (int i = 0; i < c1->size; i++)
    {
	for (int j = i; j < c2->size; j++)
	{
	    vzdialenost = obj_distance(&c1->obj[i], &c2->obj[j]);
	    avg = avg+vzdialenost;
            cnt++;
	}
   }
   return avg/cnt;
}

/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);

    float min_vzdialenost = 10000, vzdialenost;
    for (int i = 0; i < narr; i++)
    {
	for (int j = i + 1; j < narr; j++)
	{
            vzdialenost = cluster_distance(&carr[i], &carr[j]);
	    if (vzdialenost < min_vzdialenost)
	    {
		min_vzdialenost = vzdialenost;
		*c1 = i;
		*c2 = j;
	    }
	}
    }
}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}
/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
    assert(arr != NULL);
    //deklaracie premennych
    char p_pole[20] = {};
    char p_cislo[20];
    int t = 0, k = 0, pocet_objektov, p_ID;
    float p_X, p_Y;
    
    struct obj_t obj;
    struct cluster_t *cluster;
    //otvorenie suboru
    FILE *fp;
    fp = fopen(filename, "r");
    if(fp == NULL)
    {
	fprintf(stderr, "Nepodarilo sa otvorit subor\n");
	exit(20);
    }
    //nacitavanie cisla
    fscanf(fp, "%s", p_pole);
    for( int i = 0; i < 20; i++)
    {
        if(isdigit(p_pole[i]))
        {
            p_cislo[k++] = p_pole[i];
	    t++;		
        }
    }
    p_cislo[k] = '\0';
    if(t == 0)
    {
	fprintf(stderr, "V prvom riadku vo formate count = sa nenachadza cislo\n");
	fclose(fp);
	exit(10);
    }
    pocet_objektov = atoi(p_cislo);

    if(pocet_objektov <= 0)
    {
	fprintf(stderr, "Zle cislo v count\n");
	fclose(fp);
	exit(10);  
    }
    //alokovanie pamate pre clustre o velkosti premennej pocet_objektov a nasledna inicializacia
    *arr = malloc(pocet_objektov * sizeof(struct cluster_t));
    for(int i = 0; i < pocet_objektov; i++)
    {
	init_cluster(&(*arr)[i], 0);
    }
    //nacitavanie objektov a suradnic
    for( int i = 0; i < pocet_objektov; i++)
    {
        fscanf(fp, "%d %f %f", &p_ID, &p_X, &p_Y);
        if( (p_X < 0 || p_X > 1000) || (p_Y < 0 || p_Y > 1000) )
	{
	    fprintf(stderr, "Bod %d ma suradnice v neplatnom rozsahu\n", p_ID);
	    fclose(fp);
	    for(int i = 0; i < pocet_objektov; i++)
 	    {
		clear_cluster(&(*arr)[i]);
	    }   
	    free(*arr);
	    exit(10);
	}
        obj.id = p_ID;
        obj.x = p_X;
        obj.y = p_Y;

        cluster = &(*arr)[i];
        append_cluster(cluster, obj);
    }
    fclose(fp);
    return pocet_objektov;
}

/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}
//funckia na spracovanie argumentov
int spracovanie_argumentov(char *p_argv, int p_argc)
{
    if(p_argc < 2)
    {
	fprintf(stderr, "Neplatny pocet argumentov\n");
	exit (10);
    }

    int p_p_c;

    if(p_argv)
    {
	if(sscanf(p_argv, "%d", &p_p_c) != 1)
	{
 	    fprintf(stderr, "Chybny vstup z argv[2]\n");
	    exit(30);
	}
    }
    else
	p_p_c = 1;

    if(p_p_c <= 0)
    {
	fprintf(stderr,"Cislo N v argumente je mensie alebo rovne 0\n");
	exit(20);
    }

	return p_p_c;
 
} 

int main(int argc, char *argv[])
{
    struct cluster_t *clusters;

    int pocet_nacitanych_objektov;
    int pocet_pozadovanych_clustrov;
    
    //funkcia spracuje argumenty a vrati cislo N z argumentu 2 
    pocet_pozadovanych_clustrov = spracovanie_argumentov(argv[2], argc);
    //nacitanie clustrov
    pocet_nacitanych_objektov = load_clusters(argv[1], &clusters);

    int c1, c2;

    if(pocet_pozadovanych_clustrov > pocet_nacitanych_objektov)
    {
	fprintf(stderr," Pocet pozadovanych clustrov je vacsi ako pocet nacitanych clustrov\n");
	for(int i = 0; i < pocet_nacitanych_objektov; i++)
	{
	    clear_cluster(&clusters[i]);
	}
	free(clusters);
	return 10;
    }

    // vypocet v cykle .. cyklus skonci v momente ako sa dosiahne pocet pozadovanych clustrov
    while(pocet_nacitanych_objektov >  pocet_pozadovanych_clustrov)
    {
        find_neighbours(clusters, pocet_nacitanych_objektov, &c1, &c2);

        merge_clusters(&clusters[c1], &clusters[c2]);

        pocet_nacitanych_objektov = remove_cluster(clusters, pocet_nacitanych_objektov, c2);
    }

    //tlac clustrov na stdout
    print_clusters(clusters, pocet_nacitanych_objektov);
    //vycistenie clustrov a uvolnenie pamate
    for(int i = 0; i < pocet_nacitanych_objektov; i++)
    {
	clear_cluster(&clusters[i]);
    }
    free(clusters);

    return 0;
}
