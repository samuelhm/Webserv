#!/usr/bin/env python3

import sys
import os
import html
import cgi

# 🔥 Asegurar salida en modo correcto
sys.stdout.write("Status: 200 OK\r\n")
sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
sys.stdout.write("\r\n")  # 🔥 Fin de headers

# 🔥 Ahora empieza el cuerpo de la respuesta
sys.stdout.write("""<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>CGI Test</title>
</head>
<body>
""")

# 🔥 Procesar parámetros del QueryString
params = cgi.FieldStorage()

nombre = params.getfirst("nombre", "Mundo")
nombre = html.escape(nombre)

sys.stdout.write(f"<h1>Hola {nombre}</h1>\n")

# 🔥 Información adicional (opcional)
sys.stdout.write("<p>Este CGI funciona correctamente 🚀</p>\n")
sys.stdout.write("</body>\n</html>\n")
