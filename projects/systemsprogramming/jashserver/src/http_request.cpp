#include "http_request.h"

#include <sstream>
#include <iterator>

bool HTTPRequest::parse(const std::string& request)
{
    std::istringstream stream(request);

    stream >> method >> path >> version;

    std::string line;

    std::getline(stream, line);

    while (std::getline(stream, line))
    {
        if (line == "\r")
        {
            break;
        }

        auto separator = line.find(':');

        if (separator == std::string::npos)
        {
            continue;
        }

        std::string key =
            line.substr(0, separator);

        std::string value =
            line.substr(separator + 1);

        headers[key] = value;
    }

    body.assign(
        std::istreambuf_iterator<char>(stream),
        std::istreambuf_iterator<char>()
    );

    return true;
}