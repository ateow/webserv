#!/usr/bin/env python3
import cgitb
cgitb.enable()

print("Content-Type: text/html")    # HTML is following
print()                             # blank line, end of headers

print("<html>")
print("<head>")
print("<title>Test CGI</title>")
print("</head>")
print("<body>")
print("<h1>CGI Script with Infinite Loop</h1>")

# Infinite loop
i = 0
while True:
    i += 1
    # Normally, you might have some logic here, but it's just an increment to illustrate.

print("</body>")
print("</html>")