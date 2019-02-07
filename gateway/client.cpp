#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_client.hh"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <map>
#include <ctime>
#include <time.h>
#include <chrono>
#include <mutex>
#include "http_request_helper.h"

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace
using namespace std;
using namespace std::chrono;

map<string, double> durations;
map<string, milliseconds> start_times;
string currentFileName;

void setCurrentFileName(int max, bool r, bool w) {
    auto timeNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
    stringstream buffer;
    buffer << put_time(std::localtime(&timeNow), "%H:%M:%S");
    ostringstream ss;
    ss << setw(3) << setfill('0') << max;
    currentFileName = "/data/max_" + ss.str() + "_requests_" + (r ? "r_" : "") + (w ? "w_" : "") + buffer.str();
    ofstream outfile(currentFileName.c_str());
    outfile.close();
    log(currentFileName);
}

auto client_api = http_api(

        GET / _hello_world = [] () {
            return D(_message = "hello world from the client api");
        },

        POST / _send_read_requests * get_parameters(_max = int()) = [] (auto param) {
            cout << "start sending read requests." << endl;

            // create file to write durations to
            setCurrentFileName(param.max, true, false);

            // send read requests to gateway (asynchronously)
            for (int i = 0; i < param.max; i++) {
                // get current timestamp, store request
                milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                start_times.insert(pair<string, milliseconds>("db_read_response_" + to_string(i), ms));

                // send request
                map<string, string> params = {{"request", to_string(i)}, {"ip_client", "client:12346"}};
                restCallPost("gateway:4000/read_request_from_client", params);
            };
        },

        POST / _send_write_requests * get_parameters(_max = int()) = [] (auto param) {
            cout << "start sending write requests." << endl;

            // create file to write durations to
            setCurrentFileName(param.max, false, true);

            // send write requests to gateway
            for (int i = 0; i < param.max; i++) {
                // get current timestamp, store request
                milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                start_times.insert(pair<string, milliseconds>("db_write_response_" + to_string(i), ms));

                // send request
                map<string, string> params = {{"request", to_string(i)}, {"ip_client", "client:12346"}};
                restCallPost("gateway:4000/write_request_from_client", params);
            }
        },

        POST / _send_read_and_write_requests * get_parameters(_max = int()) = [] (auto param) {
            cout << "start sending read and write requests." << endl;

            // create file to write durations to
            setCurrentFileName(param.max, true, true);

            // send read and write requests to gateway
            for (int i = 0; i < param.max; i++) {
                waitSomeTime(rand() % 10);

                // decide whether to send read or write request
                int r = rand() % 2;

                // set params
                map<string, string> params = {{"request", to_string(i)}, {"ip_client", "client:12346"}};

                // get current timestamp
                milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

                if (r) {
                    start_times.insert(pair<string, milliseconds>("db_read_response_" + to_string(i), ms));
                    restCallPost("gateway:4000/read_request_from_client", params);
                } else {
                    start_times.insert(pair<string, milliseconds>("db_write_response_" + to_string(i), ms));
                    restCallPost("gateway:4000/write_request_from_client", params);
                }
            }
        },

        POST / _response *get_parameters(_response) = [] (auto param) {
            auto it = start_times.find(param.response);
            if (it == start_times.end()) {
                log("received response that was not expected: " + param.response);
            } else {
                // compute duration of response
                double diff = difftime(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(), it->second.count());
                log("received response " + param.response + " in " + to_string(diff) + " ms");
                durations.insert(pair<string, double>(param.response, diff));
                start_times.erase(param.response);
            }
        },

        POST / _write_results = [] () {
            // open file
            ofstream outfile;
            outfile.open(currentFileName.c_str(), ios::app);

            // write all successful requests in file
            map<string, double>::iterator it;
            for (it = durations.begin(); it != durations.end(); it++) {
                outfile << it->first << ";" << to_string(it->second).c_str() << endl;
            }

            // write all missing requests in file
            map<string, milliseconds>::iterator it2;
            for (it2 = start_times.begin(); it2 != start_times.end(); it2++) {
                log("missing response " + it2->first);
                outfile << "missing response " << it2->first << ";" << to_string(-1).c_str() << endl;
            }

            start_times.clear();
            durations.clear();
            outfile.close();
        }
);

int main()
{
    srand (time(NULL));
    sl::mhd_json_serve(client_api, 12346);
}
