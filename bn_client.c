/* BATTAGLIA NAVALE REMOTA - CLIENT */

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

#define PORT 6000
#define MAX_DIM 1024

/* VARIABILI */
int mat[10][10], des, fc, j, k, i, x, y, codice, cfe, difJ, difK, cod, cont, ret, attaccante, ritornoRiempi, intbuffer[4], intbuffer2[4], numG, gioco[4];
char buffer[MAX_DIM], bs, p, p2, *ipAdd;
struct sockaddr_in server;
struct hostent *hp;
/* fine variabili*/


/* AMBIENTE */
void ambiente(){
	des = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	for(j=0; j<10; j++){
		for(k=0; k<10; k++){
			mat[j][k]=0;
		}
	}
	ritornoRiempi=1;
	cont=0;
	ipAdd = malloc(64);
}
/* fine ambiente */

/***********************************************************************************************/

/* GESTORE ERRORI */
void gestoreErrori(){
	puts("per un errore sconosciuto l'applicazione ha smesso di funzionare e verrà dismessa\n");
	close(des);
	free(ipAdd);
	exit(EXIT_FAILURE);
}
/* fine gestore errori */

/* GESTORE BROKEN PIPE */
void gestoreBrokPipe(){
	puts("un utente si è ritirato o il server è stato dismesso. La partita verrà terminata\n");
	close(des);
	exit(1);
}
/* fine gestore broken pipe */

/* GESTORE TERMINAZIONE */
void gestoreTerminazione(){
	printf("\nl'applicazione è stata dismessa, chiusura socket in corso..\n");
	close(des);
	free(ipAdd);
	printf("applicazione terminata\n\n");
	exit(1);
}
/* fine gestore segnali */

/* CONTROLLO READ */
void controllo(){
	if(ret==0) gestoreBrokPipe();
	if(ret==-1){
		if(errno==EINTR){
			puts("system call fallita a causa di un segnale. L'applicazione verrà dismessa");
			exit(EXIT_FAILURE);
		}
		if(errno==EBADF){
			puts("descrittore di socket non valido. L'applicazione verrà dismessa");
			exit(EXIT_FAILURE);
		}
		else gestoreErrori();
	}
}
/* fine funzione controllo read */

/************************************************************************************************/

/* RIEMPI MATRICE */
int riempi(char vo, int dim, int riga, int col, int cod){
	ritornoRiempi=1;
	if(vo=='v'){
		for(x=riga; x<riga+dim; x++){
			if(mat[x][col]!=0) ritornoRiempi=-1;
			else mat[x][col]=cod;
		}
	}
	if(vo=='o'){
		for(y=col; y<col+dim; y++){
			if(mat[riga][y]!=0) ritornoRiempi=-1;
			else mat[riga][y]=cod;
		}
	}
	if(ritornoRiempi==-1){
		for(x=0; x<10; x++){
			for(y=0; y<10; y++){
				if(mat[x][y]==cod) mat[x][y]=0;
			}
		}
	} 
	return ritornoRiempi;
}
/* fine funzione riempi matrice */


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
/* fine funzione converti */

/* INVERTI */
char inverti(int n){
	if(n==0) return 'a';
	if(n==1) return 'b';
	if(n==2) return 'c';
	if(n==3) return 'd';
	if(n==4) return 'e';
	if(n==5) return 'f';
	if(n==6) return 'g';
	if(n==7) return 'h';
	if(n==8) return 'i';
	if(n==9) return 'l';
	else return 'x';
}
/* fine funzione converti */

/* STAMPA */
void stampa(){
	printf("----------------------------\n    A B C D E F G H I L\n0  |%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|\n1  |%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|   legenda:\n2  |%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|   1 nave da 5 (cod:1)\n3  |%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|   1 nave da 4 (cod:2)\n4  |%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|   2 navi da 3 (cod:3,4)\n5  |%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|   2 navi da 2 (cod:5,6)\n6  |%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|\n7  |%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|\n8  |%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|\n9  |%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|\n----------------------------\n\n", mat[0][0]);
}
/* fine funzione stampa */

