/* Yeseul An
 * CSS432
 * Assignment2
 * The assignment that exercise a simplified version of HTTP.
 * The server program gets a GET request from the retriever. The server opens the file
 *  that is requested and sends it to the retriever with the status code.
 */

#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>     // gethostbyname
#include <unistd.h>    // read, write, close
#include <strings.h>     // bzero
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>      // writev
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // handle the connection
#include <iostream>
#include <thread>
#include <fstream>
#include <sstream>
#include <string.h>

using namespace std;

// n number of connection request
const int n = 1;

// struct that has information of new socket
struct thread_data
{
    int sd;
};

/*
 * processData()
 * Process data of the requested file.
 * Parameter includes fileName is the file that is requested, statusCode, and fileContent.
 * Modifies values to the statusCode and fileContent so that they can be sent back
 * to the retriever.
 */
void processData(string &fileName, string &statusCode, string &fileContent)
{
    fileContent ="";

    cout << "Looking for this file " + fileName << endl;
    FILE *file = fopen(fileName.c_str(), "r");

    string fName = "notfound.html";
    FILE *errorPage = fopen(fName.c_str(), "r");

    if (file == nullptr)
    {
        statusCode = "HTTP/1.1 404 Not Found\r\n";
        while (!feof(errorPage))
        {
            char c = fgetc(errorPage);
            if (c < 0)
            {
                continue;
            }
            if (c == '\n')
            {
                fileContent += '\n';
                continue;
            }
            else if ( c == '\r')
            {
                fileContent += "\r";
                continue;
            }
            fileContent += c;
        }
        fclose(errorPage);
    } else {
        statusCode = "HTTP/1.1 200 OK\r\n";
        while (!feof(file))
        {
            string line;
            char c = fgetc(file);
            if (c < 0)
            {
                continue;
            }
            if (c == '\n')
            {
                fileContent += '\n';
                continue;
            }
            else if ( c == '\r')
            {
                fileContent += "\r";
                continue;
            }
            fileContent += c;
        }
        fclose(file);
    }
}

/*
 * processHeader()
 * Process each line of the header.
 * Parameter socket is the socket that has request information.
 * Returns processed header.
 */
string processHeader(int socket)
{
    string returnHeader= "";
    char prev = 0;
    while (true)
    {
        char cur = 0;
        recv(socket, &cur, 1, 0);
        if (cur == '\n' || cur =='\r')
        {
            if (prev == '\r' && cur == '\n')
            {
                break;
            }
        }
        else {
            returnHeader += cur;
        }
        prev = cur;
    }
    return returnHeader;
}

/*
 * process_request()
 * Processes GET request from the client that is either from browser or retriever program.
 * Also, returns status code and data content to the retriever.
 * If fileName that is requested exists, returns 200 OK code with the file, otherwise
 * returns 400 code with the error page.
 */
void *process_request(void *data)
{
    cout << "I received get request and process it." << endl;
    struct thread_data *my_data;
    my_data = (struct thread_data *)data;

    string header = "";
    string fileName ="";

    while (true)
    {
        header = processHeader(my_data->sd);
        if (header == "")
        {
            break;
        }
        if (header.substr(0,3) == "GET")
        {
            istringstream input(header);
            string code;
            input >> code >> fileName;
            fileName = fileName.substr(1, fileName.length());
            break;
        }
    }

    cout << "This is code: " << header << endl;
    cout << "This is filename: " << fileName << endl;

    string statusCode;
    string fileContent;
    processData(fileName, statusCode, fileContent);

    string pageLength = to_string(fileContent.size());
    string response = statusCode + "Content-Length: " + pageLength + "\r\n"
                      + "Content-Type: text/html\r\n\r\n" + fileContent;

    send(my_data->sd, &response[0], response.size(), 0);

    cout << "Everything goes well successfully!" << endl;
    close(my_data->sd);
    pthread_exit(NULL);
}

/*
 *  createSocket()
 *  Creates a server socket that can interpret and find the IP address from the
 *  request of the client.
 *  Returns the server socket that can be used to listen to the request.
 *  Returns -1 if socket is not created successfully.
 */
int createSocket() {

    // Set the port as 2681
    const char *service = "2681";

    // use getaddrinfo to convert hostname into the ip address
    // this both supports IPv4 and IPv6
    struct addrinfo hints;
    struct addrinfo *infoptr;
    memset(&hints, 0, sizeof(hints));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int result = getaddrinfo(nullptr, service, &hints, &infoptr);
    if (result != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
        return -1;
    }

    struct addrinfo *connection;
    int serverSd;

    for (connection=infoptr; connection != NULL; connection=connection->ai_next)
    {
        serverSd = socket(connection->ai_family, connection->ai_socktype, connection->ai_protocol);
        if (serverSd == -1)
        {
            cerr << "Invalid socket file descriptor. " << endl;
            continue;
        }

        const int on = 1;
        setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
        bind(serverSd, connection->ai_addr, connection->ai_addrlen);

        break;
    }

    if (connection == NULL)
    {
        cerr << "Unable to find connection." << endl;
        return -1;
    }

    cout << "Successfully create a socket." << endl;
    freeaddrinfo(infoptr);
    return serverSd;
}

/*
 *  Main function that is entry point of the server.
 *  Parameter arcs indicates number of arguments which is 1 for the program itself.
 *  No other argument is necessary.
 *  The server continuously waits for future clients' connection, the server
 *   doesn't stop running if there is no errors.
 */
int main(int argc, const char *argv[]) {

    if (argc != 1) {
        cerr << "There should be no argument." << endl;
        return -1;
    }

    int serverSd = createSocket();
    int result = listen(serverSd, n);
    if (result != 0)
    {
        cerr << "Unable to listen to the server." << endl;
        return -1;
    }

    int count = 1;
    while (true)
    {
        // get read to accept a new connection from a client
        struct sockaddr_storage newSockAddr;
        socklen_t newSockAddrSize = sizeof(newSockAddr);
        int newSd = accept(serverSd, (struct sockaddr *)&newSockAddr, &newSockAddrSize);
        cout << "accepted" << endl;

        if (newSd == -1)
        {
            printf("Connection not successful\n");
            continue;
        }

        printf("Connection Successful\n");

        // create a new thread
        pthread_t new_thread;
        struct thread_data data;
        data.sd = newSd;

        cout << "Creating new thread: " << to_string(count) << endl;
        int itr = pthread_create(&new_thread, nullptr, process_request, (void*) &data);
        if (itr != 0)
        {
            cout << "Unable to create a thread." << endl;
            continue;
        }
        count++;
    }
    close(serverSd);
    return 0;
}
