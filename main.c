#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "wiringx.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

#define Ant1_Goed_pin	0
#define Ant2_Goed_pin	1
#define Ant3_Goed_pin	2
#define Ant4_Goed_pin	3


char Welkom_str[2][50]={0};
//char Escape_str[100]={0};
char Ingave_str[4][20]={0};
char Antwoord_str[4][20]={0};

int col = 0;
int row = 0;
int antw_OK = 0;
int col_pos = 0;
int row_pos	= 0;
int old_col_pos = -1;
int old_row_pos	= -1;
int antw_nr = 0;
int old_col = 0;
int old_row = 0;
int time_hours = 1;
int time_minutes = 30;
int time_seconds = 0;
int old_sec=0;
int do_update=1;

static const char *LabSound = "SCI-FI Laboratory Sound.ogg";
static const char *Die = "I Expect you To Die.ogg";

//init numbers
char cijfers[10][20]={ 	{' ','=','=',' ','|',' ',' ','|',' ',' ',' ',' ','|',' ',' ','|',' ','=','=',' '},//0
						{' ',' ',' ',' ',' ',' ',' ','|',' ',' ',' ',' ',' ',' ',' ','|',' ',' ',' ',' '},//1
						{' ','=','=',' ',' ',' ',' ','|',' ','=','=',' ','|',' ',' ',' ',' ','=','=',' '},//2
						{' ','=','=',' ',' ',' ',' ','|',' ','=','=',' ',' ',' ',' ','|',' ','=','=',' '},//3
						{' ',' ',' ',' ','|',' ',' ','|',' ','=','=',' ',' ',' ',' ','|',' ',' ',' ',' '},//4
						{' ','=','=',' ','|',' ',' ',' ',' ','=','=',' ',' ',' ',' ','|',' ','=','=',' '},//5
						{' ','=','=',' ','|',' ',' ',' ',' ','=','=',' ','|',' ',' ','|',' ','=','=',' '},//6
						{' ','=','=',' ',' ',' ',' ','|',' ',' ',' ',' ',' ',' ',' ','|',' ',' ',' ',' '},//7
						{' ','=','=',' ','|',' ',' ','|',' ','=','=',' ','|',' ',' ','|',' ','=','=',' '},//8
						{' ','=','=',' ','|',' ',' ','|',' ','=','=',' ',' ',' ',' ','|',' ','=','=',' '}//9
};

static void Catch(int sig);
void Handle_Input(void);
void Draw_Text(void);
void File_Handling(void);
void Time_Handler(void);
void Finish(void);
void Loose(void);