/* POSIZIONA NAVI */
void posizionaNavi(){
	puts("PUOI POSIZIONARE LE NAVI\n");
	/***********/
	/*nave da 5*/
	codice=1;
	puts("la tua griglia è vuota, eccola:\n");
	stampa();
	printf("posiziona la nave da 5 (codice nave = 1): \n");
	do{
		printf("orizzontale(o) o verticale(v)?\n");
		scanf("%s", &p);
		if(p!='v' && p!='o') printf("ERRORE: valore non ammesso\n\n");
	}while(p!='v' && p!='o');
	puts("");
	if(p=='v'){ //se verticale
		do{
			printf("seleziona la colonna (A-L)\n");
			scanf("%s", &p2);
			k=converti(p2);
			printf("colonna = %d\n\n",k);
			if(k<0 || k>9) printf("ERRORE: valore non ammesso = %d\n\n",k);
		}while(k<0 || k>9);
		do{
			printf("seleziona la riga (ammesse da 0 a 5)\n");
			scanf("%s", &p2);
			j=(int)p2-48;
			printf("riga = %d\n\n",j);
			if(p2<'0' || p2>'5') printf("ERRORE: valore non ammesso = %d\n\n",j);
		}while(p2<'0' || p2>'5');
		ret=riempi(p,5,j,k, codice);
		if(ret==-1){
			puts("ERRORE: nave non inserita\n");
			gestoreErrori();
		}
		else puts("nave inserita corretamente!\n\n");
	}
	
	else{ //se orizzontale
		do{
			printf("seleziona la colonna (A-F)\n");
			scanf("%s", &p2);
			k=converti(p2);
			printf("colonna = %d\n\n",k);
			if(k<0 || k>5) printf("ERRORE: valore non ammesso = %d\n\n",k);			
		}while(k<0 || k>5);
		do{
			printf("seleziona la riga (0-9)\n");
			scanf("%s", &p2);
			j=(int)p2-48;
			printf("riga = %d\n\n",j);
			if(p2<'0' || p2>'9') printf("ERRORE: valore non ammesso = %d\n\n",j);
		}while(p2<'0' || p2>'9');
		ret=riempi(p,5,j,k, codice);
		if(ret==-1){
			puts("ERRORE: nave non inserita\n");
			gestoreErrori();
		}
		else puts("nave inserita corretamente!\n\n");
	}

	/***********/
	/*nave da 4*/
	codice=2;
	stampa();
	printf("\nposiziona la nave da 4 (codice nave = 2): \n");
	do{ //se ho un una sovrapposizione di navi ret=-1 e rifaccio effettuare l'inserimento della nave
		do{
			printf("orizzontale(o) o verticale(v)?\n");
			scanf("%s", &p);
			if(p!='v' && p!='o') printf("ERRORE: valore non ammesso\n\n");
		}while(p!='v' && p!='o');
		puts("");
		if(p=='v'){ //se verticale
			do{
				do{
					printf("seleziona la colonna (A-L)\n");
					scanf("%s", &p2);
					k=converti(p2);
					printf("colonna = %d\n\n",k);
					if(k<0 || k>9) printf("ERRORE: valore non ammesso = %d\n\n",k);
				}while(k<0 || k>9);
				do{
					printf("seleziona la riga (ammesse da 0 a 6)\n");
					scanf("%s", &p2);
					j=(int)p2-48;
					printf("riga = %d\n\n",j);
					if(p2<'0' || p2>'6') printf("ERRORE: valore non ammesso = %d\n\n",j);
				}while(p2<'0' || p2>'6');
				ret=riempi(p,4,j,k, codice);
				if(ret==-1){
					printf("ERRORE: non puoi sovrapporre due navi!\n");
					stampa();
				}
				else puts("nave inserita corretamente!\n\n");
			}while(ret==-1);
		}
		
		else{ //se orizzontale
			do{
				printf("seleziona la colonna (A-G)\n");
				scanf("%s", &p2);
				k=converti(p2);
				printf("colonna = %d\n\n",k);
				if(k<0 || k>6) printf("ERRORE: valore non ammesso = %d\n\n",k);
			}while(k<0 || k>6);
			do{
				printf("seleziona la riga (0-9)\n");
				scanf("%s", &p2);
				j=(int)p2-48;
				printf("riga = %d\n\n",j);
				if(p2<'0' || p2>'9') printf("ERRORE: valore non ammesso = %d\n\n",j);	
			}while(p2<'0' || p2>'9');
			ret=riempi(p,4,j,k, codice);
			if(ret==-1){
					printf("ERRORE: non puoi sovrapporre due navi!\n");
					stampa();
				}
			else puts("nave inserita corretamente!\n\n");
		}
	}while(ret==-1);

	/***********/
	/*navi da 3*/
	while(cont!=2){
		cont++;
		codice++;
		stampa();
		printf("\nposiziona la %d° nave da 3 (codice nave = %d): \n",cont,codice);
		do{ //se ho un una sovrapposizione di navi ret=-1 e rifaccio effettuare l'inserimento della nave
			do{
				printf("orizzontale(o) o verticale(v)?\n");
				scanf("%s", &p);
				if(p!='v' && p!='o') printf("ERRORE: valore non ammesso\n\n");
			}while(p!='v' && p!='o');
			puts("");
			if(p=='v'){ //se verticale
				do{
					printf("seleziona la colonna (A-L)\n");
					scanf("%s", &p2);
					k=converti(p2);
					printf("colonna = %d\n\n",k);
					if(k<0 || k>9) printf("ERRORE: valore non ammesso = %d\n\n",k);
				}while(k<0 || k>9);
				do{
					printf("seleziona la riga (ammesse da 0 a 7)\n");
					scanf("%s", &p2);
					j=(int)p2-48;
					printf("riga = %d\n\n",j);
					if(p2<'0' || p2>'7') printf("ERRORE: valore non ammesso = %d\n\n",j);
				}while(p2<'0' || p2>'7');
				ret=riempi(p,3,j,k, codice);
				if(ret==-1){
					printf("ERRORE: non puoi sovrapporre due navi!\n");
					stampa();
				}
				else puts("nave inserita corretamente!\n\n");
			}
			
			else{ //se orizzontale
				do{
					printf("seleziona la colonna (A-H)\n");
					scanf("%s", &p2);
					k=converti(p2);
					printf("colonna = %d\n\n",k);
					if(k<0 || k>7) printf("ERRORE: valore non ammesso = %d\n\n",k);
				}while(k<0 || k>7);
				do{
					printf("seleziona la riga (0-9)\n");
					scanf("%s", &p2);
					j=(int)p2-48;
					printf("riga = %d\n\n",j);
					if(p2<'0' || p2>'9') printf("ERRORE: valore non ammesso = %d\n\n",j);	
				}while(p2<'0' || p2>'9');
				ret=riempi(p,3,j,k, codice);
				if(ret==-1){
					printf("ERRORE: non puoi sovrapporre due navi!\n");
					stampa();
				}
				else puts("nave inserita corretamente!\n\n");
			}
		}while(ret==-1);
	}
	cont=0;

	/***********/
	/*navi da 2*/
	while(cont!=2){
		cont++;
		codice++;
		stampa();
		printf("\nposiziona la %d° nave da 2 (codice nave = %d): \n",cont,codice);
		do{ //se ho un una sovrapposizione di navi ret=-1 e rifaccio effettuare l'inserimento della nave
			do{
				printf("orizzontale(o) o verticale(v)?\n");
				scanf("%s", &p);
				if(p!='v' && p!='o') printf("ERRORE: valore non ammesso\n\n");
			}while(p!='v' && p!='o');
			puts("");
			if(p=='v'){ //se verticale
				do{
					printf("seleziona la colonna (A-L)\n");
					scanf("%s", &p2);
					k=converti(p2);
					printf("colonna = %d\n\n",k);
					if(k<0 || k>9) printf("ERRORE: valore non ammesso = %d\n\n",k);
				}while(k<0 || k>9);
				do{
					printf("seleziona la riga (ammesse da 0 a 8)\n");
					scanf("%s", &p2);
					j=(int)p2-48;
					printf("riga = %d\n\n",j);
					if(p2<'0' || p2>'8') printf("ERRORE: valore non ammesso = %d\n\n",j);
				}while(p2<'0' || p2>'8');
				ret=riempi(p,2,j,k, codice);
				if(ret==-1){
					printf("ERRORE: non puoi sovrapporre due navi!\n");
					stampa();
				}
				else puts("nave inserita corretamente!\n\n");
			}
			
			else{ //se orizzontale
				do{
					printf("seleziona la colonna (A-I)\n");
					scanf("%s", &p2);
					k=converti(p2);
					printf("colonna = %d\n\n",k);
					if(k<0 || k>8) printf("ERRORE: valore non ammesso = %d\n\n",k);
				}while(k<0 || k>8);
				do{
					printf("seleziona la riga (0-9)\n");
					scanf("%s", &p2);
					j=(int)p2-48;
					printf("riga = %d\n\n",j);
					if(p2<'0' || p2>'9') printf("ERRORE: valore non ammesso = %d\n\n",j);	
				}while(p2<'0' || p2>'9');
				ret=riempi(p,2,j,k, codice);
				if(ret==-1){
					printf("ERRORE: non puoi sovrapporre due navi!\n");
					stampa();
				}	
				else puts("nave inserita corretamente!\n\n");
			}
		}while(ret==-1);
	}
	cont=0;
	ret=1;
	puts("HAI INSERITO TUTTE LE NAVI! Attendere prego..\n");
	
}
/* fine funzione posiziona navi */


