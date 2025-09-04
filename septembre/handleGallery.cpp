#include "handleGallery.hpp"

// Génère le code HTML pour la liste des photos
std::string generatePhotoList(const std::string &dirPath) {
    DIR *dir;
    struct dirent *ent;
    std::ostringstream html;

    dir = opendir(dirPath.c_str());
    if (dir == NULL)
        return "<p>No photos found.</p>";

    while ((ent = readdir(dir)) != NULL) {
        std::string filename = ent->d_name;
        // On ignore . et ..
        if (filename == "." || filename == "..")
            continue;

        // Ici tu peux aussi filtrer par extension (.jpg, .png, etc.)
        html << "<div class=\"photo-card\">"
             << "<img src=\"/photos/" << filename << "\" alt=\"" << filename << "\">"
             << "<form action=\"/photos/" << filename << "?_method=DELETE\" method=\"POST\">"
             << "<button type=\"submit\">Delete</button>"
             << "</form></div>\n";
    }
    closedir(dir);
    return html.str();
}

// Charge le template et remplace {{PHOTO_LIST}}
std::string renderGallery(const std::string &templatePath, const std::string &photoDir) {
    std::ifstream file(templatePath.c_str());
    if (!file.is_open())
        return "<h1>Error: cannot open template</h1>";

    std::ostringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string content = buffer.str();
    std::string placeholder = "{{PHOTO_LIST}}";
    std::string photos = generatePhotoList(photoDir);

    size_t pos = content.find(placeholder);
    if (pos != std::string::npos)
        content.replace(pos, placeholder.length(), photos);

    return content;
}