int
main(int argc, char *argv[])
{
	int num = 0;
	int run = 1;

    int result = 0;
    int flags = MIX_INIT_OGG;
	int channel;

    signal(SIGINT, Catch);      /* Catch interrupts, for those who want to cheat! */

	//Init wiringx
	if(wiringXSetup("bananapi1", NULL) == -1) {
		wiringXGC();
		return -1;
	}
	pinMode(Ant1_Goed_pin, PINMODE_OUTPUT);
	pinMode(Ant2_Goed_pin, PINMODE_OUTPUT);
	pinMode(Ant3_Goed_pin, PINMODE_OUTPUT);
	pinMode(Ant4_Goed_pin, PINMODE_OUTPUT);
	
	//Read in the file
	File_Handling();
	
	//SDL functions
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Failed to init SDL\n");
        exit(1);
    }

    if (flags != (result = Mix_Init(flags))) {
        printf("Could not initialize mixer (result: %d).\n", result);
        printf("Mix_Init: %s\n", Mix_GetError());
        exit(1);
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096)==-1){
        printf("Could not open audio\n");
        printf("Mix_Init: %s\n", Mix_GetError());
        exit(1);
	}
	Mix_AllocateChannels(4);

    Mix_Music *music = Mix_LoadMUS(LabSound);
    //printf("Playing music\n");
    Mix_PlayMusic(music, 1);

	//Ncurses functions
    initscr();      /* initialize the curses library */
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
	raw();
    nonl();         /* tell curses not to do NL->CR/NL on output */
    cbreak();       /* take input chars one at a time, no wait for \n */
    noecho();         /* do not echo input */
	timeout(0);
	curs_set(0);

	if(has_colors()){
        start_color();
		init_pair(1, COLOR_GREEN, COLOR_BLACK);
		init_pair(2, COLOR_BLACK, COLOR_RED);
		init_pair(3, COLOR_BLACK, COLOR_GREEN);
		init_pair(4, COLOR_RED, COLOR_BLACK);
	}
	
	/*getmaxyx(stdscr,row,col);		/* get the number of rows and columns *
	Draw_Text(); 					/*Print the text on the screen*
	Handle_Input();
	refresh();*/
	clear();
	if (has_colors()){
		bkgd(COLOR_PAIR(1));
		attrset(COLOR_PAIR(1));
	}
	getmaxyx(stdscr, row, col);		/* get the number of rows and columns */
	mvprintw((row/2),((col-sizeof("Welkom bij MI6"))/2),"Welkom bij MI6");
	mvprintw((row/2)+1,((col-sizeof("Log in AUB!:"))/2),"Log in AUB!:");
	refresh();
	int wait = 1;
	while(wait){
	int c = getch();     /* refresh, accept single keystroke of input */
		if(c!=ERR){
			wait=0;
		}
	}

	clear();

    Mix_Chunk *JamesDie = Mix_LoadMUS(Die);
	channel = Mix_PlayChannel(-1, JamesDie, 0); 
	if(channel == -1) { fprintf(stderr, "Unable to play OGG file: %s\n", Mix_GetError()); } 
    
	while(Mix_Playing(channel) != 0);
	
	while (run){
		getmaxyx(stdscr, row, col);		/* get the number of rows and columns *
		if(row!=old_row || col!=old_col){ /* Handle a screen resize*
			old_col=col;
			old_row=row;
		}
		/**/
		Time_Handler();
		Handle_Input();
		Draw_Text();


		if(antw_OK>3){
			sleep(2);
			run=0;
		}
		
		if(time_seconds==0 && time_minutes==0 && time_hours==0){
			Loose();
		}
    }

    Finish();               /* we're done */
	
	return 0;
}

static void Catch(int sig){

	clear();
    if (has_colors()){
		bkgd(COLOR_PAIR(2));
		attrset(COLOR_PAIR(2));
	}
	mvprintw((row/2),((col-sizeof("YOU CANNOT ESCAPE LIKE THIS!!!"))/2),"YOU CANNOT ESCAPE LIKE THIS!!!");
	if(time_hours>0){
		if (time_minutes>10){
			time_minutes=time_minutes-10;
		}
		else if(time_minutes==10){
			time_hours--;
			time_minutes=0;
		}
		else{
			int remain_minutes = 10-time_minutes;
			time_hours--;
			time_minutes=60-remain_minutes;
		}
	}
	else if (time_minutes>10){
		time_minutes=time_minutes-10;
	}
	refresh();
	sleep(3);
	clear();
	
	/*Draw_Text();
	
	/*Remove here-under after test*
    endwin();

    exit(0);
	/**/
    signal(SIGINT, Catch);      /* Reinstate the catch */
	return;
}

void Handle_Input(void){

	int c = getch();     /* refresh, accept single keystroke of input */
	if(c!=ERR){
		do_update++;
		switch(c){
			case KEY_DC:
				Ingave_str[row_pos][col_pos]='\0';
				break;
			
			case KEY_ENTER:
			case 13:
				if(row_pos<3){
					row_pos++;
				}
				col_pos=0;
				break;//KEY_ENTER
				
			case KEY_BACKSPACE:
				if(col_pos>0){
					Ingave_str[row_pos][col_pos--]='\0';
				}
				break;//KEY_BACKSPACE
				
			case KEY_LEFT:
				if(col_pos>0){
					col_pos--;
				}
				break;
				
			case KEY_RIGHT:
				if(col_pos<19){
					col_pos++;
				}
				break;
				
			case KEY_UP:
				if(row_pos>0){
					row_pos--;
				}
				break;
			
			case KEY_DOWN:
				if(row_pos<3){
					row_pos++;
				}
				break;
				
			default:
				Ingave_str[row_pos][col_pos++]=c;
				break;
				
		}
	
	}
	//mvprintw((row/2)+1,(col)/2,"%d %d", c, KEY_DC);
}

