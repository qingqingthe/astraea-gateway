#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_client.hh"
#include <stdio.h>
#include <iostream>
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
                map<string, string> params = {{"request", to_string(i)}, {"ip_client", "client:12346"}};
                restCallPost("http://localhost:4000/read_request_from_client", params);
            };
        },

        POST / _send_write_requests * get_parameters(_max = int()) = [] (auto param) {
            cout << "start sending write requests." << endl;

            // send write requests to gateway
            for (int i = 0; i < param.max; i++) {
                map<string, string> params = {{"request", to_string(i)}, {"ip_client", "client:12346"}};
                restCallPost("http:///localhost:4000/write_request_from_client", params);
            }
        },

        POST / _send_read_and_write_requests * get_parameters(_max = int()) = [] (auto param) {
            cout << "start sending read and write requests." << endl;

            // send read and write requests to gateway
            for (int i = 0; i < param.max; i++) {
                waitSomeTime(rand() % 10);
                int r = rand() % 2;
                log(r);
                map<string, string> params = {{"request", to_string(i)}, {"ip_client", "client:12346"}};
                if (r)
                    restCallPost("http://localhost:4000/read_request_from_client", params);
                else
                    restCallPost("http://localhost:4000/write_request_from_client", params);
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
