/*
 * tinychat.c - [Starting code for] a web-based chat server.
 *
 * Based on:
 *  tiny.c - A simple, iterative HTTP/1.0 Web server that uses the 
 *      GET method to serve static and dynamic content.
 *   Tiny Web server
 *   Dave O'Hallaron
 *   Carnegie Mellon University
 */
#include "csapp.h"
#include "dictionary.h"
#include "more_string.h"

// Some defs to help us organize our code
#define REQ_ERROR 0
#define REQ_WEB 1
#define REQ_CONVERSATION 2
#define REQ_SAY 3
#define REQ_IMPORT 4
#define REQ_LOGIN 5

void doit(int fd);
dictionary_t *read_requesthdrs(rio_t *rp);
void read_postquery(rio_t *rp, dictionary_t *headers, dictionary_t *d);
void parse_query(const char *uri, dictionary_t *d);
void serve_reply(int fd, const char *username, const char *chatroom);
void serve_login(int fd, const char *pre_content);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
static void print_stringdictionary(dictionary_t *d);

/* Some helper methods */
static int what_request(const char *uri, const char *req, dictionary_t *query);
static void add_comment(char *chatroom, char *username, char *comment);
static void serve_conversation(int fd, const char *conversation);

/**
 * PLAN
 *
 * --> Front end stuff
 * --> /conversation?topic=‹topic›
 * --> /say?user=‹user›&topic=‹topic›&content=‹content›
 * --> /import?topic=‹topic›&host=‹host›&port=‹port›
 */

// What we'll use to hold all the comments
static dictionary_t *comments;

int main(int argc, char **argv) 
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);

  /* Don't kill the server if there's an error, because
     we want to survive errors due to a client. But we
     do want to report errors. */
  exit_on_error(0);

  /* Also, don't stop on broken connections: */
  Signal(SIGPIPE, SIG_IGN);

  /* Create the comments dictionary */
  comments = make_dictionary(COMPARE_CASE_SENS, free);

  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    if (connfd >= 0) {
      Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                  port, MAXLINE, 0);
      printf("Accepted connection from (%s, %s)\n", hostname, port);
      doit(connfd);
      Close(connfd);
    }
  }

  free_dictionary(comments);
}

/*
 * doit - handle one HTTP request/response transaction
 */
void doit(int fd) 
{
  char buf[MAXLINE], *method, *uri, *version;
  rio_t rio;
  dictionary_t *headers, *query;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;
  printf("%s", buf);
  
  if (!parse_request_line(buf, &method, &uri, &version)) {
    printf("URI was %s", uri);
    clienterror(fd, method, "400", "Bad Request",
                "TinyChat did not recognize the request");
  } else {
    if (strcasecmp(version, "HTTP/1.0")
        && strcasecmp(version, "HTTP/1.1")) {
      clienterror(fd, version, "501", "Not Implemented",
                  "TinyChat does not implement that version");
    } else if (strcasecmp(method, "GET")
               && strcasecmp(method, "POST")) {
      clienterror(fd, method, "501", "Not Implemented",
                  "TinyChat does not implement that method");
    } else {
      headers = read_requesthdrs(&rio);

      /* Parse all query arguments into a dictionary */
      query = make_dictionary(COMPARE_CASE_SENS, free);
      parse_uriquery(uri, query);

      if(!strcmp(method, "POST"))
	read_postquery(&rio, headers, query);

      int req_type = what_request(uri, method, query);

      // Some vars declared to make things easier on ourselves
      char *chatroom, *content, *username;

      switch(req_type){
      case REQ_LOGIN:
	printf(">>> LOGIN REQUEST\n");
	serve_login(fd, query);
	break;
      case REQ_WEB:
	printf(">>> WEB REQUEST\n");
	chatroom = dictionary_get(query, "chatroom");
	content = dictionary_get(query, "content");
	username = dictionary_get(query, "username");

	if(content != NULL && strcmp(content, ""))
	  add_comment(chatroom, username, content);

	serve_reply(fd, dictionary_get(query, "username"), chatroom);
	break;
      case REQ_CONVERSATION:
	printf(">>> CONVERSATION REQUEST\n");
	// /conversation?topic=<topic>
	chatroom = dictionary_get(query, "topic");
	char *stuff = dictionary_get(comments, chatroom);
        serve_conversation(fd, stuff);
	break;
      case REQ_SAY:
	printf(">>> SAY REQUEST\n");
	chatroom = dictionary_get(query, "topic");
	username = dictionary_get(query, "user");
	content = dictionary_get(query, "content");

	if(content == NULL)
	  content = "";

	add_comment(chatroom, username, content);
	//printf(">>> Hello\n");
	serve_reply(fd, username, chatroom);
	break;
      case REQ_IMPORT:
	printf(">>> IMPORT REQUEST\n");
	// TODO: This
	break;
      case REQ_ERROR:
      default:
	printf(">>> ERROR REQUEST");
	clienterror(fd, method, "510", "<strong>Some Crazy Error</strong>", "The request you sent is not acceptable. You may have provided too few params, or tried to get something you're not supposed to.");
      }

      /* For debugging, print the dictionary */
      print_stringdictionary(query);

      /* Clean up */
      free_dictionary(query);
      free_dictionary(headers);

      printf(">>> CURRENT STATE OF COMMENTS:\n");
      print_stringdictionary(comments);
      printf("\n");
    }

    /* Clean up status line */
    free(method);
    free(uri);
    free(version);
  }
}

