#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_client.hh"
#include <stdio.h>
#include <iostream>
#include <thread>
#include <stdlib.h>
#include "http_request_helper.h"

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace
using namespace std;

auto client_api = http_api(

        GET / _hello_world = [] () {
            return D(_message = "hello world from the client api");
        },

        POST / _send_read_requests * get_parameters(_max = int()) = [] (auto param) {
            cout << "start sending read requests." << endl;

            // send read requests to gateway (asynchronously)
            for (int i = 0; i < param.max; i++) {
                thread t0(restCallPost, (string("gateway:12345/read_request_from_client?request=") + to_string(i) + string("&ip_client=client%3A12346")));
                t0.detach();
            }
        },

        POST / _send_write_requests * get_parameters(_max = int()) = [] (auto param) {
            cout << "start sending write requests." << endl;

            // send write requests to gateway
            for (int i = 0; i < param.max; i++) {
                restCallPost(string("gateway:12345/write_request_from_client?request=") + to_string(i) + string("&ip_client=client%3A12346"));
            }
        },

        POST / _send_read_and_write_requests * get_parameters(_max = int()) = [] (auto param) {
            cout << "start sending read and write requests." << endl;

            // send read and write requests to gateway
            for (int i = 0; i < param.max; i++) {
                int r = rand() % 2;
                log(r);
                if (r)
                    restCallPost(string("gateway:12345/read_request_from_client?request=") + to_string(i) + string("&ip_client=client%3A12346"));
                else
                    restCallPost(string("gateway:12345/write_request_from_client?request=") + to_string(i) + string("&ip_client=client%3A12346"));
            }
        },

        POST / _response *get_parameters(_response) = [] (auto param) {
            log("received response " + param.response);
        }
);

int main()
{
    srand (time(NULL));
    sl::mhd_json_serve(client_api, 12346);
}