void Draw_Text(void){
	if(do_update>0){
		do_update=0;
		if (has_colors()){
			bkgd(COLOR_PAIR(1));
			attrset(COLOR_PAIR(1));
		}
		mvprintw((row/2)-10, (col-strlen(Welkom_str[0]))/2, Welkom_str[0]);
		
		//Resterende tijd op het scherm zetten
		int Cur_number=0;
		Cur_number=time_hours;
		for(int y=0; y<20; y++){
			if(y<4){
				mvaddch((row/2)-7, (col/2)-(16-y), cijfers[Cur_number][y]);
			}
			else if(y<8){
				mvaddch((row/2)-6, (col/2)-(16-(y-4)), cijfers[Cur_number][y]);
			}
			else if(y<12){
				mvaddch((row/2)-5, (col/2)-(16-(y-8)), cijfers[Cur_number][y]);
			}
			else if(y<16){
				mvaddch((row/2)-4, (col/2)-(16-(y-12)), cijfers[Cur_number][y]);
			}
			else{
				mvaddch((row/2)-3, (col/2)-(16-(y-16)), cijfers[Cur_number][y]);
			}
		}

		mvaddch((row/2)-7, (col/2)-11, ' ');
		mvaddch((row/2)-6, (col/2)-11, ':');
		mvaddch((row/2)-5, (col/2)-11, ' ');
		mvaddch((row/2)-4, (col/2)-11, ':');
		mvaddch((row/2)-3, (col/2)-11, ' ');

		Cur_number=time_minutes/10;
		for(int y=0; y<20; y++){
			if(y<4){
				mvaddch((row/2)-7, (col/2)-(9-y), cijfers[Cur_number][y]);
			}
			else if(y<8){
				mvaddch((row/2)-6, (col/2)-(9-(y-4)), cijfers[Cur_number][y]);
			}
			else if(y<12){
				mvaddch((row/2)-5, (col/2)-(9-(y-8)), cijfers[Cur_number][y]);
			}
			else if(y<16){
				mvaddch((row/2)-4, (col/2)-(9-(y-12)), cijfers[Cur_number][y]);
			}
			else{
				mvaddch((row/2)-3, (col/2)-(9-(y-16)), cijfers[Cur_number][y]);
			}
		}

		Cur_number=time_minutes%10;
		for(int y=0; y<20; y++){
			if(y<4){
				mvaddch((row/2)-7, (col/2)-(5-y), cijfers[Cur_number][y]);
			}
			else if(y<8){
				mvaddch((row/2)-6, (col/2)-(5-(y-4)), cijfers[Cur_number][y]);
			}
			else if(y<12){
				mvaddch((row/2)-5, (col/2)-(5-(y-8)), cijfers[Cur_number][y]);
			}
			else if(y<16){
				mvaddch((row/2)-4, (col/2)-(5-(y-12)), cijfers[Cur_number][y]);
			}
			else{
				mvaddch((row/2)-3, (col/2)-(5-(y-16)), cijfers[Cur_number][y]);
			}
		}
		
		mvaddch((row/2)-7, col/2, ' ');
		mvaddch((row/2)-6, col/2, ':');
		mvaddch((row/2)-5, col/2, ' ');
		mvaddch((row/2)-4, col/2, ':');
		mvaddch((row/2)-3, col/2, ' ');


		Cur_number=time_seconds/10;
		for(int y=0; y<20; y++){
			if(y<4){
				mvaddch((row/2)-7, (col/2)+(2+y), cijfers[Cur_number][y]);
			}
			else if(y<8){
				mvaddch((row/2)-6, (col/2)+(2+(y-4)), cijfers[Cur_number][y]);
			}
			else if(y<12){
				mvaddch((row/2)-5, (col/2)+(2+(y-8)), cijfers[Cur_number][y]);
			}
			else if(y<16){
				mvaddch((row/2)-4, (col/2)+(2+(y-12)), cijfers[Cur_number][y]);
			}
			else{
				mvaddch((row/2)-3, (col/2)+(2+(y-16)), cijfers[Cur_number][y]);
			}
		}
		
		Cur_number=time_seconds%10;
		for(int y=0; y<20; y++){
			if(y<4){
				mvaddch((row/2)-7, (col/2)+(6+y), cijfers[Cur_number][y]);
			}
			else if(y<8){
				mvaddch((row/2)-6, (col/2)+(6+(y-4)), cijfers[Cur_number][y]);
			}
			else if(y<12){
				mvaddch((row/2)-5, (col/2)+(6+(y-8)), cijfers[Cur_number][y]);
			}
			else if(y<16){
				mvaddch((row/2)-4, (col/2)+(6+(y-12)), cijfers[Cur_number][y]);
			}
			else{
				mvaddch((row/2)-3, (col/2)+(6+(y-16)), cijfers[Cur_number][y]);
			}
		}
		/**/
		mvprintw((row/2)-1, (col-strlen(Welkom_str[1]))/2, Welkom_str[1]);

		
		antw_OK=0;
		/* Antwoord 1 op scherm zetten */
		mvprintw((row/2),((col)/2)-((sizeof(Antwoord_str[0])/2)+2),"1:");
		if(strncmp(Antwoord_str[0], Ingave_str[0], sizeof(Antwoord_str[0]))){
			attrset(COLOR_PAIR(4));
			mvprintw((row/2),(col/2)+(sizeof(Antwoord_str[0])/2)+2,"FOUT");
			attrset(COLOR_PAIR(1));
			digitalWrite(Ant1_Goed_pin, LOW);
		}
		else{
			mvprintw((row/2),(col/2)+(sizeof(Antwoord_str[0])/2)+2,"GOED");
			antw_OK++;
			digitalWrite(Ant1_Goed_pin, HIGH);
		}
		mvprintw((row/2),(col-sizeof(Antwoord_str[0]))/2,Ingave_str[0]);

		/* Antwoord 2 op scherm zetten */
		mvprintw((row/2)+1,((col)/2)-((sizeof(Antwoord_str[1])/2)+2),"2:");
		if(strncmp(Antwoord_str[1], Ingave_str[1], sizeof(Antwoord_str[1]))){
			attrset(COLOR_PAIR(4));
			mvprintw((row/2)+1,(col/2)+(sizeof(Antwoord_str[1])/2)+2,"FOUT");
			attrset(COLOR_PAIR(1));
			digitalWrite(Ant2_Goed_pin, LOW);
		}
		else{
			mvprintw((row/2)+1,(col/2)+(sizeof(Antwoord_str[1])/2)+2,"GOED");
			antw_OK++;
			digitalWrite(Ant2_Goed_pin, HIGH);
		}
		mvprintw((row/2)+1,(col-sizeof(Antwoord_str[1]))/2,Ingave_str[1]);

		/* Antwoord 3 op scherm zetten */
		mvprintw((row/2)+2,((col)/2)-((sizeof(Antwoord_str[2])/2)+2),"3:");
		if(strncmp(Antwoord_str[2], Ingave_str[2], sizeof(Antwoord_str[2]))){
			attrset(COLOR_PAIR(4));
			mvprintw((row/2)+2,(col/2)+(sizeof(Antwoord_str[2])/2)+2,"FOUT");
			attrset(COLOR_PAIR(1));
			digitalWrite(Ant3_Goed_pin, LOW);
		}
		else{
			mvprintw((row/2)+2,(col/2)+(sizeof(Antwoord_str[2])/2)+2,"GOED");
			antw_OK++;
			digitalWrite(Ant3_Goed_pin, HIGH);
		}
		mvprintw((row/2)+2,(col-sizeof(Antwoord_str[2]))/2,Ingave_str[2]);
		
		/* Antwoord 4 op scherm zetten */
		mvprintw((row/2)+3,((col)/2)-((sizeof(Antwoord_str[3])/2)+2),"4:");
		if(strncmp(Antwoord_str[3], Ingave_str[3], sizeof(Antwoord_str[3]))){
			attrset(COLOR_PAIR(4));
			mvprintw((row/2)+3,(col/2)+(sizeof(Antwoord_str[3])/2)+2,"FOUT");
			attrset(COLOR_PAIR(1));
			digitalWrite(Ant4_Goed_pin, LOW);
		}
		else{
			mvprintw((row/2)+3,(col/2)+(sizeof(Antwoord_str[3])/2)+2,"GOED");
			antw_OK++;
			digitalWrite(Ant4_Goed_pin, HIGH);
		}
		mvprintw((row/2)+3,(col-sizeof(Antwoord_str[3]))/2,Ingave_str[3]);

		//Cursor op scherm zetten
		if (has_colors()){
			//bkgd(COLOR_PAIR(3));
			attrset(COLOR_PAIR(3));
		}
		//if(row_pos!=old_row_pos || col_pos!=old_col_pos){
			//mvprintw((row/2)+old_row_pos, ((col/2)-(sizeof(Antwoord_str[0])/2)+old_col_pos), " ");
			//old_col_pos=col_pos;
			//old_row_pos=row_pos;
			if(Ingave_str[row_pos][col_pos]=='\0'){
				mvaddch((row/2)+row_pos, ((col/2)-(sizeof(Antwoord_str[0])/2)+col_pos), ' ');
			}
			else{
				mvaddch((row/2)+row_pos, ((col/2)-(sizeof(Antwoord_str[0])/2)+col_pos), (const chtype)Ingave_str[row_pos][col_pos]);
			}
		//}
		if (has_colors()){
			//bkgd(COLOR_PAIR(1));
			attrset(COLOR_PAIR(1));
		}

		wrefresh(stdscr);
	

	}
}

