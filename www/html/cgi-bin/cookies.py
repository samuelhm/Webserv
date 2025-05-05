#!/usr/bin/env python3
import os
import http.cookies

print("Content-Type: text/html")
print()  # Empty line to separate headers from body

# Obtener las cookies del entorno
cookie_string = os.environ.get("HTTP_COOKIE", "")
cookies = http.cookies.SimpleCookie()
cookies.load(cookie_string)

# Imprimir cookies en HTML
print("<html><body>")
print("<h1>Cookies recibidas:</h1>")
if cookies:
    print("<ul>")
    for key, morsel in cookies.items():
        print(f"<li>{key} = {morsel.value}</li>")
    print("</ul>")
else:
    print("<p>No se recibieron cookies.</p>")
print("</body></html>")
