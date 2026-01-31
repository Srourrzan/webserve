#!/usr/bin/env python3
import os, sys;#, cgi

print("Content-Type: text/plain")
print()  # end headers
print("Hello from CGI!")
print("REQUEST_METHOD =", os.environ.get("REQUEST_METHOD"))
print("QUERY_STRING  =", os.environ.get("QUERY_STRING"))
# read POST body if present
if os.environ.get("REQUEST_METHOD") == "POST":
    length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
    body = sys.stdin.read(length) if length > 0 else ""
    print("POST BODY:", body)