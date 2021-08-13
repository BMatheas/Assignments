/* IZP - Projekt 1
 * autor: Branislav Mateas
 * login: xmatea00
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//deklaracia funkcii
int kopirovanie(char *DO, char *Z); //funkcia sluzi na kopirovanie znakov z pola do pola
void na_velke_pismena(char *string); //funkcia meni male znaky na velke
int neplatne_znaky(char *string); // tato funckcia deteguje neplatne znaky na vstupe
int read_stdin_adresy(char p_adresy[][100], int p_level, char *p_pattern); // tato funkcia nacita adresy zo vstupu 
int zisti_povolene_znaky( char p_adresy[][100], char *p_povolene_znaky, int p_level, int p_pocet_adries); // funkcia z nacitanych miest zisti povolene znaky
int spracovanie_argumentov(int p_argc, char **p_argv, char *p_vstup, int *p_level ); // tato funkcia sluzi na spracovanie argmuentov argc, argv[]
void podaj_hlasenie(int p_pocet_znakov, int p_pocet_adries, char adresy[][100], char *p_povolene_znaky); // tato funckia sluzi na vypis na obrazovky 
void abecedne_zoradenie(char *p_string); //tato funkcia abecedne zoradi povolene znaky podla abecedy

//deklaracia premennych
char adresy[100][100];
char vstup[50];
char povolene_znaky[50];
int level, pocet_adries, pocet_znakov,err_code;

//MAIN
int main(int argc, char ** argv)
{
	err_code = spracovanie_argumentov(argc, argv, vstup, &level);
	if(err_code != 0) return err_code;

	pocet_adries = read_stdin_adresy(adresy, level, vstup);

	pocet_znakov = zisti_povolene_znaky(adresy, povolene_znaky, level, pocet_adries);

	abecedne_zoradenie(povolene_znaky);

	podaj_hlasenie(pocet_znakov, pocet_adries, adresy, povolene_znaky);

	return 0;
}
// vystup programu je zahrnuty v tejto funkcii 
void podaj_hlasenie(int p_pocet_znakov, int p_pocet_adries, char p_adresy[][100], char *p_povolene_znaky)
{
	//ak je viac adries vypise dalsie mozne znaky, ktore mozme stlacit
	if(p_pocet_znakov > 0 && p_pocet_adries > 1)
	{
		printf("Enable: %s\n", p_povolene_znaky);
	}
	// ak je pocet adries presne 1, vypiseme najdenu adresu
	if(p_pocet_adries == 1)
	{
		printf("Found: %s\n", p_adresy[0]);
	}
	else if(p_pocet_adries == 0) // ak pocet adriesw 0, vypiseme, ze adresa nebola najdena
	{
		printf("Not found\n");
	}
}
//funkcia sluzi na spracovanie argumentov, pokial su neplatne znaky vracia chybovu hodnotu 10
int spracovanie_argumentov(int p_argc, char **p_argv, char *p_vstup, int *p_level)
{
    if(p_argc > 1)
    {
        *p_level = kopirovanie(p_vstup, p_argv[1]);// VSTUP - nacitanie argumentu a zistenie jeho dlzky
	//*p_level = strlen(p_vstup);
        na_velke_pismena(p_vstup);
        if(neplatne_znaky(p_vstup) == 1) // osetrenie neplatnych znakov na vstupe s chybovym kodom 10
        {
		fprintf(stderr, "Vstup obsahuje neplatne znaky\n");
         	return 10;
        }
    }
	return 0;
}

//funckia sluzi na zistenie dalsich moznych znakov, ktore mozme stlacit, ako hodnotu vracia pocet znakov
int zisti_povolene_znaky( char p_adresy[][100], char *p_povolene_znaky, int p_level, int p_pocet_adries)
{
	int pocet = 0;
	int found = 0;
	char znak;
	for(int i = 0; i < p_pocet_adries; i++) // prejdeme vsetky adresy
	{
		znak = p_adresy[i][p_level]; // z adresy vyberieme znak, ktory sa nachadza po dlzke vstupu
		if(pocet == 0) // ak je pocet povolenych znakov 0,
		{
			p_povolene_znaky[pocet++] = znak; // jednoducho pridame znak do zoznamu
		}
		else
		{
			// v opacnom pripade skontrolujeme duplicitu kontrolovaneho znaku
			found = 0;
			for(int j = 0; j < pocet; j++)
			{
				if(znak == p_povolene_znaky[j])
				{
					found = 1;
					break;
				}
			}
			if(found == 0)
			{
				p_povolene_znaky[pocet++] = znak; // ak sa znak v zozname nenasiel, pridame ho
			}
		}
	}
	return pocet;
}
//funkcia sluzi na kopirovanie znakov, ako hodnotu vracia pocet skopirovanych znakov
int kopirovanie(char *DO, char *Z)
{
	int i;

	for(i = 0; Z[i] != 0; DO[i] = Z[i], i++);
	
	return i;
}

// vlastna funkcia na prevod malych znakov na velke
void na_velke_pismena(char *string)
{
	for(int i = 0; string[i] != 0; i++)
	{
		if(string[i] >= 'a' && string[i] <= 'z')
		{
			string[i] -= 32;
		}
	}
}
// funckia na kontrolu neplatnych znakov, ak najde neplatne znaky vracia 1
int neplatne_znaky(char *string)
{
	int vysledok = 0;
	int pocet = 0;
	for(int i = 0; (string[i] != 0) ; i++)
	{
		if((string[i] >= 'A' && string[i] <= 'Z') || (string[i] >= 'a' && string[i] <= 'z') || string[i] == ' ') pocet++;
		else vysledok = 1;
	}
	return vysledok;
}

//funkcia na nacitanie adries zo vstupu, ako hodnotu vracia pocet nacitanych adries
int read_stdin_adresy(char p_adresy[][100], int p_level, char * p_pattern)
{
    int koniec = 0;
    int pocet = 0;
    int dlzka = 0;
    char mesto[100];

    while(koniec != 1)
	{
		dlzka = scanf("%s",mesto); //nacitanie jednej adresy zo vstupu
		na_velke_pismena(mesto); //konverzia na velke pismena
		if(dlzka > 0) //kontrola ci je zadana nejaka adresa
		{
			if(p_level > 0) //kontrola ci su zadane kriteria hladania - vstup
			{

				if(strncmp(p_pattern, mesto, p_level) == 0) //kontrola zhodnosti adresy s kriteriami
				{
					kopirovanie(p_adresy[pocet], mesto); // ulozenie do pola adresy
					pocet++;
				}
			}
			else
			{
				kopirovanie(p_adresy[pocet], mesto); // ulozenie do pola adresy - vsetky
				pocet++;
			}
		}
		else
		{
			koniec = 1; // po nacitani prazdneho riadku ukocuje vstup adries
		}
		if(pocet >= 100) //kontrolap poctu vstupnych adries, zobbrazenie chyboveho hlasenia pri dosiahnuti maxima
		{
			koniec = 1;
			fprintf(stderr, "Dosiahli ste maximalny pocet adries: 100\n");
		}

	}
	return pocet;
}

//funkcia, ktora povolone znaky ENABLE zoradi podla abecedy - BUBBLE SORT metoda
void abecedne_zoradenie(char *p_string)
{
	char temp;
	int n = strlen(p_string);
	for(int i = 0; i < n; i++)
	{
		for(int j = i+1; j < n; j++)
		{
			if(p_string[i] > p_string[j])
			{
				temp = p_string[i];
				p_string[i] = p_string[j];
				p_string[j] = temp;
			}
		}
	}

}