void File_Handling(void){
	FILE *fp;

	fp = fopen("escape.txt", "r");
	//char buff[511];
	
	if(fp==NULL){
		clear();
		if (has_colors()){
			bkgd(COLOR_PAIR(3));
			attrset(COLOR_PAIR(3));
		}
		mvprintw((row/2),((col-sizeof("Kan escape.txt niet openen"))/2),"Kan escape.txt niet openen");
		refresh();
		sleep(3);

		endwin();

		fclose(fp);
		exit(-1);
	}
	//else{
		fgets(Welkom_str[0], 50, (FILE*)fp);
		for(int i=0; i<50; i++){
			if(Welkom_str[0][i]=='\n' || Welkom_str[0][i]=='\r'){
				Welkom_str[0][i]='\0';
			}
		}
		fgets(Welkom_str[1], 50, (FILE*)fp);
		for(int i=0; i<50; i++){
			if(Welkom_str[1][i]=='\n' || Welkom_str[1][i]=='\r'){
				Welkom_str[1][i]='\0';
			}
		}
		fgets(Antwoord_str[0], 20, (FILE*)fp);
		for(int i=0; i<20; i++){
			if(Antwoord_str[0][i]=='\n' || Antwoord_str[0][i]=='\r'){
				Antwoord_str[0][i]='\0';
			}
		}
		fgets(Antwoord_str[1], 20, (FILE*)fp);
		for(int i=0; i<20; i++){
			if(Antwoord_str[1][i]=='\n' || Antwoord_str[1][i]=='\r'){
				Antwoord_str[1][i]='\0';
			}
		}
		fgets(Antwoord_str[2], 20, (FILE*)fp);
		for(int i=0; i<20; i++){
			if(Antwoord_str[2][i]=='\n' || Antwoord_str[2][i]=='\r'){
				Antwoord_str[2][i]='\0';
			}
		}
		fgets(Antwoord_str[3], 20, (FILE*)fp);
		for(int i=0; i<20; i++){
			if(Antwoord_str[3][i]=='\n' || Antwoord_str[3][i]=='\r'){
				Antwoord_str[3][i]='\0';
			}
		}
	//}
	
	fclose(fp);

}

