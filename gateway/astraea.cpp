#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_astraea.hh"
#include <stdio.h>
#include <iostream>
#include <mutex>
#include "http_request_helper.h"

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace

bool ack_process_is_running = false;
mutex m;

auto auth_api = http_api(

        GET / _hello_world = [] () {
            return D(_response = "hello world from the Astraea api");
        },

        POST / _write * get_parameters(_form) = [] (auto param) {
            log("Astraea received request " + param.form);

            // send request to logger
            map<string, string> params = {{"message", param.form}};
            restCallPost("logger:12345/log", params);

            // check if process is currently running
            m.lock();
            if(ack_process_is_running) {
                log("ack process is running");
                m.unlock();
                return;
            } else {
                ack_process_is_running = true;
                m.unlock();
            }

            // mock generating M.C. value
            waitSomeTime(80);

            // respond
            params = {{"response", string(param.form)}};
            restCallPost("gateway:4000/release", params);
            // reset running flag
            ack_process_is_running = false;
        }
);

int main()
{
    sl::mhd_json_serve(auth_api, 5000, _one_thread_per_connection, _nthreads = 3);
}

