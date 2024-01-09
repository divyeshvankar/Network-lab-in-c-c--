#include <iostream>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// Types of DNS resource records
#define T_A 1 // IPv4 address
#define T_NS 2 // Nameserver
#define T_CNAME 5 // Canonical name
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 // Mail server

std::unordered_map<std::string, std::vector<std::string>> dnsCache;

// Function Prototypes
void ngethostbyname(const char*, int);
void ChangetoDnsNameFormat(unsigned char*, const char*);
unsigned char* ReadName(unsigned char*, unsigned char*, int*);

// DNS header structure
struct DNS_HEADER
{
    unsigned short id; // identification number

    unsigned char rd : 1; // recursion desired
    unsigned char tc : 1; // truncated message
    unsigned char aa : 1; // authoritative answer
    unsigned char opcode : 4; // purpose of message
    unsigned char qr : 1; // query/response flag

    unsigned char rcode : 4; // response code
    unsigned char cd : 1; // checking disabled
    unsigned char ad : 1; // authenticated data
    unsigned char z : 1; // reserved
    unsigned char ra : 1; // recursion available

    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count;  // number of additional entries
};

// Constant sized fields of query structure
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};

// Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl; // Time To Live
    unsigned short data_len;
};
#pragma pack(pop)

// Pointers to resource record contents
struct RES_RECORD
{
    unsigned char* name;
    struct R_DATA* resource;
    unsigned char* rdata;
};

// Structure of a Query
typedef struct
{
    unsigned char* name;
    struct QUESTION* ques;
} QUERY;

int main()
{
    while (true)
    {
        char hostname[100];

        // Get the hostname from the terminal
        std::cout << "Enter Hostname to Lookup (or 'exit'): ";
        std::cin >> hostname;
        if (strcmp(hostname, "exit") == 0)
        {
            break; // Exit the loop if the user enters 'exit'
        }
        std::string domainName(hostname);
        if (dnsCache.find(domainName) != dnsCache.end())
        {
            std::cout << "Using cached response for " << domainName << "\n";
            const std::vector<std::string>& cachedIPs = dnsCache[domainName];
            int i = 1;
            for (const std::string& ip : cachedIPs)
            {
                std::cout << "Resolved IP: " << i++ << ")" << ip << "\n";
            }
        }
        else
        {
            // Now get the IP of this hostname (A record)
            ngethostbyname(hostname, T_A);
        }
    }

    return 0;
}

/*
 * Perform a DNS query by sending a packet
 * */
