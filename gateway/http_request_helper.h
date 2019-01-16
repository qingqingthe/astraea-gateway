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

using namespace std;

struct request { string message; string ip_client; time_t timestamp; bool is_sent_to_astraea; };

void waitSomeTime(int sleepTime) {
    this_thread::sleep_for(chrono::milliseconds(sleepTime));
}

void log(string message) {
    chrono::system_clock::time_point now = chrono::system_clock::now();
    time_t now_c = std::chrono::system_clock::to_time_t(now);
    cout << put_time(std::localtime(&now_c), "%F %T") << " " << message << endl;
}

size_t static WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string getResponseFromJson(string input) {
    if(input.empty()) {
        return input;
    }
    // TODO: find an adequate json implementation
    // I define that in every response there is exactly one value called "response"
    string response = input.substr(string("{\"response\":\"").length(), input.length() - string("{\"response\"=\"").length() - 2);
    return response;
}

string restCallGet(string url) {
    string response;
    CURL *curl;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        response = getResponseFromJson(response);
        curl_easy_cleanup(curl);
    }
    return response;
}

void restCallPost(string url) {
    CURL *curl;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 0.001);
        CURLcode res = curl_easy_perform(curl);
        log("called " + url);
        curl_easy_cleanup(curl);
    }
}

#endif //SILICON_HTTP_REQUEST_HELPER_H