void Time_Handler(void){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	if(old_sec!=(int)tm.tm_sec){
		do_update++;
		old_sec=(int)tm.tm_sec;
		if (time_hours>0 && time_minutes==0 && time_seconds==0){
			time_seconds=59;
			time_minutes=59;
			time_hours--;
		}
		else if(time_seconds==0 && time_minutes>0){
			time_seconds=59;
			time_minutes--;
		}
		else if(time_seconds>0){
			time_seconds--;
		}
	}
}

void Finish(void){
	clear();
    if (has_colors()){
		bkgd(COLOR_PAIR(3));
        attrset(COLOR_PAIR(3));
    }
	mvprintw((row/2),((col-sizeof("Gefeliciteerd!!!"))/2),"Gefeliciteerd!!!");
	refresh();
	int wait = 1;
	while(wait){
	int c = getch();     /* refresh, accept single keystroke of input */
		if(c!=ERR){
			wait=0;
		}
	}

    endwin();
	wiringXGC();

    exit(0);
}

void Loose(void){
	clear();
    if (has_colors()){
		bkgd(COLOR_PAIR(4));
        attrset(COLOR_PAIR(4));
    }
	mvprintw((row/2),((col-sizeof("Tijd is op!"))/2),"Tijd is op!");
	refresh();
	int wait = 1;
	while(wait){
	int c = getch();     /* refresh, accept single keystroke of input */
		if(c!=ERR){
			wait=0;
		}
	}
	
	echo();
    endwin();
	wiringXGC();

    exit(0);
}