/* CONNESSIONE*/
void connessione(){
	do{
		printf("inserisci l'ip del server a cui connetterti..\n");
		scanf("%s", ipAdd);
		hp = gethostbyname(ipAdd);
		memcpy(&server.sin_addr, hp->h_addr, 4);
		fc = connect(des, (struct sockaddr*)&server, sizeof(server));
		if(fc==-1) printf("errore nella connessione al server..\n");
		else puts("connessione stabilita..\n");
	}while(fc==-1);
	fc=1;

	/*connessione avvenuta*/
	puts("attendi comunicazione dal server..");
	
	ret=read(des, intbuffer, 1);
	controllo();
	puts("lettura eseguita con successo\n");
	
	if(intbuffer[0]==0){ //succede se sono il primo utente
		i=0;
		do{
			printf("inserisci il numero di giocatori (min 2, max 4)..\n\n");
			scanf("%s", &bs);
			cont=(int)bs-48;
			if(cont<2 || cont>4){
				printf("valore non ammesso!");
				fc=-1;
			}
			else fc=1;
		}while(fc==-1);
		memcpy(&buffer, &bs, 1);
		write(des, buffer, 1);
		/*ho un valore corretto di giocatori*/
		if(cont==2) printf("BENVENUTO! Sei l'utente 0; attendi la connessione di un altro utente..\n\n");
		else printf("BENVENUTO! Sei l'utente 0; attendi la connessione di altri %d utenti..\n\n",cont-1);
		numG=cont;
		cont=0;
		
	}
	else{ //se non sei il primo utente
		i=intbuffer[0];
		printf("BENVENUTO! Sei l'utente %d; attendi per favore..\n\n",i);
		read(des, intbuffer, 1);
		controllo();
		numG=intbuffer[0];
		printf("num utenti: %d\n",numG);
		/*connesione completata, torno al main*/
	}
	for(x=0; x<numG; x++){
		gioco[x]=1;
	}
	x=0;
}
/* fine funzione connessione */


