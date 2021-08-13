/*
	Autor: Branislav Mateas
	Login: xmatea00
	Projekt c.2 do IOS
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>
#include <assert.h>

/* Premenne s hodnotami z argumentov */
int R, C, ART/*ms*/, ABT/*ms*/;

/* Premenne pre pracu so zdielanou pamatou */
int *shared_memory;
int SM_ID;

/* Premenne pre pracu so semaformi */
sem_t *teraz_ja        = NULL;
sem_t *all_aboard      = NULL;
sem_t *autobus         = NULL;
sem_t *zapis_do_suboru = NULL;
sem_t *rider_finish    = NULL;
sem_t *next_rider      = NULL;

/* Premenne pre procesy */
pid_t BUS;
pid_t RIDER_GEN;
pid_t RID;

/* Premenna na pracu so suborom */
FILE *fp;

/* Dalsie pomocne premenne */
int end_BUS=1;

/* Funckia tlaci riadky do suboru */
void print_do_suboru(char *string)
{
	sem_wait(zapis_do_suboru);
	shared_memory[4]++;
	shmctl(SM_ID, IPC_STAT, 0);
	fprintf (fp, "%d %s", shared_memory[4], string);
	fflush(fp);
	sem_post(zapis_do_suboru);
}


/* Funkcia nacita argumenty, pri chybe konci program a vracia 1 */
void spracovanie_argumentov(int p_argc, char *p_argv[])
{
	if(p_argc != 5)
	{
		fprintf(stderr, "Neplatny pocet argumentov\n");
		exit(1);
	}
	if(sscanf(p_argv[1], "%d", &R) == 0)
	{
		fprintf(stderr, "Neplatny vstup v argumentoch\n");
		exit(1);
	}
	if(sscanf(p_argv[2], "%d", &C) == 0)
	{
		fprintf(stderr, "Neplatny vstup v argumentoch\n");
		exit(1);
	}
	if(sscanf(p_argv[3], "%d", &ART) == 0)
	{
		fprintf(stderr, "Neplatny vstup v argumentoch\n");
		exit(1);
	}
	if(sscanf(p_argv[4], "%d", &ABT) == 0)
	{
		fprintf(stderr, "Neplatny vstup v argumentoch\n");
		exit(1);
	}
}
/* Funckia skontroluje interval hodnot z argumentov, pri chybe konci program a vracia 1 */
void spracovanie_intervalov()
{
	if(R <= 0)
	{
		fprintf(stderr, "Neplatny rozsah hodnot v argumente\n");
		exit(1);
	}
	if(C <= 0)
	{
		fprintf(stderr, "Neplatny rozsah hodnot v argumente\n");
		exit(1);
	}
	if(ART < 0 || ART > 1000)
	{
		fprintf(stderr, "Neplatny rozsah hodnot v argumente\n");
		exit(1);
	}
	if(ABT < 0 || ABT > 1000)
	{
		fprintf(stderr, "Neplatny rozsah hodnot v argumente\n");
		exit(1);
	}
}

/* Funkcia uvolni zdielanu pamat */
void ukoncenie_zdielanej_pamate()
{
	if(shmdt(shared_memory) == -1 )
	{
		fprintf(stderr, "Problem s odpojenim zdielanej pamate");
		exit(1);
	}
	if(shmctl(SM_ID, IPC_RMID, NULL) == -1)
	{
		fprintf(stderr, "Problem s odstranenim zdielanej pamate");
		exit(1);
	}
}

/* Funckia zavrie a odlinkuje vsetky semafory */
void ukoncenie_semaforov()
{
	sem_close(teraz_ja);
	sem_close(all_aboard);
	sem_close(autobus);
	sem_close(zapis_do_suboru);
	sem_close(rider_finish);
	sem_close(next_rider);
	sem_unlink("/xmatea00.proj2.teraz_ja");
	sem_unlink("/xmatea00.proj2.all_aboard");
	sem_unlink("/xmatea00.proj2.autobus");
	sem_unlink("/xmatea00.proj2.zapis_do_suboru");
	sem_unlink("/xmatea00.proj2.rider_finish");
	sem_unlink("/xmatea00.proj2.next_rider");
}

