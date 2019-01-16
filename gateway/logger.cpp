#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_logger.hh"
#include <stdio.h>
#include "http_request_helper.h"

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace

auto logger_api = http_api(

        GET / _hello_world = [] () {
            log("Logger received hello world call");
            return D(_message = "hello world from the logger api");
        },

        POST / _read_request * get_parameters(_request) = [] (auto param) {
            log("Logger received read request " + param.request);
        },

        POST / _write_request * get_parameters(_request) = [] (auto param) {
            log("Logger received write request " + param.request);
        },

        POST / _db_response * get_parameters(_response) = [] (auto param) {
            log("Logger received db response " + param.response);
        }
);

int main()
{
    sl::mhd_json_serve(logger_api, 12345);
}

