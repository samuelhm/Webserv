#!/usr/bin/env python3

import cgi
import os
import html

# Analiza los parámetros del query string
params = cgi.FieldStorage()
nombre = params.getfirst("nombre", "Mundo")
nombre = html.escape(nombre)  # Sanitiza para evitar inyección de HTML

print("Content-Type: text/html")
print(f"""
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>Hola</title>
</head>
<body>
    <h1>Hola {nombre}</h1>
</body>
</html>
""")