/*
 * read_requesthdrs - read HTTP request headers
 */
dictionary_t *read_requesthdrs(rio_t *rp) 
{
  char buf[MAXLINE];
  dictionary_t *d = make_dictionary(COMPARE_CASE_INSENS, free);

  Rio_readlineb(rp, buf, MAXLINE);
  printf("%s", buf);
  while(strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    parse_header_line(buf, d);
  }
  
  return d;
}

void read_postquery(rio_t *rp, dictionary_t *headers, dictionary_t *dest)
{
  char *len_str, *type, *buffer;
  int len;
  
  len_str = dictionary_get(headers, "Content-Length");
  len = (len_str ? atoi(len_str) : 0);

  type = dictionary_get(headers, "Content-Type");
  
  buffer = malloc(len+1);
  Rio_readnb(rp, buffer, len);
  buffer[len] = 0;

  if (!strcasecmp(type, "application/x-www-form-urlencoded")) {
    parse_query(buffer, dest);
  }

  free(buffer);
}

static char *ok_header(size_t len, const char *content_type) {
  char *len_str, *header;
  
  header = append_strings("HTTP/1.0 200 OK\r\n",
                          "Server: TinyChat Web Server\r\n",
                          "Connection: close\r\n",
                          "Content-length: ", len_str = to_string(len), "\r\n",
                          "Content-type: ", content_type, "\r\n\r\n",
                          NULL);
  free(len_str);

  return header;
}

static void serve_conversation(int fd, const char *conversation){
  char *body = append_strings(conversation, "\r\n", NULL);
  size_t len = strlen(body);
  char *header = ok_header(len, "text/plain; charset=utf-8");
  Rio_writen(fd, header, strlen(header));
  printf("Response headers:\n");
  printf("%s", header);
  free(header);

  Rio_writen(fd, body, len);
  printf("Body:\n");
  printf("%s", body);
  free(body);
}

