#ifndef Method_HPP
#define Method_HPP

# include <iostream>
# include <string>
# include <map>

class Method {
public:
	Method();
	Method(const Method &other);
	Method &operator=(const Method &other);
	virtual ~Method();

	virtual void parse_request(const std::string &raw_request) = 0;
    virtual std::string generate_response() const = 0;

    const std::string &get_method() const { return method; }
    const std::string &get_uri() const { return uri; }
    const std::string &get_version() const { return version; }

protected:
    std::string method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

#endif // Method_HPP