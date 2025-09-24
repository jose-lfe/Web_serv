<?php
header("Content-Type: text/plain");

// Récupération des paramètres de l'URL
if (isset($_GET['name'])) {
    echo "Hello " . htmlspecialchars($_GET['name']);
} else {
    echo "Hello world"; // pas de parametre
}
?>
