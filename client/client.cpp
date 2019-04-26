#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_client.hh"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "http_request_helper.h"
#include <ctime>
#include <vector>
#include <mutex>


using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace
using namespace std;
using namespace std::chrono;


std::mutex mtx;

static const int INT_BUF = 12;
static const int TIMEOUT = 10; 

vector<long int> receive_times;
unsigned int last_second;



void record_response_times() {
    FILE *fp = fopen("/received/received.csv", "w");
    char str_millis[INT_BUF];
    log("recording initialized");
    if (receive_times.size() > 0) {
        for (int i = 0; i < receive_times.size(); i++){
            long int millis = receive_times[i];
            sprintf(str_millis, "%ld", millis);
            fprintf(fp, "%s\n", str_millis);
            log("recorded str_millis " + string(str_millis));
//             myfile << str_millis;
//             myfile << "\n";
            memset(str_millis, 0, sizeof(str_millis));
        }
        fclose(fp);
        log("after closing?");
        FILE *afp = fopen("/received/response_times.csv", "r");
        char ch;
        while((ch = fgetc(afp)) != EOF) {
            printf("%c", ch);
        }
        fclose(afp);
        printf("%s", "file complete\n");
    }
    log("kann man etwas hier schreiben?");
    return;
} 



bool check_timeout(unsigned int last_second, unsigned int current_second) {
    if ((last_second + TIMEOUT) > current_second) {
        return false;
    }
    return true;
}

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
                map<string, string> params = {{"request", to_string(i)}, {"ip_client", "client:6000"}};
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
            long int ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            std::time_t current_second = std::time(0);
            if (receive_times.size() == 0) {
                receive_times.push_back(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
                last_second = current_second; 
            }
            if (check_timeout(last_second, current_second) == true){
                record_response_times();
            } else {
                 receive_times.push_back(ms); 
            }
            last_second = current_second;
         }
);

int main(){
    log("STARTING CLIENT");
    srand (time(NULL));
    sl::mhd_json_serve(client_api, 6000, _nthreads = 2);
}
