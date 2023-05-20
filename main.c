#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 22
#define MAX_CLIENTS 10

// Struct om informatie van een client bij te houden
typedef struct {
    int socket;
    struct sockaddr_in address;
    // Voeg hier eventueel andere relevante gegevens toe
} ClientInfo;

// Functie om clientgegevens te verwerken
void* handleClient(void* arg) {
    ClientInfo* client = (ClientInfo*)arg;
    // Implementeer hier de logica voor het verwerken van de gegevens van de client

    // Sluit de client socket
    close(client->socket);

    // Geef de resources vrij
    free(client);
    pthread_exit(NULL);
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddress;
    pthread_t threads[MAX_CLIENTS];
    int numClients = 0;

    // Maak het logbestand
    FILE* logFile = fopen("log.txt", "w");
    if (logFile == NULL) {
        perror("Kan logbestand niet maken");
        exit(1);
    }

    // Creëer de server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Kan server socket niet maken");
        exit(1);
    }

    // Configureer het server adres
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Bind de server socket aan het opgegeven adres en poort
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Kan server socket niet binden");
        exit(1);
    }

    // Luister naar inkomende verbindingen
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Kan niet luisteren naar inkomende verbindingen");
        exit(1);
    }

    printf("TCP-server is gestart en luistert op poort %d...\n", PORT);

    // Accepteer inkomende verbindingen en behandel ze
    while (1) {
        // Accepteer een nieuwe verbinding
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            perror("Kan de verbinding niet accepteren");
            exit(1);
        }

        // Verwerk alleen nieuwe verbindingen als er nog geen MAX_CLIENTS zijn bereikt
        if (numClients < MAX_CLIENTS) {
            // Maak een nieuwe clientinfo struct en vul deze met