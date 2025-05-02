#include "Method.hpp"

Method::Method() {
	// Constructor implementation
	std::cout << "Default constructor for class Method called." << std::endl;
}

Method::Method(const Method &other) {
	// Copy constructor implementation
	std::cout << "Copy constructor for class Method called." << std::endl;
}

Method &Method::operator=(const Method &other) {
	// Assignment operator implementation
	if (this != &other) {
		// Copy attributes
	}
	return *this;
}

Method::~Method() {
	// Destructor implementation
}

const std::string& Method::get_method() const {
	return method; 
}

const std::string& Method::get_uri() const {
	return uri;
}

const std::string& Method::get_version() const {
	return version;
}