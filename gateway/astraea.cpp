#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_astraea.hh"
#include <stdio.h>
#include <iostream>
#include "http_request_helper.h"

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace

bool ack_process_is_running = false;

auto auth_api = http_api(

        GET / _hello_world = [] () {
            return D(_response = "hello world from the Astraea api");
        },

        POST / _ack_read_response * get_parameters(_response) = [] (auto param) {
            log("Astraea received read response " + param.response);

            // check if process is currently running
            if(ack_process_is_running) {
                log("ack process is running");
                return;
            } else {
                ack_process_is_running = true;
            }

            // send request to logger
            restCallPost(string("logger:12345/db_response?response=") + string(param.response));

            // mock generating M.C. value
            waitSomeTime(80);

            // respond
            restCallPost(string("gateway:12345/read_response_from_auth?response=auth_response_") + param.response);

            // reset running flag
            ack_process_is_running = false;
        },

        POST / _ack_write_request * get_parameters(_request) = [] (auto param) {
            log("Astraea received write request " + param.request);

            // check if process is currently running
            if(ack_process_is_running) {
                log("ack process is running");
                return;
            } else {
                ack_process_is_running = true;
            }

            // send request to logger
            restCallPost(string("logger:12345/write_request?request=") + string(param.request));

            // mock generating M.C. value
            waitSomeTime(80);

            // respond
            restCallPost(string("gateway:12345/write_response_from_auth?response=auth_response_") + param.request);

            // reset running flag
            ack_process_is_running = false;
        }
);

//auto auth_api_ga = add_global_middlewares<request_logger>::to(auth_api);

int main()
{
    sl::mhd_json_serve(auth_api, 12345, _one_thread_per_connection, _nthreads = 3);
}

