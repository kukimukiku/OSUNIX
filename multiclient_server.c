#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define BUFFLEN 1024
#define MAXCLIENTS 2



int findemptyuser(int c_sockets[]){
    int i;
    for (i = 0; i <  MAXCLIENTS; i++){
        if (c_sockets[i] == -1){
            return i;
        }
    }
    return -1;
}

int main(int argc, char *argv[]){

    const char *words_arr[] = {"UNIX", "LINUX", "WINDOWS"};
    char random[50];
    srand(time(NULL));
    int nr = rand()%3; // 0 - 9 //gaaunamas random zodzio indeksas is array
	strcpy(random, words_arr[nr]);
    int l=strlen(random); //length of word
    char hidden_word[l];
    for(int i = 0; i < l; i++){ //padaromas "pasleptas" zodis 
        hidden_word[i]='-';
    }
    int guesses=6; //leistinu suklydimu spejant skaicius
    int right_answ=0; //ar ats teisingas
    int letters_count=0; //atspetu raidziu skaicius

    unsigned int port;
    unsigned int clientaddrlen;
    int l_socket; //prisijungimo laukimo socketas 
    int c_sockets[MAXCLIENTS]; //prisijungusiu klientu socketas
    fd_set read_set;

    struct sockaddr_in servaddr; //serverio adreso struktura 
    struct sockaddr_in clientaddr; //prisijungusio kliento adreso struktura

    int maxfd = 0;
    int i;

    char buffer[BUFFLEN];

    if (argc != 2){
        fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
        return -1;
    }


    port = atoi(argv[1]);
    if ((port < 1) || (port > 65535)){
        fprintf(stderr, "ERROR #1: invalid port specified.\n");
        return -1;
    }

    if ((l_socket = socket(AF_INET, SOCK_STREAM,0)) < 0){
        fprintf(stderr, "ERROR #2: cannot create listening socket.\n");
        return -1;
    }

    printf(">  %d %d\n", 7, htons(256));

    //isvaloma ir uzpildoma serverio struktura
    memset(&servaddr,0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(0); 
    servaddr.sin_port = htons(port); 

    if (bind (l_socket, (struct sockaddr *)&servaddr,sizeof(servaddr))<0){ //bind the socket to localhost port
        fprintf(stderr,"ERROR #3: bind listening socket.\n");
        return -1;
    }

    if (listen(l_socket, 5) <0){ 
        fprintf(stderr,"ERROR #4: error in listen().\n");
        return -1;
    }                           

    //visus klientu socketus padaro empty
    for (i = 0; i < MAXCLIENTS; i++){
        c_sockets[i] = -1;
    }


    while(1){
        FD_ZERO(&read_set); //clear the socket set
        for (i = 0; i < MAXCLIENTS; i++){
            if (c_sockets[i] != -1){
                FD_SET(c_sockets[i], &read_set);
                if (c_sockets[i] > maxfd){
                    maxfd = c_sockets[i];
                }
            }
        }

        FD_SET(l_socket, &read_set);
        if (l_socket > maxfd){
            maxfd = l_socket;
        }
        
        select(maxfd+1, &read_set, NULL , NULL, NULL);

        if (FD_ISSET(l_socket, &read_set)){
            int client_id = findemptyuser(c_sockets);
            if (client_id != -1){
                clientaddrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, clientaddrlen);
                c_sockets[client_id] = accept(l_socket, 
                    (struct sockaddr*)&clientaddr, &clientaddrlen);
                printf("Connected:  %s\n",inet_ntoa(clientaddr.sin_addr));
            }
        }
        for (i = 0; i < MAXCLIENTS; i++){
            if (c_sockets[i] != -1){
                if (FD_ISSET(c_sockets[i], &read_set)){
                    memset(&buffer, BUFFLEN, 1);
                    int r_len = recv(c_sockets[i], &buffer, BUFFLEN, 0); //gaunama raide is kliento

                    

                    for(int i = 0; i < l; i++){
                        if(buffer[0] == random[i] && buffer[1] == '\n'){ //ziurima ar raide yra kazkur zodyje
                            hidden_word[i] = random[i];
                            right_answ = 1; //jei yra - atsakymas teisingas
                            letters_count++; //atspetu raidziu skaicius ++
                        }
                    }
                    if(r_len <= 0) {
                        close(c_sockets[i]);
                    }
                    if(right_answ == 0){
		                guesses--; //jei neatspeta, galimu spejimu skaicius --
		            }
                    right_answ = 0; 

                    for (int j = 0; j < MAXCLIENTS; j++){ //kad abu klientai zaistu kartu o ne su serveriu po viena
                        if (c_sockets[j] != -1){
                            
		                    if(letters_count == l){ //jei atspetu raidziu skaicius lygus zodzio ilgiui - zaidimas laimetas
			                    int w_len = send(c_sockets[j], "You and your friend won", 23, 0);
                                close(c_sockets[j]);
                                c_sockets[j] = -1;
                                if(w_len <= 0){
                                    close(c_sockets[j]);
                                    c_sockets[j] = -1;
                                }
		                    }
		                    else if(guesses == 0){ //jei spejimai pasibaige - zaidimas pralaimetas
			                    int w_len = send(c_sockets[j], "You and your friend lost. Word was ", 35, 0);
                                int w1_len = send(c_sockets[j], random, l, 0);
                                close(c_sockets[j]);
                                c_sockets[j] = -1;
                                if(w_len <= 0 || w1_len <= 0){
                                    close(c_sockets[j]);
                                    c_sockets[j] = -1;
                                }
		                    }
		                    else{ //kitu atveju abiems zaidejams nusiunciamas "pasleptas" zodis ir speta raide
                                int w_len = send(c_sockets[j], hidden_word, l, 0);
                                int w1_len = send(c_sockets[j], " ", 1, 0);
                                int w2_len = send(c_sockets[j], buffer, 2, 0);
                                if (w_len <= 0 || w1_len <= 0 || w2_len <= 0){
                                    close(c_sockets[j]);
                                    c_sockets[j] = -1;
                                }
                            }
                            

                        }
                    }
                }
            }
        }
    }

    return 0;
}