/* LETTURA */
void lettura(){
	ret = read(des, buffer, 1);
	controllo();
	if(buffer[0]=='c') printf("%d è stato COLPITO!!!\n\n", intbuffer[0]);
	if(buffer[0]=='a') printf("ACQUA! Il giocatore %d non è stato colpito..\n\n", intbuffer[0]);
	if(buffer[0]=='f') printf("COLPITO E AFFONDATO! il giocatore %d ha perso una nave..\n\n", intbuffer[0]);
	if(buffer[0]=='e'){
			printf("ELIMINATO! Il giocatore %d ha perso tutte le navi..\n\n", intbuffer[0]);
			gioco[intbuffer[0]]=0;
			ret = read(des, intbuffer, 1);
			controllo();
			if(intbuffer[0]!=1) printf("siamo rimasti in %d\n\n", intbuffer[0]);
			if(intbuffer[0]==1){
				puts("COMPLIMENTI HAI VINTO!");
				exit(0);
			}
	}
}
/* fine funzione lettura */

/* ATTACCO */
void attacco(){
	printf("EI TOCCA TE!\n");
	do{
		printf("inserisci il numero dell'utente da attaccare\n");
		scanf("%s", &bs);
		cont=(int)bs-48;
		if(cont==i || cont<0 || cont>numG-1 || gioco[cont]==0) puts("utente non valido! Riprova..");
	}while(cont==i || cont<0 || cont>numG-1 || gioco[cont]==0);
	/*invio chi è il nemico*/
	intbuffer[0]=cont;
	printf("avversario: %d\n",cont);
	write(des,intbuffer,1);
	
	do{			
		puts("inserisci le cordinate (es: a4)");
		scanf("%s", buffer);
		j=converti(buffer[0]);
		k=(int)buffer[1]-48;
		if(j<0 || j>9 || k<0 || k>9) puts("mossa non valida! Riprovare");
	}while(j<0 || j>9 || k<0 || k>9);
	/*invio cordinata j*/
	intbuffer2[0]=k;
	write(des, intbuffer2, 1);
	
	/*invio cordinata k*/
	intbuffer2[0]=j;
	write(des, intbuffer2, 1);
	
	ret = read(des, buffer, 1);
	controllo();
	if(buffer[0]!='y') printf("hai immesso una sequenza di caratteri non valida.");
	else puts("mossa confermata dal server, attendo risposta..");
	lettura();
}
/* fine funzione attacco */

