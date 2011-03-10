/* BATTAGLIA NAVALE REMOTA - SERVER */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h> 
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>

#define MAX_DIM 12
#define GIOCATORI 4
#define PORT 6000

/* VARIABILI */
struct sockaddr_in server;
struct sockaddr client[4];
char buffer[MAX_DIM], buffer2[MAX_DIM], timeout;
int length, ds_sock, des[4], numG, i, j, k, r, q, x, avversario, gioco[4], intbuffer[3], inGioco, contE, retW, ritAut, ERR;
/* fine variabili */

/* AMBIENTE */
void ambiente(){
	ds_sock = socket(AF_INET, SOCK_STREAM, 0);
	length = sizeof(client[1]);
	ERR = 0;
	inGioco = 0;
	gioco[0]=0; gioco[1]=0; gioco[2]=0; gioco[3]=0; //flag=1 se 1 l'iesimo giocatore è in gioco
}
/* fine ambiente */


/* GESTORE ERRORI */
void gestoreErrori(){
	printf("errore sconosciuto. L'applicazione verrà terminata\n");
	for(q=0; q<numG; q++){
		close(des[q]);
	}
	ERR=1;
}
/* fine gestore errori */

/* GESTORE BROKEN PIPE */
void gestoreBrokPipe(){
	signal(SIGPIPE, gestoreBrokPipe);
	puts("un utente si è disconnesso, pertanto la partita terminerà..");
	inGioco=0;
	ERR=1;
	for(q=0; q<numG; q++){
		close(des[q]);
	}
}
/* fine gestore broken pipe */

/* GESTORE TERMINAZIONE */
void gestoreTerminazione(){
	printf("\nterminazione forzata dell'applicazione in corso..\n");
	printf("attendere chiusura socket..\n");
	for(q=0; q<numG; q++){
		close(des[q]);
	}
	printf("terminazione completata..\n\n");
	exit(1);
}
/* fine gestore terminazione */


/* GESTORE ELIMINAZIONE */
void gestoreElim(){
	printf("concorrente %d eliminato\n\n", avversario);
	gioco[avversario]=0;
	close(des[avversario]);
	inGioco--;
	intbuffer[0]=inGioco;
	printf("sono rimasti %d giocatori\n\n",inGioco);
	
	for(contE=0; contE<numG; contE++){
		if(contE!=avversario && gioco[contE]==1){
			write(des[contE], buffer, 1);
			write(des[contE], intbuffer, 1);
		}
	}
}

/* GESTORE TIMEOUT */
void gestoreTimeout(){
	printf("nessuna connessione. Aspettare ancora? (y/n)\n");
	scanf("%s", &timeout);
	if(timeout=='n'){
		printf("chiusura programma..\n\n");
		exit(0);
	}
	else alarm(120);
	signal(SIGALRM, gestoreTimeout);
}	
/* fine gestore timeout */


/* CONVERTI */ 
int converti(char c){
	if(c=='a') return 0;
	if(c=='b') return 1;
	if(c=='c') return 2;
	if(c=='d') return 3;
	if(c=='e') return 4;
	if(c=='f') return 5;
	if(c=='g') return 6;
	if(c=='h') return 7;
	if(c=='i') return 8;
	if(c=='l') return 9;
	else return -1;
}
/* fine funzione converti*/


