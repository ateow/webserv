#!/usr/bin/env python3
import sys
import os
from urllib.parse import parse_qs

def get_input():
    """Retrieves and parses the input based on the method type."""
    method = os.environ["REQUEST_METHOD"]
    if method == "POST":
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        input_data = sys.stdin.read(content_length)
    elif method == "GET":
        input_data = os.environ.get("QUERY_STRING", "")
    else:
        return {}
    
    return parse_qs(input_data)

def main():
    print("Content-Type: text/html")
    print()  # blank line required, end of headers

    inputs = get_input()
    name = inputs.get('name', [''])[0]
    
    if name:
        greeting = f"Hello, {name}!"
    else:
        greeting = "Hello, World! What's your name?"

    print(f"<html><body><h1>{greeting}</h1></body></html>")

if __name__ == "__main__":
    main()