void serve_login(int fd, const char *pre_content){ // TODO: Fix the params here
  size_t len;
  char *body, *header;
  
  body = append_strings("<html><body>\r\n",
                        "<p>Welcome to TinyChat</p>\r\n",
                        "<form action=\"reply\" method=\"post\"",
                        " enctype=\"application/x-www-form-urlencoded\"",
                        " accept-charset=\"UTF-8\">\r\n",
                        "<input type=\"text\" name=\"username\" placeholder=\"username\">\r\n",
			"<input type=\"text\" name=\"chatroom\" placeholder=\"chatroom\">\r\n",
                        "<input type=\"submit\" value=\"Login\">\r\n",
                        "</form></body></html>\r\n",
                        NULL);
  
  len = strlen(body);

  /* Send response headers to client */
  header = ok_header(len, "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));
  printf("Response headers:\n");
  printf("%s", header);

  free(header);

  /* Send response body to client */
  Rio_writen(fd, body, len);

  free(body);
}

static int what_request(const char *uri, const char *req, dictionary_t *query){
  printf(">>> REQ: %s \t URI: %s\n", req, uri);

  if (!strcasecmp(req, "GET")){
    if (!strcasecmp(uri, "/"))
      return REQ_LOGIN;
    else if (starts_with("/conversation", uri)) // TODO: Ensure params
      return REQ_CONVERSATION;
    else if (starts_with("/say", uri) && dictionary_get(query, "user") && dictionary_get(query, "topic"))
      return REQ_SAY;
    else if (starts_with("/import", uri)) // TODO: Ensure params
      return REQ_IMPORT;
  } else if (!strcasecmp(req, "POST")){
    if(!strcasecmp(uri, "/reply") && 
       dictionary_get(query, "username") != NULL && 
       dictionary_get(query, "chatroom") != NULL)
      return REQ_WEB;
    else
      return REQ_ERROR;
  }

  return REQ_ERROR;
}

static void add_comment(char *chatroom, char *username, char *comment){
  char *old = dictionary_get(comments, chatroom);

  if(old == NULL){
    //printf(">>> Nothing for %s...", chatroom);
    char *new = append_strings(username, " : ", comment, "\n", NULL);
    dictionary_set(comments, chatroom, new);
  } else {
    //printf(">>> Appending to old comment...");
    char *new = append_strings(old, username, " : ", comment, "\n", NULL);
    dictionary_set(comments, chatroom, new);
  }
}

void serve_reply(int fd, const char *username, const char *chatroom){

  size_t len;
  char *body, *header;
  char *herp = (char*)dictionary_get(comments, chatroom);

  if(herp != NULL)
    body = append_strings("<html><body>\r\n",
			  "<p>Welcome to TinyChat --- ", chatroom, "</p>\r\n",
			  "<pre>", herp, "</pre>\r\n"
			  "<form action=\"reply\" method=\"post\"",
			  " enctype=\"application/x-www-form-urlencoded\"",
			  " accept-charset=\"UTF-8\">\r\n",
			  "<input type=\"text\" name=\"content\">\r\n",
			  "<input type=\"submit\" value=\"Send\">\r\n",
			  "<input type=\"hidden\" name=\"username\" value=\"", username, "\">\r\n",
			  "<input type=\"hidden\" name=\"chatroom\" value=\"", chatroom, "\">\r\n",
			  "</form></body></html>\r\n",
			  NULL);
  else
    body = append_strings("<html><body>\r\n",
			  "<p>Welcome to TinyChat --- ", chatroom, "</p>\r\n",
		        "<form action=\"reply\" method=\"post\"",
                        " enctype=\"application/x-www-form-urlencoded\"",
                        " accept-charset=\"UTF-8\">\r\n",
                        "<input type=\"text\" name=\"content\">\r\n",
			"<input type=\"submit\" value=\"Send\">\r\n",
			"<input type=\"hidden\" name=\"username\" value=\"", username, "\">\r\n",
			"<input type=\"hidden\" name=\"chatroom\" value=\"", chatroom, "\">\r\n",
                        "</form></body></html>\r\n",
                        NULL);

  len = strlen(body);

  /* Send response headers to client */
  header = ok_header(len, "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));
  printf("Response headers:\n");
  printf("%s", header);

  free(header);

  /* Send response body to client */
  Rio_writen(fd, body, len);

  free(body);
}

/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
  size_t len;
  char *header, *body, *len_str;

  body = append_strings("<html><title>Tiny Error</title>",
                        "<body bgcolor=""ffffff"">\r\n",
                        errnum, " ", shortmsg,
                        "<p>", longmsg, ": ", cause,
                        "<hr><em>The Tiny Web server</em>\r\n",
                        NULL);
  len = strlen(body);

  /* Print the HTTP response */
  header = append_strings("HTTP/1.0 ", errnum, " ", shortmsg,
                          "Content-type: text/html; charset=utf-8\r\n",
                          "Content-length: ", len_str = to_string(len), "\r\n\r\n",
                          NULL);
  free(len_str);
  
  Rio_writen(fd, header, strlen(header));
  Rio_writen(fd, body, len);

  free(header);
  free(body);
}

static void print_stringdictionary(dictionary_t *d)
{
  int i, count;

  count = dictionary_count(d);
  for (i = 0; i < count; i++) {
    printf("%s=%s\n",
           dictionary_key(d, i),
           (const char *)dictionary_value(d, i));
  }
  printf("\n");
}