/* ASSEGNAZIONE */
void assegna(){numG = (int)buffer[0];
	bzero((char*)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(6000);
	server.sin_addr.s_addr = INADDR_ANY;
	bind(ds_sock, (struct sockaddr*)&server, sizeof(server));
	listen(ds_sock, GIOCATORI);
}
/* fine funzione assegnazione*/


/* AUTENTICAZIONE UTENTI */
int autenticazione(){
	while((des[0] = accept(ds_sock, &client[0], &length)) == -1){
		puts("attendo utenti");
		if(errno==EINTR){
			printf("errore nella system call dovuto a un segnale\n");
		}
		if(errno==EBADF || errno==EPIPE) break;
	}
	if(errno!=EBADF || errno!=EPIPE) puts("utente 0 connesso\n");
	signal(SIGALRM, SIG_IGN);
	do{	
		intbuffer[0]=0;
		retW = write(des[0], intbuffer, 1);
		if(retW==-1){
			if(errno==EBADF || errno==EPIPE) puts("l'utente ha chiuso la connessione prima di inserire i giocatori");
			if(errno==EINTR) puts("errore nella chiamata a sistema");
			return 0;
		}
		retW = read(des[0], buffer, 1);
		if(retW<1){
			if(errno== EBADF || errno==EPIPE) puts("l'utente ha chiuso la connessione prima di inserire i giocatori");
			if(errno==EINTR) puts("errore nella chiamata a sistema");
			return 0;
		}
		numG = (int)buffer[0]-48;
	}while(numG<2 || numG>GIOCATORI); //utente setta num giocatori
	
	printf("si gioca in %d\n",numG);
	gioco[0]=1;
		
	/*attendo connesioni degli altri utenti e creo i socket*/
	for(i=1; i<numG; i++){
		printf("attendo utente %d..\n",i);
		des[i] = accept(ds_sock, &client[i], &length);
		intbuffer[0]=i;
		printf("utente %d connesso!\n",i);
		gioco[i]=1;
		retW = write(des[i], intbuffer, 1);
		if(retW==-1){
			if(errno==EBADF || errno==EPIPE) puts("l'utente ha chiuso la connessione prima di inserire i giocatori");
			if(errno==EINTR) puts("errore nella chiamata a sistema");
			return 0;
		}
	}
	
	/*annuncio agli utenti, tranne che a 0, il num di giocatori*/
	i=0;
	intbuffer[0]=numG;
	for(i=1; i<numG; i++){
		retW = write(des[i], intbuffer, 1);
		if(retW==-1){
			if(errno==EBADF || errno==EPIPE) printf("l'utente %d ha chiuso la connessione\n",i);
			if(errno==EINTR) puts("errore nella chiamata a sistema");
			return 0;
		}
	}
	/*gli utenti possono posizionare le navi*/
	buffer[0]='p';
	for(i=0; i<numG; i++){
		retW = write(des[i], buffer, 1);
		if(retW==-1){
			if(errno==EBADF || errno==EPIPE) printf("l'utente %d ha chiuso la connessione\n",i);
			if(errno==EINTR) puts("errore nella chiamata a sistema");
			return 0;
		}
	}
	for(i=0; i<numG; i++){
		retW = read(des[i], buffer, 1);
		if(retW<1){
			if(errno==EBADF || errno==EPIPE) printf("l'utente %d ha chiuso la connessione\n",i);
			if(errno==EINTR) puts("errore nella chiamata a sistema");
			return 0;
		}
	}
	puts("tutti gli utenti hanno sistemato le navi!");
	inGioco = numG;
	return 1;
}
/* fine funzione autenticazione utenti */


/* PARTITA */
void partita(){
	while(inGioco>1){
		for(i=0; i<numG; i++){
			if(gioco[i]==1){ //solo se è ancora in gioco può arrivare il suo turno
				intbuffer[0]=i;
				if(ERR!=1)printf("è il turno di %d\n",intbuffer[0]);
				/*annuncio a tutti il turno*/
				for(x=0; x<numG; x++){
					retW = write(des[x], intbuffer, 1);
					if(retW==-1 && ERR==1) printf("ERRORE: errno=%d, ret=%d\n",errno,retW);
				}
				/*prendo la mossa dal giocatore i*/
				retW=read(des[i], intbuffer, 1);
				avversario = intbuffer[0]; //utente attaccato
				if(avversario!=i && ERR!=1) printf("avversario: %d\n",avversario);
				read(des[i], intbuffer, 1);
				j = intbuffer[0]; //colonna
				read(des[i], intbuffer, 1);				
				k = intbuffer[0]; //riga
				
				/*mando conferma mossa a i*/
				buffer2[0] = 'y';
				write(des[i], buffer2, 1); 
				
				/*annuncio a tutti chi è attaccato*/
				intbuffer[0]=avversario;
				if(avversario!=i && ERR!=1) puts("annuncio l'avversario a tutti");
				for(x=0; x<numG; x++){
					if(x!=i) write(des[x], intbuffer, 1);
				}
				if(avversario!=i && ERR!=1) printf("attacco: %d %d contro %d\n",k,j,avversario);
				
				/*mando all'avversario la cordinata j e poi la k*/
				if(avversario!=i && ERR!=1) puts("inoltro la mossa all'avversario");
				intbuffer[0]=j;
				write(des[avversario], intbuffer, 1);
				intbuffer[0]=k;
				write(des[avversario], intbuffer, 1);
	
				/*leggo la risposta dall'avversario*/
				read(des[avversario], buffer, 1);
				if(avversario!=i && ERR!=1) puts("l'avversario ha risposto");				
				
				/*controllo che il giocatore sia o non sia stato elimnato*/
				if(buffer[0]=='e'){
					printf("il giocatore %d è stato eliminato, avvio la procedura di eliminazione..\n",avversario);
					gestoreElim();
				}
				else{
					if(avversario!=i && ERR!=1) puts("annuncio la mossa ai giocatori");
					for(r=0; r<numG; r++){
						if(r!=avversario) write(des[r], buffer, 1);
					}
				}
				/*controllo d'errore*/
			}
		}
	}
}
/* fine funzione partita */


/* MAIN */
int main(){
	printf("\n\n                SERVER BATTAGLIA NAVALE (on-line!)\n\n");
		
	/*armo i segnali*/
	signal(SIGTERM, gestoreTerminazione);
	signal(SIGINT, gestoreTerminazione);
	signal(SIGQUIT, gestoreTerminazione);
	signal(SIGHUP, gestoreTerminazione);

	signal(SIGILL, gestoreErrori);
	signal(SIGSEGV, gestoreErrori);
	signal(SIGXCPU, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);

	/*inizializzo variabili*/	
	printf("carico l'ambiente..\n");
	ambiente();
		
	/*bind, listen*/
	assegna();
	
	while(1){
		/*nuova partita*/
		signal(SIGPIPE, SIG_IGN);
		printf("NUOVA PARTITA\n");
		printf("attendo connessione primo utente..\n");
		alarm(120);
		signal(SIGALRM, gestoreTimeout);
		
		/*attendo connessione utenti*/
		ritAut = autenticazione();
		signal(SIGPIPE, gestoreBrokPipe);
		signal(SIGALRM, SIG_IGN);
		
		/*inizia la partita*/
		if(ritAut==1) printf("Si inizia!!\n\n");
		else{
			for(i=0; i<numG; i++){
				close(des[i]);
			}
		}
		partita();
		if(errno==EPIPE) puts("partita terminata a causa della disconnessione di un utente");
		else puts("partita finita, disconnessione socket in corso..");
		printf("disconnessione completata!\n\n");
		ERR=0;
	} /*chiudo while(1)*/
}
/* fine main */
/*************/
