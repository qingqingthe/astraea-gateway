#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_gateway.hh"
#include <stdio.h>
#include "http_request_helper.h"
#include <list>
#include <iterator>
#include <thread>
#include <queue>

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace
using namespace std;

queue<request*> readQueue;
queue<request*> writeQueue;
queue<request*> demoQueue;
bool initiateQueues = false;

auto gateway_api = http_api(

        GET / _hello_world = [] () {
            return D(_message = "hello world from the gateway api");
        },

        POST / _read_request_from_client * get_parameters(_request = string(), _ip_client = string()) = [] (auto param) {
            log("gateway received read request " + string(param.request) + " from " + param.ip_client);

            // push request to read queue
            struct request request = { param.request, param.ip_client, time(nullptr), false };
            struct request *ptr;
            ptr = &request;
            readQueue.push(ptr);
            log("read queue size: " + to_string(readQueue.size()));

            // send request to db
            string response = restCallGet(string("database:12345/read_request?request=") + string(param.request));

            // send request to astraea (asynchronously)
            thread t0(restCallPost, (string("astraea:12345/ack_read_response?response=") + string(response)));
            t0.detach();

            // set sent to astraea flag to true
            ptr->is_sent_to_astraea = true;
            log("flag: " + string(readQueue.front()->is_sent_to_astraea ? "true" : "false"));
        },

        POST / _read_response_from_auth * get_parameters(_response) = [] (auto param) {
            log("received read response " + string(param.response) + " from astraea");

            // get all requests from queue that were sent to astraea
            list<request*> requests;
            while (readQueue.front()->is_sent_to_astraea || readQueue.front()->timestamp < time(nullptr) - 5) {
                // delete outdated requests
                if (readQueue.front()->timestamp < time(nullptr) - 5) {
                    log("queued request is outdated: " + readQueue.front()->message);
                    readQueue.pop();
                }

                requests.push_back(readQueue.front());
                readQueue.pop();
                log("request popped from queue: " + requests.front()->message);
            } log(to_string(requests.size()));

            // send responses
            for (list<request*>::iterator it = requests.begin(); it != requests.end(); ++it) {
                log("found request" + (*it)->message + (*it)->ip_client);

                // return response to client
                restCallPost(string((*it)->ip_client) + string("/response?response=") + param.response);
            }

            log("end of function");
        },

        POST / _write_request_from_client * get_parameters(_request, _ip_client) = [] (auto param) {
            log("gateway received write request " + string(param.request) + " from " + param.ip_client);

            // push request to queue
            struct request request = { param.request, param.ip_client, time(nullptr), false };
            struct request *ptr;
            ptr = &request;
            writeQueue.push(ptr);
            log("write queue size: " + to_string(readQueue.size()));

            // send request to astraea
            thread t0(restCallPost, (string("astraea:12345/ack_write_request?request=") + string(param.request)));
            t0.detach();
            log("gateway sent write request to astraea");

            // set sent to astraea flag to true
            ptr->is_sent_to_astraea = true;
        },

        POST / _write_response_from_auth * get_parameters(_response) = [] (auto param) {
            log("received write response " + param.response + " from astraea");

            // get all requests from queue that were sent to astraea
            list<request*> requests;
            while (writeQueue.front()->is_sent_to_astraea || writeQueue.front()->timestamp < time(nullptr) - 5) {
                // delete outdated requests
                if (writeQueue.front()->timestamp < time(nullptr) - 5) {
                    log("queued request is outdated: " + writeQueue.front()->message);
                    writeQueue.pop();
                }

                requests.push_back(writeQueue.front());
                writeQueue.pop();
                log("request popped from queue: " + requests.front()->message);
            } log(to_string(requests.size()));

            for (list<request*>::iterator it = requests.begin(); it != requests.end(); ++it) {
                log("found request" + (*it)->message + (*it)->ip_client);

                // send request to db
                string response = restCallGet(string("database:12345/write_request?request=") + string((*it)->message));
                log(response);

                // return response to client
                restCallPost(string((*it)->ip_client) + string("/response?response=") + response);
            }

            log("end of function");
        }
);

int main()
{
    sl::mhd_json_serve(gateway_api, 12345, _linux_epoll, _nthreads = 10);
}
