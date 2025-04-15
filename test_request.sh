#!/bin/zsh

# Lista de URIs para testear
uris=(
	"/index.html"                   			 # recurso existente en location
	"/location/noexiste.html"                    # recurso no existente
	"/%ZZ"                                       # bad request (carácter inválido)
	"/cgi-bin/script1.cgi"                       # CGI sin path_info ni query
	"/cgi-bin/script1.cgi/info"                  # CGI con path_info
	"/cgi-bin/script1.cgi?param=1"               # CGI con query_string
	"/cgi-bin/script1.cgi/info?param=1&x=2"      # CGI con path_info y query_string
	"/cgi-bin/noexiste.cgi"                      # CGI que no existe
)

host="http://localhost:8080"

echo "Probador de URIs HTTP - pulsa cualquier tecla para enviar cada una"

for uri in "${uris[@]}"
do
	echo "\n>>> Probando: $uri"
	read -k "tecla?Pulsa una tecla para enviar: "
	echo ""
	curl -i "$host$uri"
	echo "\n=============================================="
done
