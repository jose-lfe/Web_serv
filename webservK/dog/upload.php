<?php

if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_FILES['photo'])) {
    $uploads_dir = __DIR__ . '/photos';
    if (!is_dir($uploads_dir)) {
        mkdir($uploads_dir, 0755, true);
    }

    $tmp_name = $_FILES['photo']['tmp_name'];
    $name = basename($_FILES['photo']['name']);

    if (move_uploaded_file($tmp_name, "$uploads_dir/$name")) {
        echo "Upload successful!";
    } else {
        echo "Error while uploading";
    }
} else {
    echo "No file to upload";
}

?>