//
// Created by Dana Kuban on 2018-12-19.
//

#ifndef SILICON_HTTP_REQUEST_HELPER_H
#define SILICON_HTTP_REQUEST_HELPER_H

#include <stdio.h>
#include <iostream>
#include <curl/curl.h>
#include <jsoncpp/json.h>
#include <chrono>
#include <thread>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <map>
#include <string>
#include <mutex>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
mutex mt;

struct request {
    string message;
    string ip_client;
    chrono::milliseconds timestamp;
    bool read_request;
public:
    string toString() {
        cout << message;
        cout << ip_client;
        cout << timestamp.count();
        ostringstream ss;
        ss << "{\n\tmessage: " << message << ",\n\tip_client: " << ip_client
           << ",\n\ttimestamp: " << to_string(timestamp.count())
           << ",\n\tread_request: " << std::boolalpha << read_request << "\n}" << endl;
        return ss.str();
    }
};

string createForm(string ip_client, string request) {
    ostringstream ss;
    ss << "{\"User\": \"" << ip_client << "\", \"Query\": \"" << request << "\"}";
    return ss.str();
}

void waitSomeTime(int sleepTime) {
    this_thread::sleep_for(chrono::milliseconds(sleepTime));
}

void log(string message) {
    mt.lock();
    const boost::posix_time::ptime now =
            boost::posix_time::microsec_clock::local_time();
    const boost::posix_time::time_duration td = now.time_of_day();
    char buf[40];
    sprintf(buf, "%02i:%02i:%02i.%03ld",
            td.hours(), td.minutes(), td.seconds(), td.total_milliseconds() -
                                                    ((td.hours() * 3600 + td.minutes() * 60 + td.seconds()) * 1000));
    cout << buf << " " << message << endl;
    mt.unlock();
}

size_t static WriteCallback(char *contents, size_t size, size_t nmemb, void *userp) {
    ((string *) userp)->append((char *) contents, size * nmemb);
    return size * nmemb;
}

string getResponseFromJson(string input) {
    if (input.empty()) {
        return input;
    }
    // TODO: find an adequate json implementation
    // I define that in every response there is exactly one value called "response"
    string response = input.substr(string("{\"response\":\"").length(),
                                   input.length() - string("{\"response\"=\"").length() - 2);
    return response;
}

string escapeParams(CURL *curl, string url, map <string, string> params) {
    // encode params
    map<string, string>::iterator it;
    map <string, string> encodedParams;
    for (it = params.begin(); it != params.end(); it++) {
        encodedParams.insert(pair<string, string>(string(curl_easy_escape(curl, it->first.c_str(), it->first.length())),
                                                  string(curl_easy_escape(curl, it->second.c_str(),
                                                                          it->second.length()))));
    }

    // merge url
    if (!encodedParams.empty()) {
        url += "?";
    }
    for (it = encodedParams.begin(); it != encodedParams.end(); it++) {
        if (!(url.back() == '?')) {
            url += "&";
        }
        url += it->first + "=" + it->second;
    }

    return url;
}

string restCallGet(string url, map <string, string> params) {
    string response;
    CURL *curl;
    curl = curl_easy_init();
    if (curl) {
        url = escapeParams(curl, url, params);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        response = getResponseFromJson(response);
        log("called " + url);
        curl_easy_cleanup(curl);
    }
    return response;
}

void restCallPostAsync(string url, map <string, string> params) {
    CURL *curl;
    curl = curl_easy_init();
    if (curl) {
        url = escapeParams(curl, url, params);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 0.001);
        CURLcode res = curl_easy_perform(curl);
        log("called " + url);
        curl_easy_cleanup(curl);
    }
}

void restCallPost(string url, map <string, string> params) {
    thread t0(restCallPostAsync, url, params);
    t0.detach();
}

#endif //SILICON_HTTP_REQUEST_HELPER_H