/* DIFESA*/
void difesa(){
	puts("SEI SOTTO ATTACCO!");
	attaccante = intbuffer2[0];
	ret = read(des, intbuffer, 1);
	controllo();
	difJ = intbuffer[0]; //coordinata x attacco
	read(des, intbuffer, 1);
	controllo();
	difK = intbuffer[0]; //coordinata y attacco
	cfe=0;		
	if(mat[difJ][difK]!=0){
		cod=mat[difJ][difK]; //codice nave colpita
		mat[difJ][difK]=0; //tolgo il pezzo di nave colpito
		for(j=0; j<10; j++){
			for(k=0; k<10; k++){
				if(mat[j][k]==cod) cfe=1; //solo colpito c'è ancora il codice
			}
		}
		if(cfe==1){ //solo colpito
			buffer[0]='c';
			printf("MALEDIZIONE! Sei stato colpito..\n\n");
			write(des, buffer, 1);
		}
		else{ //colpito e affondato. Eliminato?
			cfe=1;
			for(j=0; j<10; j++){
				for(k=0; k<10; k++){
					if(mat[j][k]!=0) cfe=0; //solo colpito e affondato
				}
			}
			if(cfe==0){
				buffer[0]='f';
				printf("MALEDIZIONE NO! Colpito e affondato..\n\n");
				write(des, buffer, 1);
			}
			else{ //eliminato
				buffer[0]='e';
				printf("HAI PERSO! Mi dispiace non hai più navi..\n\n");
				printf("chiudo la comunicazione con il server..\n");
				write(des, buffer, 1);
				sleep(1); //do il tempo al server di annunciare la mossa
				close(des);
				printf("partita terminata\n\n");
				exit(1);
			}
		}
	}
	else{
		buffer[0]='a';
		printf("CHE FORTUNA! Non sei stato colpito..\n\n");
		write(des, buffer, 1);
	}
}				
/* fine funzione difesa */


/* PARTITA */
void partita(){
	while(1){
		ret = read(des, intbuffer, 1); //leggo il turno e vedo la mappa
		controllo();
		stampa();
		printf("turno = %d, (se te lo fossi dimenticato, tu sei %d)\n",intbuffer[0],i);
		if(intbuffer[0]==i) attacco(); //se è il mio turno, attacco
		else { //altrimenti
			printf("non tocca a te, è il turno di %d\n",intbuffer[0]);
			intbuffer2[0]=intbuffer[0];
			ret = read(des, intbuffer, 1); //leggo verso di chi è l'attacco
			controllo();			
			if((intbuffer[0]==i)) difesa(); //se sono attaccato mi difendo
			else{
				if(intbuffer[0]!=intbuffer2[0]) printf("attacco verso %d\n", intbuffer[0]);
				lettura(); //altrimenti leggo il risultato dell'attacco
			}
		}
	}
}
/* fine funzione partita */

/* MAIN */
int main(){
	printf("\n                   BATTAGLIA NAVALE (on-line!)\n\n");
	ambiente();

	signal(SIGTERM, gestoreTerminazione);
	signal(SIGINT, gestoreTerminazione);
	signal(SIGQUIT, gestoreTerminazione);
	signal(SIGHUP, gestoreTerminazione);
	signal(SIGPIPE, gestoreBrokPipe);

	signal(SIGILL, gestoreErrori);
	signal(SIGSEGV, gestoreErrori);if(ret==-1){
					printf("ERRORE: non puoi sovrapporre due navi!\n");
					stampa();
				}
	signal(SIGXCPU, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	
	connessione();
	
	/*pronto per posizionare le navi*/
	ret=read(des, buffer, 1);
	controllo();
	if(buffer[0]!='p') gestoreErrori(); //controllo errore;
	else{
		posizionaNavi();
		buffer[0]='y';
		write(des, buffer, 1);
	}
	/*navi posizionate*/
	
	/*inizia la partita*/
	partita();
	exit(0);
}	
/* fine main */
/*************/
