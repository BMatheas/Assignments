/* IZP - zaklady programovania
 * Projekt 2 - iteracne vypocty
 * Branislav Mateas
 *
 * xmatea00@fit.vutbr.cz
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void output_tan(char *p_uholA, char *p_N, char *p_M); //funkcia po zadani argumentu --tan vypocita tangens uhlu v zadanom rozsahu a vypise na stdout
void output_m(double p_uholA, double p_uholB, int p_existA, int p_existB, double p_kamera);// funckia vypocita vzdialenost a vysku meraneho objektu po zadani argumentu -c alebo -m  
double vyska(double p_uhol, double p_dlzka, double p_kamera); //funckia vypocita vysku meraneho objektu
double dlzka(double p_uhol, double p_kamera); //funckia vypocita vzdialenost objektu
double x_na_y(double p_x, int p_y); // funkcia umocni cislo p_x na p_y 
double taylor_tan(double x, unsigned int n); // funkcia vypocita tangens uhlu v radianoch formou tayloroveho polynomu
double cfrac_tan(double x, unsigned int n); // funkcia vypocita tangens uhlu v raidanoch formou zretazeneho zlomku
void disp_info();


int main(int argc, char **argv)
{
    char argument[10];
    char arg_help[] = "--help";
    char arg_tan[] = "--tan";
    char m[] = "-m";
    char c[] = "-c";

    double X;
    double uholA;
    double uholB;
    int existA = 0;
    int existB = 0;

    if(argc < 2)
    {
        fprintf(stderr, "Program bol spusteny bez argumentu\n");
        return 10;
    }
    strcpy(argument, argv[1]);
    if(strcmp(argument, arg_help) == 0) //HELP
    {
        disp_info();
        return 0;
    }

    else if(strcmp(argument, arg_tan) == 0) //TAN
    {
        if(argc < 4)
        {
            fprintf(stderr, "Nezadali ste vsetky argumenty A, N a M\n");
            return 20;
        }
        output_tan(argv[2], argv[3], argv[4]);
        return 0;
    }

    else if(strcmp(argument, m) == 0 || strcmp(argument, c) == 0) // M alebo C
    {
        if(strcmp(argument, c) == 0) // kontroluje ci argument bol C
        {
            X = atof(argv[2]);
            if(X <= 0 || X > 100)
            {
                    fprintf(stderr, "Zle zadany argument X\n");
                    return 30;
            }
            uholA = atof(argv[4]);
            if(uholA <= 0 || uholA > 1.4)
            {
                fprintf(stderr, "Zle zadany uhol A\n");
                return 40;
            }
            existA = 1;
            if(argv[5]) // je zadany uhol B
            {
                uholB = atof(argv[5]);
                if(uholB <= 0 || uholB > 1.4)
                {
                    fprintf(stderr, "Zle zadany uhol B\n");
                    return 40;
                }
                existB = 1;
            }
        }
        else // ak tam nie je C
        {
            X = 1.5;
            uholA = atof(argv[2]);
            if(uholA <= 0 || uholA > 1.4)
            {
                fprintf(stderr, "Zle zadany uhol A\n");
                return 40;
            }
            existA = 1;
            if(argv[3])
            {
                uholB = atof(argv[3]);
                if(uholB <= 0 || uholB > 1.4)
                {
                    fprintf(stderr, "Zle zadany uhol B\n");
                    return 40;
                }
                existB = 1;
            }
        }
        output_m(uholA, uholB, existA, existB, X);
    }
    else
    {
        fprintf(stderr, "Nezadali ste vhodny argument\n");
        return 50;
    }
    return 0;
}
void output_tan(char *p_uholA, char *p_N, char *p_M) // funckia vypise na obrazovku vysledky v pripade, ze bol zadany argument --tan
{
    double uholA = atof(p_uholA);
    int N = atoi(p_N);
    int M = atoi(p_M);
    if( (N >= 14 || M >= 14) || (N <= 0 || M <= 0) || ( N > M ))
    {
        fprintf(stderr, "Argumenty N a M su zle zadane\n");
        exit(60);
    }
    else
    {
        double tan_taylor, tan_cfrac, tan_math;
        for(int i = N; i <= M; i++)
        {
            tan_taylor = taylor_tan(uholA, i);
            tan_cfrac = cfrac_tan(uholA, i);
	    tan_math = tan(uholA);
            printf("%d %e %e %e %e %e\n", i, tan_math, tan_taylor, fabs(tan_math - tan_taylor), tan_cfrac, fabs(tan_math - tan_cfrac));
        }
    }
}

void output_m(double p_uholA, double p_uholB, int p_existA, int p_existB, double p_kamera) //funckia vypise na obrazovku vysledky v pripade, ze argument programu bol -m alebo -c
{
    double d, v;
        d = dlzka(p_uholA, p_kamera);
        v = vyska(p_uholB, d, p_kamera);

        if(p_existA && !p_existB)
        {
            printf("%.10e\n", d);
        }
        if(p_existA && p_existB)
        {
            printf("%.10e\n%.10e\n", d, v);
        }
}

double vyska(double p_uhol, double p_dlzka, double p_kamera) //funckia vzpocita vysku meraneho objektu
{
    return cfrac_tan(p_uhol, 10) * p_dlzka + p_kamera; //10 iteracii som si zvolil pretoze pre kazdy uhol v rozsahu 0 - 1.4 je potreba 10 iteracii pre dosiahnutie presnosti na 10 desatinnych miest
}

double dlzka(double p_uhol, double p_kamera) //funckia vypocita vzdialenost D
{
    return p_kamera / cfrac_tan(p_uhol, 10); // 10 iteracii pretoze pre vsetky uhly v danom rozsahu su s presnostou na 10 desatinnych miest
}

double cfrac_tan(double x, unsigned int n) //funkcia vypocita tangens uhlu pomocou zretazeneho zlomku
{
    double cf = 0.;
    double a;
    for(unsigned int i = n; i > 1; i--)
    {
        a = (2*i-1);
        cf = (x*x)/(a-cf);

    }
    return x / ( 1 - cf);
}

double taylor_tan(double x, unsigned int n) //funkcia vypocita tangens uhlu pomocou tayleroveho radu
{
    double citatel[13] = {1, 1, 2, 17, 62, 1382, 21844, 929569, 6404582, 443861162, 18888466084, 113927491862, 58870668456604};
    double menovatel[13] = {1, 3, 15, 315, 2835, 155925, 6081075, 638512875, 10854718875, 1856156927625, 194896477400625, 49308808782358125, 3698160658676859375};
    double result = 0.;
    int mocnina = 1;
    double ans = 0.;

    for( unsigned int i = 0; i <=n; i++)
    {
        result = citatel[i]*x_na_y(x,mocnina)/menovatel[i];
        ans += result;
        mocnina += 2;
    }
    return ans;
}
double x_na_y(double p_x, int p_y) // funckia ktora umocni parameter p_x na p_y
{
    double temp = 1;
    for(int i = 0; i < p_y; i++)
    {
        temp *= p_x;
    }
    return temp;
}
void disp_info() // vypise na obrazovku obsluhu programu
{
    printf("Program sa spusta:\n");
    printf("./proj2 --help\n");
    printf("alebo\n");
    printf("./proj2 --tan A N M\n");
    printf("\tVypocita tangens uhlu A\n\tA - je uhol v radianoch, z ktoreho sa bude pocitat tangens\n\tN - je cislo iteracie OD\n\tM - je cislo iteracie DO\n");
    printf("alebo\n");
    printf("./proj2 [-c X] -m A [B]\n");
    printf("\tVypocita vzdialenost D, a pokial je zadany uhol B vypocita vysku meraneho objektu V\n\tX - nastavi vysku kamery 0 < X <= 100, pokial nie je zadane [-c X] program nastavi implicitne vysku 1.5\n\tA - uhol v radianoch v rozsahu 0 < A <= 1.4\n\tB - uhol v radianoch v rozsahu 0 < B <= 1.4\n");
}
