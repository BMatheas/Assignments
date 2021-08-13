/*
*	PROJEKT DO PREDMETU IMS
*	AUTORI: Branislav Mateas (XMATEA00), Petr Zufan (XZUFAN01)
*/


#include "simlib.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;


#define co2p 55
#define co2mp 98
#define co2hp 140
#define co2idle 0.4625
#define co2l 15.6

void convert_weight(int track, double number, long int vlak)
{
	if(number >= 1000)
	{
		number = number / 1000;
		if(number >= 1000)
		{
			number = number / 1000;
			if(vlak)
				printf("Uhlikova stopa dieseloveho vlaku na trasu: %d km, je: %.4lf t\n", track, number);
			else
				printf("Uhlikova stopa elektrickeho vlaku na trasu: %d km, je: %.4lf t\n", track, number);
		}
		else
		{
			if(vlak)
				printf("Uhlikova stopa dieseloveho vlaku na trasu: %d km, je: %.4lf kg\n", track, number);
			else
				printf("Uhlikova stopa elektrickeho vlaku na trasu: %d km, je: %.4lf kg\n", track, number);
		}
	}
	else
	{
		if(vlak)
				printf("Uhlikova stopa dieseloveho vlaku na trasu: %d km, je: %.4lf g\n", track, number);
			else
				printf("Uhlikova stopa elektrickeho vlaku na trasu: %d km, je: %.4lf g\n", track, number);
	}

	return ;
}

int Uniform(int min, int max) {
  return int( Uniform(double(min), double(max/*+1*/)) );
}

