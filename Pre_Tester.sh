#!/bin/bash

# Contenido HTML embellecido
html_content='<!DOCTYPE html>
<html lang="es">
<head>
	<meta charset="UTF-8">
	<title>Hola Mundo</title>
	<style>
		body {
			background: linear-gradient(135deg, #667eea, #764ba2);
			color: white;
			font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
			display: flex;
			justify-content: center;
			align-items: center;
			height: 100vh;
			margin: 0;
		}
		.container {
			text-align: center;
			background-color: rgba(0, 0, 0, 0.3);
			padding: 2rem 3rem;
			border-radius: 1rem;
			box-shadow: 0 4px 20px rgba(0,0,0,0.4);
		}
		h1 {
			font-size: 3rem;
			margin-bottom: 1rem;
		}
		p {
			font-size: 1.2rem;
		}
	</style>
</head>
<body>
	<div class="container">
		<h1>¡Hola, Mundo!</h1>
		<p>Bienvenido a tu servidor web con estilo 😎</p>
	</div>
</body>
</html>
'

# Función para crear archivo HTML bonito
create_html_file() {
	echo "$html_content" > "$1"
}

# Crear directorios
mkdir -p YoupiBanane/nop
mkdir -p YoupiBanane/Yeah

# Crear archivos con HTML embellecido
create_html_file YoupiBanane/youpi.bad_extension
echo "" > YoupiBanane/youpi.bla  # Este se usará como CGI (vacío o lo que necesites)
create_html_file YoupiBanane/nop/youpi.bad_extension
create_html_file YoupiBanane/nop/other.pouic
create_html_file YoupiBanane/Yeah/not_happy.bad_extension

echo "Estructura y archivos generados con contenido bonito 😄"
