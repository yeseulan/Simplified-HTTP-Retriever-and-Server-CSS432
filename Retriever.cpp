/* Yeseul An
 * CSS432
 * Assignment2
 * The assignment that exercise a simplified version of HTTP.
 * The retriever program issues a GET request to the server for the requested file and outputs the file
 *  to the screen when it returns back from the server.
 */

#include <iostream>
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netdb.h>        // gethostbyname
#include <stdlib.h>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <string.h>
#include <string>
#include <algorithm>
#include <cstring>
#include <vector>
using namespace std;

const char* fileName;
const char* serverName;
const static char* port;

// The example file that is used in write on when the content is returned from the server
const string FILENAME = "output.txt";

void printAndWrite(int &socket, int &bufSize, string sName, string fName, const char *p, string of, bool flg);
string processLine(string l);
string trim(string str);
string processTag(string m);
int createSocket(const char * argv[]);
int createNewRequest(int socket, string sN, string fN);

string parseCode(int sd)
{
    string msg = "";
    char prev = 0;
    while ( true )
    {
        char cur = 0;
        recv(sd , &cur , 1 , 0);
        if ( cur == '\n' || cur == '\r' )
        {
            if ( prev == '\r' && cur == '\n' ) // For each header, it is ended with a \r\n
                break;
        }
        else {
            msg += cur;
        }
        prev = cur;
    }
    return msg;
}

int createRequest(int socket) {
    string request = string("GET " + string(fileName) + " HTTP/1.1\r\n" + "Host: " + string(serverName) + "\r\n\r\n");

    cout << "Request is below : " << endl;
    cout << request;

    int sendResult = send(socket, request.c_str(), strlen(request.c_str()), 0);
    if (sendResult <= 0)
    {
        cout << "Unable to send the request." << endl;
        return -1;
    }

    bool outToFile = false;
    string returnedMessage = "";
    int bufsize = 0;
    while (true)
    {
        string returnedMessage = parseCode(socket);
        if (returnedMessage == "")
        {
            break;
        }
        cout << returnedMessage << endl;
        if (returnedMessage.substr(9, 15) == "200 OK")
        {
            outToFile = true;
        }
        if (returnedMessage.substr(0, 15) == "Content-Length:"){
            bufsize = atoi(returnedMessage.substr(16, returnedMessage.length()).c_str());
        }
    }

    // Write on to the first output.txt file the whole html
    printAndWrite(socket, bufsize, serverName, fileName, port, FILENAME, outToFile);

    close(socket);
    cout << "Finished the client program." << endl;
    cout << endl;
    return 0;
}

/*
 * printAndWrite()
 * Parameter socket is the client socket that receives data, bufSize is buffer size, fileName
 * is the name of the file requested, flg is the flag that indicates whether the file should be written
 * into the file.
 * Prints to the console the file that requests, and output to the file if requested file is
 * available. If file is not availalbe, it prints out the customized error page to the console
 * and don't write any output files.
 */

void printAndWrite(int &socket, int &bufSize, string sName, string fName, const char *p, string of, bool flg)
{
    char buffer[bufSize];
    string outToConsole;
    ofstream myfile;
    if (flg)
    {
        myfile.open(of);
        cout << "File Name: " << of << endl;
        recv(socket, &buffer, bufSize, 0);
        for (int i = 0; i < bufSize; i++)
        {
            char c = buffer[i];
            outToConsole += c;
            myfile << c;
        }
        cout << "Finished write out. " << endl;
    } else {
        recv(socket, &buffer, bufSize, 0);
        for (int i = 0; i < bufSize; i++)
        {
            char c = buffer[i];
            outToConsole += c;
        }
    }

    myfile.close();
    cout << endl;
    cout << "Content of the file below: " << endl;
    cout << outToConsole << endl;

    // Process each line in the output file
    string line = "";
    if (flg) {
        istringstream stream(outToConsole);
        while (getline(stream, line)) {
            line = processLine(line);
            if (line != "") // This means that it is img or script tag
            {
                string tmpServer;
                string tmpFile;
                if (line.find("/") != string::npos)
                {
                    size_t cut = line.find_first_of("/");
                    tmpServer = (cut == string::npos) ? "" : line.substr(0, cut);
                    tmpFile = (cut == string::npos) ? "" : line.substr(cut+1);
                } else { // if it is local host
                    tmpFile = line;
                    tmpServer = string(sName);
                }
                cout << tmpServer << endl;
                const char *param[4];
                tmpFile = "/" + tmpFile;

                param[1] = tmpServer.c_str();
                param[2] = tmpFile.c_str();
                param[3] = port;

                int socketMade = createSocket(param);
                cout << "Socket is created " << endl;
                createNewRequest(socketMade, tmpServer, tmpFile);
            }
        }
    }
}

