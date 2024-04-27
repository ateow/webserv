# webserv
42 Webserv group project working repository

Error Handling:

METHODS:
* if not upper case: return 400
* if uppercase but method not recongise: return 405


┌──(kali㉿kali)-[~/Desktop/webserv]\
└─$ curl -I -X GET http://127.0.0.1/ \
HTTP/1.1 200 OK\
Server: nginx/1.24.0\

┌──(kali㉿kali)-[~/Desktop/webserv]\
└─$ curl -I -X Get http://127.0.0.1/ \
HTTP/1.1 400 Bad Request\
Server: nginx/1.24.0\

┌──(kali㉿kali)-[~/Desktop/webserv]\
└─$ curl -I -X XXX http://127.0.0.1/ \
HTTP/1.1 405 Not Allowed\
Server: nginx/1.24.0\

┌──(kali㉿kali)-[~/Desktop/webserv]\
└─$ curl -I -X GET1 http://127.0.0.1/ \
HTTP/1.1 400 Bad Request\
Server: nginx/1.24.0\