void ngethostbyname(const char* host, int query_type)
{
    unsigned char buf[65536], *qname, *reader;
    int i, j, stop, s;

    struct sockaddr_in a;
    struct RES_RECORD answers[20]; // the replies from the DNS server
    struct sockaddr_in dest;
    struct DNS_HEADER* dns = nullptr;
    struct QUESTION* qinfo = nullptr;

    std::cout << "Resolving " << host;

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Create a UDP socket for DNS queries

    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr("172.17.1.1"); // DNS servers

    // Set up the DNS header
    dns = (struct DNS_HEADER*)&buf;

    dns->id = (unsigned short)htons(getpid());
    dns->qr = 0;         // This is a query
    dns->opcode = 0;     // This is a standard query
    dns->aa = 0;         // Not authoritative
    dns->tc = 0;         // Message not truncated
    dns->rd = 1;         // Recursion Desired
    dns->ra = 0;         // Recursion not available
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1); // We have only 1 question
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;

    // Point to the query portion
    qname = (unsigned char*)&buf[sizeof(struct DNS_HEADER)];

    ChangetoDnsNameFormat(qname, host);

    qinfo = (struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)]; // Fill in the question

    qinfo->qtype = htons(query_type); // Type of the query, e.g., A, MX, CNAME, NS, etc.
    qinfo->qclass = htons(1);         // It's the internet

    std::cout << "\nSending Packet...";
    if (sendto(s, (char*)buf, sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1) + sizeof(struct QUESTION), 0, (struct sockaddr*)&dest, sizeof(dest)) < 0)
    {
        perror("sendto failed");
    }
    std::cout << "Done";

    // Receive the answer
    socklen_t i_len = sizeof dest;
    std::cout << "\nReceiving answer...";
    if (recvfrom(s, (char*)buf, 65536, 0, (struct sockaddr*)&dest, &i_len) < 0)
    {
        perror("recvfrom failed");
    }
    std::cout << "Done";

    dns = (struct DNS_HEADER*)buf;

    // Move ahead of the DNS header and the query field
    reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1) + sizeof(struct QUESTION)];

    std::cout << "\nThe response contains : ";
    std::cout << "\n " << ntohs(dns->q_count) << " Questions.";
    std::cout << "\n " << ntohs(dns->ans_count) << " Answers.\n\n";

    // Start reading answers
    stop = 0;
    std::vector<std::string> resolvedIPs; // Store the resolved IP addresses

    for (i = 0; i < ntohs(dns->ans_count); i++)
    {
        answers[i].name = ReadName(reader, buf, &stop);
        reader = reader + stop;

        answers[i].resource = (struct R_DATA*)(reader);
        reader = reader + sizeof(struct R_DATA);

        if (ntohs(answers[i].resource->type) == 1) // If it's an IPv4 address
        {
            answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len));

            for (j = 0; j < ntohs(answers[i].resource->data_len); j++)
            {
                answers[i].rdata[j] = reader[j];
            }

            reader = reader + ntohs(answers[i].resource->data_len);
        }
        else
        {
            answers[i].rdata = ReadName(reader, buf, &stop);
            reader = reader + stop;
        }

        // Calculate TTL in minutes and seconds
        int ttl_seconds = ntohl(answers[i].resource->ttl);
        int ttl_minutes = ttl_seconds / 60; // Calculate minutes
        int ttl_remaining_seconds = ttl_seconds % 60; // Calculate remaining seconds

        // Print answers
        std::cout << "---------------------------------\n";
        std::cout << "Type : " << ntohs(answers[i].resource->type) << "\n";
        std::cout << "TTL : " << ttl_minutes << " minutes " << ttl_remaining_seconds << " seconds\n";
        std::cout << "Name : " << answers[i].name << "\n";

        if (ntohs(answers[i].resource->type) == T_A) // IPv4 address
        {
            long* p;
            p = (long*)answers[i].rdata;
            a.sin_addr.s_addr = (*p);
            std::cout << "Data : " << inet_ntoa(a.sin_addr) << "\n";
            resolvedIPs.push_back(inet_ntoa(a.sin_addr));
        }

        if (ntohs(answers[i].resource->type) == T_CNAME)
        {
            // Canonical name for an alias
            std::cout << "Data : " << answers[i].rdata << "\n";
        }
    }
    dnsCache[host] = resolvedIPs; // Store the resolved IPs in the cache

    // Cleanup
    close(s);
}

/*
 * Read a DNS name from a response packet
 * */
unsigned char* ReadName(unsigned char* reader, unsigned char* buffer, int* count)
{
    unsigned char* name;
    unsigned int p = 0, jumped = 0, offset;
    int i, j;

    *count = 1;
    name = (unsigned char*)malloc(256);

    name[0] = '\0';

    // Read the names in 3www6google3com format
    while (*reader != 0)
    {
        if (*reader >= 192)
        {
            offset = (*reader) * 256 + *(reader + 1) - 49152; // Handle compression
            reader = buffer + offset - 1;
            jumped = 1; // We have jumped to another location so counting won't go up
        }
        else
        {
            name[p++] = *reader;
        }

        reader = reader + 1;

        if (jumped == 0)
        {
            *count = *count + 1; // If we haven't jumped to another location, we can count up
        }
    }

    name[p] = '\0'; // String complete
    if (jumped == 1)
    {
        *count = *count + 1; // Number of steps we actually moved forward in the packet
    }

    // Convert 3www6google3com0 to www.google.com
    for (i = 0; i < (int)strlen((const char*)name); i++)
    {
        p = name[i];
        for (j = 0; j < (int)p; j++)
        {
            name[i] = name[i + 1];
            i = i + 1;
        }
        name[i] = '.';
    }
    name[i - 1] = '\0'; // Remove the last dot
    return name;
}

/*
 * Convert a hostname to DNS name format (e.g., www.google.com to 3www6google3com)
 * */
void ChangetoDnsNameFormat(unsigned char* dns, const char* host)
{
    int lock = 0, i;
    strcat((char*)host, ".");

    for (i = 0; i < strlen((char*)host); i++)
    {
        if (host[i] == '.')
        {
            *dns++ = i - lock;
            for (; lock < i; lock++)
            {
                *dns++ = host[lock];
            }
            lock++; // Move to the next segment
        }
    }
    *dns++ = '\0'; // Null-terminate the DNS name
}


