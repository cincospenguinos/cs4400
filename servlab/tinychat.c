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

void doit(int fd);
dictionary_t *read_requesthdrs(rio_t *rp);
void read_postquery(rio_t *rp, dictionary_t *headers, dictionary_t *d);
void parse_query(const char *uri, dictionary_t *d);
void serve_reply(int fd, const char *username, const char *chatroom);
void serve_login(int fd, const char *pre_content);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
static void print_stringdictionary(dictionary_t *d);

static void add_comment(const char *chatroom, const char *username, const char *value);

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

      /* This was for testing
      if (!strcasecmp(method, "GET") && starts_with("/test", uri)){
        dictionary_set(comments, "yo", dictionary_get(query, "content"));
	clienterror(fd, method, "200", "It Worked", "It worked!");
      }
      */
      
      /* The user POSTed something - should only be to /reply */
      if (!strcasecmp(method, "POST") && !strcasecmp(uri, "/reply")){
	printf(">>> POSTed something to /reply\n");
        read_postquery(&rio, headers, query);
	char *username = dictionary_get(query, "username");
	char *chatroom = dictionary_get(query, "chatroom");
	char *content = dictionary_get(query, "content");

	if(content != NULL && chatroom != NULL){
	  printf(">>> Set something in the dictionary\n");
	  dictionary_set(comments, chatroom, content);
	}

	serve_reply(fd, username, chatroom);
      }

      /* The user requested the conversation */
      if (starts_with("/conversation", uri) && !strcasecmp(method, "GET")){
	// TODO: This - for the bots
	printf(">>> Request to \"%s\"; probably a bot\n", uri);
      }

      /* Show the login page */
      if (!strcasecmp(method, "GET") && !strcasecmp(uri, "/")){
	printf(">>> User requested main page\n");
	serve_login(fd, query);
      }

      /* For debugging, print the dictionary */
      print_stringdictionary(query);

      // If we got this far, then it's a request we shouldn't worry about
      clienterror(fd, method, "403", "Forbidden", "Not permitted");

      /* Clean up */
      free_dictionary(query);
      free_dictionary(headers);
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

void serve_login(int fd, const char *pre_content){
  // TODO: This
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

void serve_reply(int fd, const char *username, const char *chatroom){

  size_t len;
  char *body, *header;
  char *herp = (char*)dictionary_get(comments, chatroom);

  if(herp != NULL)
    body = append_strings("<html><body>\r\n",
			  "<p>Welcome to TinyChat --- ", chatroom, "</p>\r\n",
			  "<p>", herp, "</p>\r\n",
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
