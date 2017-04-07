#include "http-client.h"
#include "curl/curl.h"
#include <iostream>

using namespace rcar;
            
static size_t writeDataCallback( void* buffer, size_t size, size_t nmemb, void* lp_void ) {
    //std::string *response = dynamic_cast<std::string*>((std::string*)lp_void);
    if(buffer != NULL )  {    
        std::cout << "Baidu Response Message:" << std::endl << (char *)buffer  << std::endl;
    }
    return size * nmemb;
}

int HttpClient::post(const std::string &url, const std::string &input ) {
    CURL* curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
    curl_easy_setopt(curl, CURLOPT_POST, 1 );
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, input.c_str() );
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback );
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1 );
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30 );
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30 );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1 );
    
     
    struct curl_slist *list = NULL;
    list = curl_slist_append(list, 
            "Content-Type:application/x-www-form-urlencoded;charset=utf-8");
            //"Content-Type:application/json;charset=utf-8");
    list = curl_slist_append(list, 
            "User-Agent:BCCS_SDK/3.0(Linux;Linux kernel version 15.0.0)");
 
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_perform(curl );
    curl_easy_cleanup(curl );
    return 0;
}

int HttpClient::get(const std::string &url, const std::string &input ) {
    static std::string response; // maybe not threadsafe
    CURL* curl = curl_easy_init();

    response = "";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
    curl_easy_setopt(curl, CURLOPT_POST, 1 );
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, input.c_str() );
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL );
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback );
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1 );
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3 );
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3 );
    curl_easy_perform(curl );
    curl_easy_cleanup(curl );
    return 0;
}
