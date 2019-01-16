#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_client.hh"
#include <stdio.h>
#include <iostream>
#include <thread>
#include "http_request_helper.h"

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace
using namespace std;

auto client_api = http_api(

        GET / _hello_world = [] () {
            return D(_message = "hello world from the client api");
        },

        POST / _send_read_requests = [] () {
            cout << "start sending read requests." << endl;

            // send requests to gateway (asynchronously)
            for (int i = 0; i < 100; i++) {
                thread t0(restCallPost, (string("gateway:12345/read_request_from_client?request=") + to_string(i) + string("&ip_client=client%3A12346")));
                t0.detach();
            }
        },

        POST / _send_write_requests = [] () {
            cout << "start sending write requests." << endl;

            // send request to gateway
            for (int i = 0; i < 100; i++) {
                restCallPost(string("gateway:12345/write_request_from_client?request=") + to_string(i) + string("&ip_client=client%3A12346"));
                //waitSomeTime(5000);
            }
        },

        POST / _response *get_parameters(_response) = [] (auto param) {
            log("received response " + param.response);
        }
);

int main()
{
    sl::mhd_json_serve(client_api, 12346);
}
