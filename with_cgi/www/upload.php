<?php
/*
echo "Méthode de requête : " . $_SERVER['REQUEST_METHOD'] . "\n";
echo "--- _FILES ---\n";
var_dump($_FILES);
echo "--- _POST ---\n";
var_dump($_POST);
echo "--- RAW INPUT ---\n";
file_put_contents("/tmp/raw_input.txt", file_get_contents("php://input"));
*/
/*
// Ancien script d'upload
if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_FILES['photo'])) {
    $uploads_dir = __DIR__ . '/photos';
    if (!is_dir($uploads_dir)) {
        mkdir($uploads_dir, 0755, true);
    }

    $tmp_name = $_FILES['photo']['tmp_name'];
    $name = basename($_FILES['photo']['name']);

    if (move_uploaded_file($tmp_name, "$uploads_dir/$name")) {
        echo "Upload réussi !";
    } else {
        echo "Erreur lors de l'upload.";
    }
} else {
    echo "Pas de fichier uploadé.";
}
*/

// Nouveau script pour debug
echo "<pre>";

file_put_contents("/tmp/php_input.txt", file_get_contents("php://input"));

echo "Méthode de requête : " . $_SERVER['REQUEST_METHOD'] . "\n";
echo "Fichiers uploadés :$_FILES\n";
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    echo "\n--- _FILES ---\n";
    print_r($_FILES);

    if (isset($_FILES['photo'])) {
        $uploads_dir = __DIR__ . '/photos';
        echo "\nDossier de destination : $uploads_dir\n";

        if (!is_dir($uploads_dir)) {
            if (mkdir($uploads_dir, 0755, true)) {
                echo "Dossier créé avec succès.\n";
            } else {
                echo "Échec de la création du dossier.\n";
            }
        }

        $tmp_name = $_FILES['photo']['tmp_name'];
        $name = basename($_FILES['photo']['name']);

        echo "Nom temporaire : $tmp_name\n";
        echo "Nom original : $name\n";

        if (move_uploaded_file($tmp_name, "$uploads_dir/$name")) {
            echo "Upload réussi !\n";
        } else {
            echo "Erreur lors de l'upload.\n";
        }
    } else {
        echo "Clé 'photos' absente dans \$_FILES.\n";
    }
} else {
    echo "Pas une requête POST.\n";
}

echo "\n--- _SERVER ---\n";
print_r($_SERVER);

echo "\n--- En-têtes de la requête ---\n";
foreach (getallheaders() as $name => $value) {
    echo "$name: $value\n";
}

echo "</pre>";

?>