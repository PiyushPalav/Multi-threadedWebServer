#include "http_server.h"

#include <vector>

#include <sys/stat.h>

#include <fstream>
#include <sstream>
#include <errno.h>

vector<string> split(const string &s, char delim) {
  vector<string> elems;

  stringstream ss(s);
  string item;

  while (getline(ss, item, delim)) {
    if (!item.empty())
      elems.push_back(item);
  }

  return elems;
}

string get_file_contents(string filename) {
  ifstream in(filename, std::ios::in);
  if (in) {
    string file_contents;
    in.seekg(0, in.end);
    file_contents.resize(in.tellg());
    in.seekg(0, in.beg);
    in.read(&file_contents[0], file_contents.size());
    in.close();
    return(file_contents);
  }
  throw(errno);
}

HTTP_Request::HTTP_Request(string request) {
  vector<string> lines = split(request, '\n');
  vector<string> first_line = split(lines[0], ' ');

  this->HTTP_version = "1.0"; // We'll be using 1.0 irrespective of the request

  /*
   TODO : extract the request method and URL from first_line here
  */

  this->method = first_line[0];
  this->url = first_line[1];

  if (this->method != "GET") {
    cerr << "Method '" << this->method << "' not supported" << endl;
    exit(1);
  }
}

HTTP_Response *handle_request(string req) {
  HTTP_Request *request = new HTTP_Request(req);

  HTTP_Response *response = new HTTP_Response();

  string url = string("html_files") + request->url;

  response->HTTP_version = "1.0";

  struct stat sb;
  if (stat(url.c_str(), &sb) == 0) // requested path exists
  {
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";

    string body;
    ifstream f;

    if (S_ISDIR(sb.st_mode)) {
      /*
      In this case, requested path is a directory.
      TODO : find the index.html file in that directory (modify the url
      accordingly)
      */
     url = url + string("/index.html");
    }

    /*
    TODO : open the file and read its file_contents
    */
    body = get_file_contents(url);

    /*
    TODO : set the remaining fields of response appropriately
    */
    response->url = url;
    response->body = body;
    response->content_length = response->body.size();
  }

  else {
    response->status_code = "404";
    response->status_text = "Not Found";
    /*
    TODO : set the remaining fields of response appropriately
    */
    response->content_type = "text/html";
    response->url = url;
    response->body = get_file_contents("html_files/error.html");
    response->content_length = response->body.size();
  }

  delete request;

  return response;
}

string HTTP_Response::get_string() {
  /*
  TODO : implement this function
  */
 string response_string;

 response_string = string("HTTP/") + string(HTTP_version) + " " + string(status_code) + " " + string(status_text) + "\r\n";
 response_string += string("Content-Type: ") + string(content_type) + "\r\n";
 response_string += string("Content-Length: ") + to_string(content_length) + "\r\n";
 response_string += "\r\n";
 response_string += string(body);

 return response_string;
}