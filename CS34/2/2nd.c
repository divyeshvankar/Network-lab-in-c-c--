#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

// Define the maximum cache size and maximum response size
#define CACHE_SIZE 5
#define MAX_RESPONSE_SIZE 8192 // Adjust this as needed

// Define a structure to represent a web page
struct WebPage {
    char url[1024];
    char* content;
    struct WebPage* next;
    struct WebPage* prev; // Add a previous pointer for LRU
};

// Define a structure to represent the cache
struct Cache {
    struct WebPage* head;
    struct WebPage* tail; // Add a tail pointer for LRU
    int size;
};

// Function to insert a web page at the front of the cache
void insert_at_front(struct Cache* cache, const char* url, const char* content) {
    // Allocate memory for the new web page
    struct WebPage* new_page = (struct WebPage*)malloc(sizeof(struct WebPage));
    // Copy the URL into the new page's structure
    strncpy(new_page->url, url, sizeof(new_page->url) - 1);
    // Dynamically allocate memory for content and copy it
    new_page->content = strdup(content);
    // Initialize pointers
    new_page->next = cache->head;
    new_page->prev = NULL;

    // Update previous pointer for the current head
    if (cache->head != NULL) {
        cache->head->prev = new_page;
    }

    cache->head = new_page;

    // If the cache was empty, update the tail pointer
    if (cache->size == 0) {
        cache->tail = new_page;
    }

    cache->size++;

    // Evict the least recently used page if the cache is full
    if (cache->size > CACHE_SIZE) {
        struct WebPage* last_page = cache->tail;
        // Update the tail pointer to the previous page
        cache->tail = last_page->prev;
        cache->tail->next = NULL;

        // Free the dynamically allocated content
        free(last_page->content);
        // Free the page structure itself
        free(last_page);

        cache->size--;
    }
}

// Function to move a web page to the front of the cache (LRU)
void move_to_front(struct Cache* cache, struct WebPage* page) {
    // If the page is already at the front, no need to do anything
    if (page == cache->head) {
        return;
    }

    // Update pointers for the adjacent pages
    if (page->prev != NULL) {
        page->prev->next = page->next;
    }
    if (page->next != NULL) {
        page->next->prev = page->prev;
    }

    // Update the tail pointer if needed
    if (page == cache->tail) {
        cache->tail = page->prev;
    }

    // Move the page to the front
    page->next = cache->head;
    page->prev = NULL;
    cache->head->prev = page;
    cache->head = page;
}

// Function to display the contents of the cache with a serial number
void display_cache(struct Cache* cache, int iteration) {
    struct WebPage* current = cache->head;
    int serial = 1;
    printf("Cache Contents for Iteration %d:\n", iteration);
    while (current != NULL) {
        printf("[%d] URL: %s\n", serial, current->url);
        current = current->next;
        serial++;
    }
}

// Function to check if a URL is in the cache
struct WebPage* find_in_cache(struct Cache* cache, const char* url) {
    struct WebPage* current = cache->head;
    while (current != NULL) {
        if (strcmp(current->url, url) == 0) {
            return current; // Found in cache
        }
        current = current->next;
    }
    return NULL; // Not found in cache
}

int main() {
    // Initialize the cache
    struct Cache cache = {NULL, NULL, 0};
    int client_socket;
    struct sockaddr_in server_address;
    struct addrinfo hints, *server;
    char input_url[1024];
    int iteration = 1;
    char choice;

    while (1) {
        // Prompt the user to enter the URL of the webpage
        printf("Enter the URL (or 'exit' to quit): ");
        fgets(input_url, sizeof(input_url), stdin);
        input_url[strcspn(input_url, "\n")] = '\0';  // Remove newline

        if (strcmp(input_url, "exit") == 0) {
            break;  // Exit the program if 'exit' is entered
        }

        // Remove "http://" or "https://" prefixes if present
        if (strncmp(input_url, "http://", 7) == 0) {
            memmove(input_url, input_url + 7, strlen(input_url) - 6);
        } else if (strncmp(input_url, "https://", 8) == 0) {
            memmove(input_url, input_url + 8, strlen(input_url) - 7);
        }

        // Check if the page is already in the cache
        struct WebPage* cached_page = find_in_cache(&cache, input_url);
        if (cached_page != NULL) {
            // Page is in cache, move it to the front (LRU)
            move_to_front(&cache, cached_page);
            printf("Retrieved content for Iteration %d (from cache):\n%s\n", iteration, cached_page->content);
            display_cache(&cache, iteration);
            iteration++;
            continue; // Skip the rest of the loop
        }

        // Initialize the socket
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Extract the hostname from the URL
        char hostname[1024];
        if (sscanf(input_url, "%1023[^/]/", hostname) != 1) {
            perror("Invalid URL format");
            exit(EXIT_FAILURE);
        }

        // Set up server_address using getaddrinfo
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(hostname, "80", &hints, &server) != 0) {
            perror("Error in resolving hostname");
            exit(EXIT_FAILURE);
        }

        memcpy(&server_address, server->ai_addr, sizeof(struct sockaddr_in));

        freeaddrinfo(server);  // Free the server info structure

        // Connect to the server
        if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
            perror("Connection to the server failed");
            exit(EXIT_FAILURE);
        }

        // HTTP GET request
        char request[2048];
        snprintf(request, sizeof(request), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", hostname);

        // Send the HTTP request
        if (send(client_socket, request, strlen(request), 0) == -1) {
            perror("Error sending request");
            exit(EXIT_FAILURE);
        }

        // Receive and store the response
        char response[MAX_RESPONSE_SIZE];
        int bytes_received = 0;
        int total_bytes = 0;
        while (1) {
            int bytes = recv(client_socket, response + total_bytes, sizeof(response) - total_bytes, 0);
            if (bytes <= 0) {
                break;
            }
            total_bytes += bytes;
            if (total_bytes >= sizeof(response)) {
                break;  // Avoid buffer overflow
            }
        }

        response[total_bytes] = '\0';

        // Insert the retrieved page into the cache
        insert_at_front(&cache, input_url, response);

        // Display the retrieved content
        printf("Retrieved content for Iteration %d:\n%s\n", iteration, response);

        // Display the cache contents for the current iteration
        display_cache(&cache, iteration);

        iteration++;  // Increment the iteration counter

        // Clean up and close the socket
        close(client_socket);
    }

    return 0;
}