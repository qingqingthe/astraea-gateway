#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_database.hh"
#include <stdio.h>
#include "http_request_helper.h"

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace

auto logger_api = http_api(

        GET / _hello_world = [] () {
            return D(_message = "hello world from the database api");
        },

        GET / _read_request * get_parameters(_request) = [] (auto param) {
            log("Database received read request " + param.request);
            return D(_response = "db_read_response_" + param.request);
        },

        GET / _write_request * get_parameters(_request) = [] (auto param) {
            log("Database received write request " + param.request);
            return D(_response = "db_write_response_" + param.request);
        }
);

int main()
{
    sl::mhd_json_serve(logger_api, 3306);
}