/* Inicializuje semafory, subor, a zdielanu pamat */
void inicializacia()
{
	/* Otvorenie suboru */
	fp = fopen("proj2.out", "w");
	setbuf(fp, NULL);
	if(fp == NULL )
	{
		fprintf(stderr, "Problem s otvorenim suboru");
		exit(1);
	}

	/* Inicializacia semaforov */
	if((teraz_ja  = sem_open("/xmatea00.proj2.teraz_ja", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
	{
		fprintf(stderr, "Problem so semaforom teraz_ja - /xmatea00.proj2.teraz_ja\n");
		fclose(fp);
		exit(1);
	}
	if((all_aboard  = sem_open("/xmatea00.proj2.all_aboard", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
	{
		fprintf(stderr, "Problem so semaforom all_aboard - /xmatea00.proj2.all_aboard\n");
		sem_close(teraz_ja);
		sem_unlink("/xmatea00.proj2.teraz_ja");
		fclose(fp);
		exit(1);
	}
	if((autobus  = sem_open("/xmatea00.proj2.autobus", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
	{
		fprintf(stderr, "Problem so semaforom autobus - /xmatea00.proj2.autobus\n");
		sem_close(teraz_ja);
		sem_close(all_aboard);
		sem_unlink("/xmatea00.proj2.teraz_ja");
		sem_unlink("/xmatea00.proj2.all_aboard");
		fclose(fp);
		exit(1);
	}
	if((zapis_do_suboru = sem_open("/xmatea00.proj2.zapis_do_suboru", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
	{
		fprintf(stderr, "Problem so semaforom autobus - /xmatea00.proj2.zapis_do_suboru\n");
		sem_close(teraz_ja);
		sem_close(all_aboard);
		sem_close(autobus);
		sem_unlink("/xmatea00.proj2.teraz_ja");
		sem_unlink("/xmatea00.proj2.all_aboard");
		sem_unlink("/xmatea00.proj2.autobus");
		fclose(fp);
	 	exit(1);
	}
	if((rider_finish = sem_open("/xmatea00.proj2.rider_finish", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
	{
		fprintf(stderr, "Problem so semaforom autobus - /xmatea00.proj2.zapis_do_suboru\n");
		sem_close(teraz_ja);
		sem_close(all_aboard);
		sem_close(autobus);
		sem_close(zapis_do_suboru);
		sem_unlink("/xmatea00.proj2.teraz_ja");
		sem_unlink("/xmatea00.proj2.all_aboard");
		sem_unlink("/xmatea00.proj2.autobus");
		sem_unlink("/xmatea00.proj2.zapis_do_suboru");
		fclose(fp);
	 	exit(1);
	}
	if((next_rider = sem_open("/xmatea00.proj2.next_rider", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
	{
		fprintf(stderr, "Problem so semaforom autobus - /xmatea00.proj2.zapis_do_suboru\n");
		sem_close(teraz_ja);
		sem_close(all_aboard);
		sem_close(autobus);
		sem_close(zapis_do_suboru);
		sem_close(rider_finish);
		sem_unlink("/xmatea00.proj2.teraz_ja");
		sem_unlink("/xmatea00.proj2.all_aboard");
		sem_unlink("/xmatea00.proj2.autobus");
		sem_unlink("/xmatea00.proj2.zapis_do_suboru");
		sem_unlink("/xmatea00.proj2.rider_finish");
		fclose(fp);
	 	exit(1);
	}

	/* Inicializacia zdielanej pamate */
	int SM_SIZE = sizeof(int)*5;
	SM_ID = shmget(IPC_PRIVATE, SM_SIZE, IPC_CREAT | 0666);
	if( SM_ID < 0)
	{
		ukoncenie_semaforov();
		fclose(fp);
		fprintf(stderr, "Problem s inicializaciou pamate\n"); 
		exit(1);

	}
	shared_memory = (int*) shmat(SM_ID, 0, 0);
	if(shared_memory == NULL)
	{
		ukoncenie_semaforov();
		fclose(fp);
		fprintf(stderr, "Nepodarilo sa priradit zdielanu pamat\n");
		exit(1);
	}

	/* Inicializovanie hodnot */
	shared_memory[0] = 0; // CR
	shared_memory[1] = 0; // pocet prepravenych
	shared_memory[2] = R; // R 
	shared_memory[3] = 0; // aktualny pocet ludi v autobuse 
	shared_memory[4] = 0; // A
 }

/* Funckia spracuje proces RIDER */
void handle_rider(int ID)
{
	char buf[256];

	sprintf (buf, "\t : RID %d \t : start\n",ID);
	print_do_suboru(buf);

	sem_wait(teraz_ja);
	shared_memory[0]++;
	shmctl(SM_ID, IPC_STAT, 0);

	sprintf (buf, "\t : RID %d \t : enter: %d\n",ID, shared_memory[0]);
	print_do_suboru(buf);

	sem_post(teraz_ja);
	sem_wait(autobus);
	shared_memory[0]--;
	shared_memory[3]++;
	shared_memory[1]++;
	shmctl(SM_ID, IPC_STAT, 0);
	sprintf (buf, "\t : RID %d \t : boarding\n", ID);
	print_do_suboru(buf);
	if(shared_memory[0] == 0 || shared_memory[3]>= C)
	{
		shared_memory[3] = 0;
		sprintf (buf, "\t : BUS \t \t : end boarding: %d\n", shared_memory[0]);
		print_do_suboru(buf);
		sem_post(all_aboard);
	}
	else
	{
		sem_post(autobus);
	}
	sem_wait(rider_finish);
	sprintf (buf, "\t : RID %d \t : finish\n", ID);
	print_do_suboru(buf);
	sem_post(next_rider);
}

/* Funcia na generovanie riderov */
void generate_rider(int pocet, int cas)
{
	pid_t pid[pocet];

	for(int i = 0; i < pocet; i++)
	{
		pid[i] = fork();
		if(pid[i] == 0)
		{
			handle_rider(i+1);
			exit(0);
		}
		if(pid[i] == -1)
		{
			fprintf(stderr, "Nepodarilo sa vytvorit proces pre cestujuceho %d\n", i+1);
			fclose(fp);
			ukoncenie_zdielanej_pamate();
			ukoncenie_semaforov();
			exit(1);	
		}
		if(cas > 0)
			usleep((rand() % cas)*1000);
	}
	
}
/* MAIN */
int main(int argc, char **argv)
{
	/* Spracuju sa argumenty */
	spracovanie_argumentov(argc, argv);
	spracovanie_intervalov();

	/* Spravi sa potrebna incicializacia */
	inicializacia();

	char buf[256];

	BUS = fork(); // vytvori sa proces BUS
	if( BUS == 0)
	{
		sprintf (buf, "\t : BUS \t \t : start\n");
		print_do_suboru(buf);
		while(end_BUS) // cyklus konci prave ked boli vsetci pasazieri transportovani  
		{
			int vybavenych;
			sem_wait(teraz_ja);
			sprintf(buf,"\t : BUS \t \t : arrival\n");
			print_do_suboru(buf);
			vybavenych = 0;
			if(shared_memory[0] > 0)
			{
				if(shared_memory[0] > C )
				{
					vybavenych = C;
				}
				else
				{
					vybavenych = shared_memory[0];
				}
				sprintf(buf,"\t : BUS \t \t : start boarding: %d\n", shared_memory[0]);
				print_do_suboru(buf);
				sem_post(autobus);
				sem_wait(all_aboard);
			}	
			sem_post(teraz_ja);
			sprintf(buf,"\t : BUS \t \t : depart\n");
			print_do_suboru(buf);
			if(ABT > 0)
				usleep((rand() % ABT)*1000);

			sprintf(buf, "\t : BUS \t \t : end\n");
			print_do_suboru(buf);
			/* ukoncia sa vsetky procesy, ktore boli transportovane */ 
			for( int i = 0; i < vybavenych; i++)
			{
				sem_post(rider_finish);
				sem_wait(next_rider);
			}
			if(shared_memory[1] >= shared_memory[2]) end_BUS = 0;
		}
		sprintf(buf, "\t : BUS \t \t : finish\n");
		print_do_suboru(buf);
		exit(0);
	}
	else if( BUS == -1)
	{
		fprintf(stderr, "Nepodarilo sa vyvtorit proces BUS\n");
		fclose(fp);
		ukoncenie_zdielanej_pamate();
		ukoncenie_semaforov();
		exit(1);
	}
	else
	{
		RIDER_GEN = fork(); // vytvori sa proces RIDER_GEN
		if (RIDER_GEN == 0)
		{
			generate_rider(R, ART);
			exit(0);
		}
		else if (RIDER_GEN == -1)
		{
			printf ("Zlyhalo vytvorenie RIDER_GEN procesu\n");
			fclose(fp);
			ukoncenie_zdielanej_pamate();
			ukoncenie_semaforov();
			exit(1);
		}
	}

	/* Cakanie na skoncenie procesov autobus a generator riderov a uvolnenie zdrojov*/
	waitpid(BUS, NULL, 0);
	waitpid(RIDER_GEN, NULL, 0);	
	fclose(fp);
	ukoncenie_semaforov();
	ukoncenie_zdielanej_pamate();
	exit(0);
	return 0;
}