int main(int argc, char** argv)
{

	
	int track_length, pocet_zastavok;
	int pocet_vagonov, mini;
	sscanf(argv[1], "%d", &track_length);
	sscanf(argv[2], "%d", &pocet_zastavok);
	sscanf(argv[3], "%d", &pocet_vagonov);
	sscanf(argv[4], "%d", &mini);

	if(pocet_zastavok < 2)
		exit(20);
	if(pocet_vagonov < 1)
		exit(20);
	Init(0);
    RandomSeed(time(NULL));

	double length = track_length/pocet_zastavok;
    int load = Uniform(0, (130*pocet_vagonov));
    int pozadovane_minimum_l = ((130*pocet_vagonov)*mini)/100;

    int passengers = Uniform(0,(90*pocet_vagonov));
	int pozadovane_minimum_p = ((90*pocet_vagonov)*mini)/100;

	cout << "Delka trate je: " << track_length << "km Pocet zastavek je: " << pocet_zastavok << ", Pocet vagonu je: " << pocet_vagonov << ", Procento minima je: " << mini << endl;
		cout << "MINIMUM CESTUJICICH JE: " << pozadovane_minimum_p  << endl;
		cout << "MINIMUM NAKLADU JE: " << pozadovane_minimum_l  << endl;
		
		cout << endl;
		
		cout << "START PASS: " << passengers  << endl;
		
		double passenger_co, el_passenger_co;
		double wait_passenger_co, wait_el_passenger_co;
		double time = 0, time_wait = 0, time_wait_f = 0;
		int spozdeni = 0, spozdeni_load = 0;
		int aktualny_pocet_cestujucich = passengers;
		int tmp;
		double time_tmp = 0;
		int cnt = 0;
		int celkovy_pocet = passengers, celkovy_pocet_wait = passengers;
		while(passengers < pozadovane_minimum_p && time_tmp < 30)
		{
			time_tmp += Uniform(1,5);
			tmp = Uniform(0, (90*pocet_vagonov) - passengers);
			passengers += tmp;
			cnt += tmp;

			if(!(passengers < pozadovane_minimum_p))
			{
				time_wait += time_tmp + (double(cnt*4))/double(60);
				spozdeni += time_tmp;
				celkovy_pocet_wait += cnt;
			}
			else if(!(time_tmp < 30))
			{
				time_wait += time_tmp;
				spozdeni += time_tmp;
				celkovy_pocet_wait += cnt;
				break;
			}
		}
		
		time += (double(aktualny_pocet_cestujucich*4))/double(60);
		if(aktualny_pocet_cestujucich >= ((90*pocet_vagonov)/4)*3)
		{
			passenger_co = aktualny_pocet_cestujucich*co2p*length;
			el_passenger_co = (aktualny_pocet_cestujucich*co2p*length)*0.72;
		}
		else if((aktualny_pocet_cestujucich <= ((90*pocet_vagonov)/4)*3) && (aktualny_pocet_cestujucich >= ((90*pocet_vagonov)/2)))
		{
			passenger_co = aktualny_pocet_cestujucich*co2mp*length;
			el_passenger_co = (aktualny_pocet_cestujucich*co2mp*length)*0.72;
		} else 
		{
			passenger_co = aktualny_pocet_cestujucich*co2hp*length;
			el_passenger_co = (aktualny_pocet_cestujucich*co2hp*length)*0.72;
		}
		
		time_wait_f += (double(aktualny_pocet_cestujucich*4))/double(60);
		time_wait_f += time_wait;
		if(passengers >= ((90*pocet_vagonov)/4)*3)
		{
			wait_passenger_co = passengers*co2p*length;
			wait_el_passenger_co = (passengers*co2p*length)*0.72;
		}
		else if((passengers <= ((90*pocet_vagonov)/4)*3) && (passengers >= ((90*pocet_vagonov)/2)))
		{
			wait_passenger_co = passengers*co2mp*length;
			wait_el_passenger_co = (passengers*co2mp*length)*0.72;
		} else 
		{
			wait_passenger_co = passengers*co2hp*length;
			wait_el_passenger_co = (passengers*co2hp*length)*0.72;
		}

		double idle_co = time*co2idle;
		double wait_idle_co = time_wait_f*co2idle;

		double el_idle_co = (time*co2idle)*0.72;
		double wait_el_idle_co = (time_wait_f*co2idle)*0.72;
		
		int vystupi = 0, nastupi = 0;
		int vystupi_wait = 0, nastupi_wait = 0;
		
		cout << endl;
		
		for(int i = 1; i < pocet_zastavok; i++)
		{
			//cout << "Cestujuci: " << aktualny_pocet_cestujucich << endl;
			//cout << "Cestujuci2: " << passengers << endl;
			cout << endl;
			if(pocet_zastavok == 2 || i == pocet_zastavok-1)
			{
				time = (double(aktualny_pocet_cestujucich*4))/double(60);
				time_wait_f = (double(passengers*4))/double(60);
				idle_co	+= time*co2idle;
				el_idle_co += (time*co2idle)*0.72;
				wait_idle_co	+= time_wait_f*co2idle;
				wait_el_idle_co += (time_wait_f*co2idle)*0.72;
				break;
			}	
			
			if(aktualny_pocet_cestujucich != 0)
			{
				vystupi = Uniform(0, aktualny_pocet_cestujucich);
				
				vystupi_wait = Uniform(0, passengers);
				
				time = (double(vystupi*4))/double(60);
				
				time_wait_f = (double(vystupi_wait*4))/double(60);
				
				idle_co	+= time*co2idle;
				
				el_idle_co += (time*co2idle)*0.72;
				
				wait_idle_co	+= time_wait_f*co2idle;
			
				wait_el_idle_co += (time_wait_f*co2idle)*0.72;
				
				aktualny_pocet_cestujucich -= vystupi;
				
				passengers -= vystupi_wait;
				
				//cout << "Vystupilo: " << vystupi << endl;	
				//cout << "Vystupilo2: " << vystupi_wait << endl;	
			}
			cout << endl;
			
			nastupi = Uniform(0, ((90*pocet_vagonov)-aktualny_pocet_cestujucich));
			
			nastupi_wait = Uniform(0, ((90*pocet_vagonov)-passengers));
			
			celkovy_pocet += nastupi;
			celkovy_pocet_wait += nastupi_wait;
			
			//cout << "Nastupilo: " << nastupi << endl;
			//cout << "Nastupilo2: " << nastupi_wait << endl;
			
			time = (double(nastupi*4))/double(60);
			
			time_wait_f = (double(nastupi_wait*4))/double(60);

			aktualny_pocet_cestujucich += nastupi;
			
			passengers += nastupi_wait;

			idle_co	+= time*co2idle;
			
			el_idle_co += (time*co2idle)*0.72;
			
			wait_idle_co	+= time_wait_f*co2idle;
			
			wait_el_idle_co += (time_wait_f*co2idle)*0.72;
			
			tmp = 0;
			time_tmp = 0;
			cnt = 0;
			time_wait = 0;
			
			while(passengers < pozadovane_minimum_p && time_tmp < 30)
			{
				time_tmp += Uniform(1,5);
				tmp = Uniform(0, (90*pocet_vagonov) - passengers);
				passengers += tmp;
				cnt += tmp;

				if(!(passengers < pozadovane_minimum_p))
				{
					time_wait += time_tmp + (double(cnt*4))/double(60);
					spozdeni += time_tmp;
					celkovy_pocet_wait += cnt;
				}
				else if(!(time_tmp < 30))
				{
					time_wait += time_tmp;
					spozdeni += time_tmp;
					celkovy_pocet_wait += cnt;
					break;
				}
			}
			
			time_wait_f = time_wait;
			
			wait_idle_co	+= time_wait_f*co2idle;
			
			wait_el_idle_co += (time_wait_f*co2idle)*0.72;
			
			if(aktualny_pocet_cestujucich >= ((90*pocet_vagonov)/4)*3)
			{
				passenger_co += aktualny_pocet_cestujucich*co2p*length;
				el_passenger_co += (aktualny_pocet_cestujucich*co2p*length)*0.72;
			}
			else if((aktualny_pocet_cestujucich <= ((90*pocet_vagonov)/4)*3) && (aktualny_pocet_cestujucich >= ((90*pocet_vagonov)/2)))
			{
				passenger_co += aktualny_pocet_cestujucich*co2mp*length;
				el_passenger_co += (aktualny_pocet_cestujucich*co2mp*length)*0.72;
			} else 
			{
				passenger_co += aktualny_pocet_cestujucich*co2hp*length;
				el_passenger_co += (aktualny_pocet_cestujucich*co2hp*length)*0.72;
			}
			
			if(passengers >= ((90*pocet_vagonov)/4)*3)
			{
				wait_passenger_co += passengers*co2p*length;
				wait_el_passenger_co += (passengers*co2p*length)*0.72;
			}
			else if((passengers <= ((90*pocet_vagonov)/4)*3) && (passengers >= ((90*pocet_vagonov)/2)))
			{
				wait_passenger_co += passengers*co2mp*length;
				wait_el_passenger_co += (passengers*co2mp*length)*0.72;
			} else 
			{
				wait_passenger_co += passengers*co2hp*length;
				wait_el_passenger_co += (passengers*co2hp*length)*0.72;
			}
			
		}
		///////////////////////////////////////////////////////////////////////////////KONEC PASAZERU///////////////////////////////////////////////////////////////////////////////////
		
		cout << "STARTING LOAD: " << load << endl;
		double time_load = (double(load*5));
		double wait_time_load = time_load;
		int wait_load = load;
		time_tmp = 0;
		cnt = 0;
		int celkovy_naklad = load, celkovy_naklad_wait = load;
		while(wait_load < pozadovane_minimum_l && time_tmp < 100)
		{
			time_tmp += Uniform(1,5);
			tmp = Uniform(0, (130*pocet_vagonov) - wait_load);
			cnt += tmp;
			wait_load  += tmp;
			if(!(wait_load < pozadovane_minimum_l))
			{
				wait_time_load += time_tmp+(double(cnt*5));
				spozdeni_load += time_tmp;
				celkovy_naklad_wait+=cnt;
				break;
			}
			else if(!(time_tmp < 100))
			{
				wait_time_load += time_tmp;
				spozdeni_load += time_tmp;
				celkovy_naklad_wait+=cnt;
				break;
			}
		}
		
		double load_co = load*co2l*length;
		
		double wait_load_co = wait_load*co2l*length;
		
		double load_idle_co = time_load*co2idle;
		
		double wait_load_idle_co = wait_time_load*co2idle;

		double el_load_co = (load*co2l*length)*0.72;
		
		double wait_el_load_co = (wait_load*co2l*length)*0.72;
		
		double load_el_idle_co = (time_load*co2idle)*0.72;
		
		double wait_load_el_idle_co = (wait_time_load*co2idle)*0.72;

		int vylozil = 0, nalozil = 0;
		int vylozil_wait = 0, nalozil_wait = 0;
		
		cout << endl;
		
		for(int i = 1; i < pocet_zastavok; i++)
		{
			//cout << "Aktualny naklad: " << load << endl;
			//cout << "Aktualny naklad2: " << wait_load << endl;
			if(pocet_zastavok == 2 || i == pocet_zastavok-1)
			{
				time_load = double(load*5);
				wait_time_load = double(wait_load*5);
				load_idle_co	+= time_load*co2idle;
				wait_load_idle_co	+= wait_time_load*co2idle;
				load_el_idle_co += (time_load*co2idle)*0.72;
				wait_load_el_idle_co += (wait_time_load*co2idle)*0.72;
				break;
			}
			if(load != 0)
			{
				vylozil = Uniform(0, load);
				
				vylozil_wait = Uniform(0, wait_load);
				
				time_load = (double(vylozil*5));
				
				wait_time_load = (double(vylozil_wait*5));
				
				load_idle_co	+= time_load*co2idle;
				
				wait_load_idle_co	+= wait_time_load*co2idle;
				
				load_el_idle_co += (time_load*co2idle)*0.72;
				
				wait_load_el_idle_co += (wait_time_load*co2idle)*0.72;
				
				load -= vylozil;
				
				wait_load -= vylozil_wait;
				
				//cout << "Vylozil: " << vylozil << endl;
				//cout << "Vylozil2: " << vylozil_wait << endl;
			}
			nalozil = Uniform(0, ((130*pocet_vagonov)-load));
			
			nalozil_wait = Uniform(0, ((130*pocet_vagonov)-wait_load));
			
			celkovy_naklad+=nalozil;
			celkovy_naklad_wait+=nalozil_wait;
			
			//cout << "Nalozil: " << nalozil << endl;
			//cout << "Nalozil2: " << nalozil_wait << endl;
			
			load += nalozil;
			
			wait_load += nalozil_wait;
			
			time_load = (double(nalozil*5));
			
			wait_time_load = (double(nalozil_wait*5));
			
			load_idle_co	+= time_load*co2idle;
			wait_load_idle_co	+= wait_time_load*co2idle;
			load_el_idle_co += (time_load*co2idle)*0.72;
			wait_load_el_idle_co += (wait_time_load*co2idle)*0.72;
			
			tmp = 0;
			time_tmp = 0;
			cnt = 0;
			wait_time_load = 0;
			while(wait_load < pozadovane_minimum_l && time_tmp < 100)
			{
				time_tmp += Uniform(1,5);
				tmp = Uniform(0, (130*pocet_vagonov) - wait_load);
				cnt += tmp;
				wait_load  += tmp;
				if(!(wait_load < pozadovane_minimum_l))
				{
					wait_time_load += time_tmp+(double(cnt*5));
					spozdeni_load += time_tmp;
					celkovy_naklad_wait+=cnt;
					break;
				}
				else if(!(time_tmp < 100))
				{
					wait_time_load += time_tmp;
					spozdeni_load += time_tmp;
					celkovy_naklad_wait+=cnt;
					break;
				}
		}


			wait_load_idle_co	+= wait_time_load*co2idle;

			wait_load_el_idle_co += (wait_time_load*co2idle)*0.72;

			load_co += load*co2l*length;
			wait_load_co += wait_load*co2l*length;
			el_load_co += (load*co2l*length)*0.72;
			wait_el_load_co += (wait_load*co2l*length)*0.72;
		}
		
		
		////////////////////////////////////////////////////////////////////////////////////////////////////Konec nakladu/////////////////////////////////////////////////
		
		cout << endl;
		cout << "Vlak s nakladem vezl dohromady:" << celkovy_naklad << endl;
		cout << "Vlak co ceka s nakladem vezl dohromady:" << celkovy_naklad_wait << endl;
		cout << endl;
		
		
		cout << endl;
		cout << "Uhlikova stopa pro vlak, ktery neceka na minimum nakladu:" <<  endl;
		cout << endl;
		
		convert_weight(track_length, (load_co + load_idle_co), 1);
		convert_weight(track_length, ( el_load_co + load_el_idle_co ), 0);
		
		cout << endl;
		cout << "Uhlikova stopa pro vlak, ktery ceka na minimum nakladu:" <<  endl;
		cout << endl;
		
		cout << "Celkove zpozdeni vlaku je:" << spozdeni_load << endl;
		
		convert_weight(track_length, (wait_load_co + wait_load_idle_co), 1);
		convert_weight(track_length, ( wait_el_load_co + wait_load_el_idle_co ), 0);
		
		
		cout << endl;
		cout << "Vlak s pasazery vezl dohromady lidi:" << celkovy_pocet << endl;
		cout << "Vlak co ceka s pasazery vezl dohromady lidi:" << celkovy_pocet_wait << endl;
		cout << endl;
		
		cout << endl;
		cout << "Uhlikova stopa pro vlak, ktery neceka na minimum pasazeru:" <<  endl;
		cout << endl;
		
		convert_weight(track_length,(passenger_co + idle_co), 1);
		convert_weight(track_length,(el_passenger_co + el_idle_co), 0);
		
		cout << endl;
		cout << "Uhlikova stopa pro vlak, ktery ceka na minimum pasazeru:" <<  endl;
		cout << endl;
		
		cout << "Celkove zpozdeni vlaku je:" << spozdeni << endl;
		
		convert_weight(track_length,(wait_passenger_co + wait_idle_co), 1);
		convert_weight(track_length,(wait_el_passenger_co + wait_el_idle_co), 0);
		
		cout << endl;
		cout << "Uhlikova stopa pro vlak, ktery neceka na minimum pasazeru a nakladu:" <<  endl;
		cout << endl;
		
		
		double hybrid_idle = 0;
		
		if(idle_co/2>load_idle_co/2){
			hybrid_idle=idle_co/2;
		} else {
			hybrid_idle=load_idle_co/2;
		}
		
		double wait_hybrid_idle = 0;
		
		if(wait_idle_co/2>wait_load_idle_co/2){
			wait_hybrid_idle=wait_idle_co/2;
		} else {
			wait_hybrid_idle=wait_load_idle_co/2;
		}
		
		double el_hybrid_idle = 0;
		
		if(el_idle_co/2>load_el_idle_co/2){
			el_hybrid_idle=el_idle_co/2;
		} else {
			el_hybrid_idle=load_el_idle_co/2;
		}
		
		double wait_el_hybrid_idle = 0;
		
		if(wait_el_idle_co/2>wait_load_el_idle_co/2){
			wait_el_hybrid_idle=wait_el_idle_co/2;
		} else {
			wait_el_hybrid_idle=wait_load_el_idle_co/2;
		}
		
		cout << endl;
		cout << "Hybridni vlak s pasazery vezl dohromady lidi:" << celkovy_pocet/2 << ", dohromady nakladu:" << celkovy_naklad/2 << endl;
		cout << "Hybridni vlak co ceka s pasazery vezl dohromady lidi:" << celkovy_pocet_wait/2 << ", dohromady nakladu:" << celkovy_naklad_wait/2 << endl;
		cout << endl;
		
		convert_weight(track_length,(passenger_co/2 + load_co/2)+hybrid_idle, 1);
		convert_weight(track_length,(el_passenger_co/2 +el_load_co/2)+el_hybrid_idle, 0);
		
		cout << endl;
		cout << "Uhlikova stopa pro vlak, ktery ceka na minimum pasazeru a nakladu:" <<  endl;
		cout << endl;
		
		if(spozdeni > spozdeni_load){
		cout << "Celkove zpozdeni vlaku je:" << spozdeni << endl;
		} else {
		cout << "Celkove zpozdeni vlaku je:" << spozdeni_load << endl;
		}
		
		convert_weight(track_length,(wait_passenger_co/2 + wait_load_co/2)+wait_hybrid_idle, 1);
		convert_weight(track_length,(wait_el_passenger_co/2+wait_el_load_co/2)+wait_el_hybrid_idle, 0);
	
	return 0;
}