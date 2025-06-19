<?php

//commande pour tester chunked: curl -v --http1.1 -H "Transfer-Encoding: chunked" --data-binary @/home/jose-lfe/Downloads/N.png http://localhost:8002/upload/upload.php


// pour test chunked
if ($_SERVER['REQUEST_METHOD'] == 'POST')
{
    $uploads_dir = getenv("UPLOAD_DIR");
    if (!$uploads_dir) {
        $uploads_dir = __DIR__ . '/uploads';
    }
    if (!is_dir($uploads_dir)) {
        mkdir($uploads_dir, 0755, true);
    }
    $data = file_get_contents("php://input");
    // Log la taille reçue dans un fichier temporaire
    file_put_contents("/tmp/php_upload_size.log", "Taille reçue: " . strlen($data) . " octets\n", FILE_APPEND);

    $filename = $uploads_dir . '/N.png';
    if (file_put_contents($filename, $data) !== false) {
        echo "Upload chunked reçu !";
    } else {
        echo "Erreur lors de l'upload.";
    }
}
else if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_FILES['photo']))
{
    // Récupération du nom de dossier cible (variable)
    $subdir = isset($_POST['dir']) ? basename($_POST['dir']) : 'uploads';
    $uploads_dir = getenv("UPLOAD_DIR");
    if (!$uploads_dir)
    {
        $uploads_dir = __DIR__ . '/uploads'; // fallback si la variable n'est pas définie
    }

    // Création du dossier s'il n'existe pas
    if (!is_dir($uploads_dir)) {
        mkdir($uploads_dir, 0755, true);
    }

    // Données du fichier
    $tmp_name = $_FILES['photo']['tmp_name'];
    $name = basename($_FILES['photo']['name']);

    // Déplacement
    if (move_uploaded_file($tmp_name, "$uploads_dir/$name")) {
        echo "Upload réussi dans $subdir !";
    } else {
        echo "Erreur lors de l'upload.";
    }
}
else
{
    echo "Pas de fichier uploadé.";
}
?>