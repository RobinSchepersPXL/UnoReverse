#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <curl/curl.h>

#define PORT 22
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024
#define LOG_FILE_PATH "log.txt"
#define GEOLOCATION_API_URL "http://ip-api.com/json/"

// Functie om HTTP GET-verzoek te doen
size_t write_callback(char *data, size_t size, size_t nmemb, void *userdata) {
    // Deze functie kan worden aangepast om de gegevens te verwerken zoals gewenst
    FILE *log_file = (FILE *)userdata;
    fprintf(log_file, "Received HTTP response: %s\n", data);
    return size * nmemb;
}

int main() {
    int server_fd, new_socket, client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in address, client_address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    int data_counter = 0; // Teller voor succesvol afgeleverde gegevens

    // Open logbestand voor schrijven
    FILE *log_file = fopen(LOG_FILE_PATH, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    // Maak een socketbestand descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Stel opties van de socket in
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Koppel de socket aan het opgegeven IP-adres en poort
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Luister op de socket naar inkomende verbindingen
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("UnoReverse server is listening on port %d\n", PORT);

    while (1) {
        // Accepteer inkomende verbindingen
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Haal het IP-adres van de client op
        char *client_ip = inet_ntoa(client_address.sin_addr);
        printf("New connection from IP address: %s\n", client_ip);

        // Log het IP-adres in het logbestand
        fprintf(log_file, "New connection from IP address: %s\n", client_ip);

        // Start een HTTP-clientverbinding en doe een GET-verzoek naar de IP Geolocation API
        CURL *curl;
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        if (curl) {
            // Stel de URL in op basis van het IP-adres van de client
            char geolocation_url[256];
            snprintf(geolocation_url, sizeof(geolocation_url), "%s%s", GEOLOCATION_API_URL, client_ip);

            // Stel de schrijffunctie voor de HTTP-client in
            curl_easy_setopt(curl, CURLOPT_URL, geolocation_url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, log_file);
            // Voer het GET-verzoek uit
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }

            // Sluit de HTTP-clientverbinding af
            curl_easy_cleanup(curl);
        }

        // Sluit de socketverbinding met de client
        close(new_socket);

        // Voeg de gegevens toe aan het logbestand
        fprintf(log_file, "Data received from client: %s\n", buffer);

        // Verhoog de teller voor succesvol afgeleverde gegevens
        data_counter++;

        // Log het aantal succesvol afgeleverde gegevens bij het sluiten van de verbinding
        fprintf(log_file, "Number of successfully delivered data: %d\n", data_counter);

        // Sluit het logbestand
        fclose(log_file);
    }

    return 0;
}