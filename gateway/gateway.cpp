#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_gateway.hh"
#include <stdio.h>
#include "http_request_helper.h"
#include <list>
#include <iterator>
#include <thread>
#include <mutex>
#include <queue>

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace
using namespace std;

queue<request> readQueue;
queue<request> writeQueue;

// for debugging reasons
void printQueue(const queue<request> &q) {
    queue<request> duplicate = q;
    while (!duplicate.empty()) {
        cout << &duplicate.front() << endl;
        cout << duplicate.front().toString() << endl;
        duplicate.pop();
    }
}

// get all requests from queue that were sent to astraea
list<request> getRequestsSentToAstraea(queue<request> &q) {
    list<request> requests;
    queue<request> duplicate = q;
    int countPops = 0;
    while (duplicate.front().is_sent_to_astraea || duplicate.front().timestamp < time(nullptr) - 5) {
        // delete outdated requests
        if (duplicate.front().timestamp < time(nullptr) - 5) {
            log("queued request is outdated: " + duplicate.front().message);
            duplicate.pop();
            countPops ++;
        }

        // store valid requests in list
        requests.push_back(duplicate.front());
        duplicate.pop();
        countPops ++;

        // break if queue is empty
        if (duplicate.empty()) {
            log("read queue is empty!");
            break;
        }
    }

    // pop outdated and responded requests from actual list
    for (int i = 0; i < countPops; i++) {
        q.pop();
    }

    return requests;
}

auto gateway_api = http_api(

        GET / _hello_world = [] () {
            return D(_message = "hello world from the gateway api");
        },

        POST / _read_request_from_client * get_parameters(_request = string(), _ip_client = string()) = [] (auto param) {
            log("gateway received read request " + string(param.request) + " from " + param.ip_client);

            // send request to db
            string response = restCallGet(string("database:12345/read_request?request=") + string(param.request));

            // send request to astraea (asynchronously)
            thread t0(restCallPost, (string("astraea:12345/ack_read_response?response=") + string(response)));
            t0.detach();

            // push request to read queue
            struct request request = { response, param.ip_client, time(nullptr), true };
            readQueue.push(request);
        },

        POST / _read_response_from_auth * get_parameters(_response) = [] (auto param) {
            log("received read response " + string(param.response) + " from astraea");

            // get all requests from queue that were sent to astraea
            list<request> requests = getRequestsSentToAstraea(readQueue);
            log("found " + to_string(requests.size()) + " requests to respond to");

            // send responses
            for (list<request>::iterator it = requests.begin(); it != requests.end(); ++it) {
                //log("found request" + it->message + it->ip_client);
                restCallPost(string(it->ip_client) + string("/response?response=") + it->message);
            }
        },

        POST / _write_request_from_client * get_parameters(_request, _ip_client) = [] (auto param) {
            log("gateway received write request " + string(param.request) + " from " + param.ip_client);

            // send request to astraea
            thread t0(restCallPost, (string("astraea:12345/ack_write_request?request=") + string(param.request)));
            t0.detach();

            // push request to queue
            struct request request = { param.request, param.ip_client, time(nullptr), true };
            writeQueue.push(request);
        },

        POST / _write_response_from_auth * get_parameters(_response) = [] (auto param) {
            log("received write response " + param.response + " from astraea");

            // get all requests from queue that were sent to astraea
            list<request> requests = getRequestsSentToAstraea(writeQueue);
            log("found " + to_string(requests.size()) + " requests to respond to");

            for (list<request>::iterator it = requests.begin(); it != requests.end(); ++it) {
                //log("found request" + it->message + it->ip_client);

                // send request to db
                string response = restCallGet(string("database:12345/write_request?request=") + string(it->message));

                // return response to client
                restCallPost(string(it->ip_client) + string("/response?response=") + response);
            }
        }
);

int main()
{
    sl::mhd_json_serve(gateway_api, 12345, _linux_epoll, _nthreads = 20);
}