int createNewRequest(int socket, string sN, string fN)
{

    string newRequest = string("GET " + fN + " HTTP/1.1\r\n" + "Host: " + sN + "\r\n\r\n");
    cout << "New Request is below: " << endl;
    cout << newRequest;

    int sendResult = send(socket, newRequest.c_str(), strlen(newRequest.c_str()), 0);
    if (sendResult <= 0)
    {
        cout << "Unable to send the request." << endl;
        return -1;
    }

    bool outToFile = false;
    string returnedMessage = "";
    int size = 0;
    while (true)
    {
        string returnedMessage = parseCode(socket);
        if (returnedMessage == "")
        {
            break;
        }
        cout << returnedMessage << endl;
        if (returnedMessage.substr(9, 15) == "200 OK")
        {
            outToFile = true;
        }
        if (returnedMessage.substr(0, 15) == "Content-Length:") {
            size = atoi(returnedMessage.substr(16, returnedMessage.length()).c_str());
            cout << size << endl;
        }
    }

    string outputFileName;
    if (fN.find(".js") != string::npos)
    {
        outputFileName = "script_object.js";
    } else {
        outputFileName = "img_object.jpg";
    }

    char buffer[size];
    string outToConsole;
    ofstream myfile;
    myfile.open(outputFileName);
    cout << "File Name: " << outputFileName << endl;
    recv(socket, &buffer, size, 0);
    for (int i = 0; i < size; i++)
    {
        char c = buffer[i];
        outToConsole += c;
        myfile << c;
    }
    cout << "Finished write out new request. " << endl;

    myfile.close();
    close(socket);
    cout << "Finished new request of the client program." << endl;
    cout << endl;
    return 0;
}

string processLine(string l)
{
    l = trim(l);
    string scriptTag = "<script";
    string imgTag= "<img";

    string check1 =  l.substr(0,7);
    string check2 = l.substr(0,4);

    string returnMsg = "";

    if (check1 == scriptTag)
    {
        returnMsg = processTag(l);
    }

    if (check2 == imgTag)
    {
        returnMsg = processTag(l);
    }

    return returnMsg;
}

string trim(string str)
{
    size_t start = str.find_first_not_of(' ');
    str = (start == string::npos) ? "" : str.substr(start);

    size_t end = str.find_last_not_of(' ');
    str = (end == string::npos) ? "" : str.substr(0, end + 1);
    return str;
}

string processTag(string m)
{
    stringstream ss(m);
    string item = "";
    bool img = false;
    bool script = false;
    vector<string> tokens;
    while (getline(ss, item, ' '))
    {
        tokens.push_back(item);
    }

    if (tokens[0].substr(0, 4) == "<img") {
        for (int i = 1; i < tokens.size(); i++) {
            if (tokens[i].substr(0,3) == "src") {
                m = tokens[i];
                img = true;
                break;
            }
            else {
                continue;
            }
        }
    }

    if (tokens[0].substr(0, 7) == "<script") {
        for (int i = 1; i < tokens.size(); i++) {
            if (tokens[i].substr(0,3) == "src") {
                m = tokens[i];
                script = true;
                break;
            }
            else {
                continue;
            }
        }
    }

    size_t start1 = m.find_first_of("\"");
    m = (start1 == string::npos) ? "" : m.substr(start1+1);
    if (m.substr(0, 4) == "http")
    {
        m = m.substr(8);
        if (m.substr(0,3) != "www")
        {
            m = "www." + m;
        }
    } else if (m.substr(0, 5) == "https")
    {
        m = m.substr(9);
        if (m.substr(0,3) != "www")
        {
            m = "www." + m;
        }
    }
    size_t end1 = m.find_last_of("\"");
    m = (end1 == string::npos) ? "" : m.substr(0, end1);
    return m;
}

/*
 * createSocket
 * Parameter argcv indicates the arguments from the command line including server address, file name,
 *  and port number.
 * Uses getaddrinfo to get the IP address of the server name and create the client socket and connectt.
 * Return -1 if it can't successfully create socket and connect to the socket and return the socket
 *  if it successfully creates it.
 */
int createSocket(const char *argv[]) {

    // use getaddrinfo to convert hostname into the ip address
    // this both supports IPv4 and IPv6
    struct addrinfo hints, *infoptr;
    memset(&hints, 0, sizeof(hints));

    // Sets its properties
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    serverName= argv[1];
    fileName=argv[2];
    port = argv[3];

    int result = getaddrinfo(serverName, argv[3], &hints, &infoptr);

    if (result != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
        return -1;
    }

    struct addrinfo *connection;
    int clientSd;
    int connectionResult;

    // Loop through connections until it finds the connection
    for (connection = infoptr; connection != NULL; connection = connection->ai_next)
    {
        clientSd = socket(connection->ai_family, connection->ai_socktype, connection->ai_protocol);
        if (clientSd == -1)
        {
            cerr << "Invalid socket file descriptor. " << endl;
            continue;
        }

        connectionResult = connect(clientSd, connection->ai_addr, connection->ai_addrlen);

        if (connectionResult == -1)
        {
            cerr << "Invalid socket connection" << endl;
            continue;
        }

        // Connected successfully
        cout << "Found a connection" << endl;
        break;

    }

    // If connection is null, it means it couldn't find the connection and return -1
    if (connection == NULL)
    {
        cerr << "Unable to find connection." << endl;
        return -1;
    }

    // free the infoptr pointer
    freeaddrinfo(infoptr);
    return clientSd;
}

/*
 * Entry point of the retriever program.
 * Parameter argc indicates the number of arguments which are server address, port number, and file name
 *  to retrieve.
 * Parameter argv indicates values of arguments from the command line.
 * Return -1 if it can't successfully create socket, calls createRequest and return 0 if successful.
 */
int main(int argc, const char *argv[]) {

    if (argc != 4) {
        cerr << "You need 3 arguments:" << endl;
        cerr << endl;
        cerr << "1. Server Address: server address being requested" << endl;
        cerr << "2. File Name: file being requested " << endl;
        cerr << "3. Port Number: server port being requested " << endl;

        return -1;
    }

    int createdSocket = createSocket(argv);

    if (createdSocket < 0) {
        cout << "The socket can't be created." << endl;
        return -1;
    }

    return createRequest(createdSocket);
    return 0;
}
