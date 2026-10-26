#!/bin/sh
printf 'Content-Type: text/html\r\n\r\n'
printf '<!doctype html><html><head><meta charset="utf-8"><title>CGI</title><style>body{font-family:sans-serif;background:#0b1730;color:white;padding:50px}a{color:#7ab8ff}</style></head><body><h1>CGI is working ✓</h1><p>Executed by a separate process.</p><p>Method: %s</p><p>Query: %s</p><a href="/">Back to home</a></body></html>\n' "$REQUEST_METHOD" "$QUERY_STRING"
