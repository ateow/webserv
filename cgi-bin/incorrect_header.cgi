#!/usr/bin/env python3
import cgitb
cgitb.enable()

# Incorrect header syntax (missing 'Content-Type' and newline)
print("This is not a valid HTTP header")

print("<html><head><title>Bad Header</title></head><body>")
print("<h1>The HTTP header above is incorrect!</h1>")
print("</body></html>")
