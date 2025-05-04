<?php
echo "<h1>Datos recibidos:</h1>";
echo "<pre>";
echo "Método: " . $_SERVER['REQUEST_METHOD'] . "\n\n";

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    echo "Cuerpo:\n";
    echo file_get_contents('php://input');
} else {
    echo "GET params:\n";
    print_r($_GET);
}
echo "</pre>";
?>
<?php
phpinfo();
?>
