# Simplified-HTTP-Retriever-and-Server-CSS432

You will write two programs that exercise a simplified version of HTTP.  The retriever will work in conjunction with any web server and the server will work in conjunction with any web browser.  This way, you can test your software independently of each other.

1)  Your retriever takes in an input from the command line and parses the server address and file that is being requested.  The program then issues a GET request to the server for the requested file.  When the file is returned by the server, the retriever outputs the file to the screen and to the file system.  If the server returns an error code instead of a OK code, then the retriever should not save the file and should display on the screen whatever error page was sent with the error.  Your retriever should process the webpage source code for <script> tags and <img> tags and issue GET requests for those files and save them to the file system.  Your retriever should exit after receiving the response.

2)  Your server waits for a connection and an HTTP GET request (you may do this single threaded or multi-threaded).  After receiving the GET request, the server opens the file that is requested and sends it (along with the HTTP 200 OK code, of course).  If the file requested does not exist, the server should return a 404 Not Found code along with a custom File Not Found page.

You will submit the code for both assignments with a build and demo script.  The demo script should run through all of the following test cases.  You will also submit screen shots of your programs executing all of the following test cases. 

