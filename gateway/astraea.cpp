#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_astraea.hh"
#include <stdio.h>
#include <iostream>
#include <mutex>
#include <chrono>
#include "http_request_helper.h"

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace
using namespace chrono;

bool ack_process_is_running = false;
mutex m;

string getIdFromForm(string form) {
    return form.substr(string("{\"Id\": \"").length(), form.substr(string("{\"Id\": \"").length(), form.length()).find("\""));
}

void generateMCValue(string form) {
    // mock generating M.C. value
    waitSomeTime(80);

    // respond
    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    map <string, string> params = {{"response", form}, {"timestamp", to_string(ms.count())}, {"id", getIdFromForm(form)}};
    restCallPost("gateway:4000/release", params);

    // reset running flag
    ack_process_is_running = false;
}

auto auth_api = http_api(

        GET / _hello_world = [] () {
            return D(_response = "hello world from the Astraea api");
        },

        GET / _write * get_parameters(_form) = [] (auto param) {
            log("Astraea received request " + param.form);

            // send request to logger
            map <string, string> params = {{"message", param.form}};
            restCallPost("logger:12345/log", params);

            // check if process is currently running
            m.lock();
            if (ack_process_is_running) {
                log("ack process is running");
                m.unlock();
                return D(_response = "blocked");
            } else {
                thread t0(generateMCValue, param.form);
                t0.detach();
                ack_process_is_running = true;
                m.unlock();
                return D(_response = "accepted");
            }
        }
);

int main()
{
    sl::mhd_json_serve(auth_api, 5000);
}

