#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_gateway.hh"
#include <stdio.h>
#include "http_request_helper.h"
#include <list>
#include <iterator>
#include <mutex>
#include <queue>

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace
using namespace std;
using namespace chrono;

queue <request> readQueue;
queue <request> writeQueue;
// this flag is to remember which kind of request was sent to astraea and has not been responded yet
bool answerReadRequests;
// this flags are to remember which kind of request can be sent currently to astraea
bool sendReadRequestsToAstraea = true;
bool sendWriteRequestsToAstraea = true;
/**
 * normal case: some requests are sent to astraea. flags are set, when astraea responds, we know by the flags which
 * requests were processed by astraea.
 *
 * case which needs all three flags: some read requests are sent to astraea:
 * sendReadRequestsToAstraea = true;
 * sendWriteRequestsToAstraea = false;
 * astraea responds -> before the response arrives at the gateway another request arrives at astraea,
 * is processed quickly and arrives at the gateway before the (previous) response
 * so the flags are reset before we can evaluate to which requests we have to respond
*/
mutex m;

// for debugging reasons
void printQueue(queue <request> &q) {
    queue <request> duplicate = q;
    while (!duplicate.empty()) {
        cout << &duplicate.front() << endl;
        cout << duplicate.front().toString() << endl;
        duplicate.pop();
    }
}

void sendRequestToAstraea(string ip_client, string request, bool readRequest) {
    map <string, string> params = {{"form", createForm(string(ip_client), string(request))}};
    string response = restCallGet("astraea:5000/write", params);

    // flag to decide whether request is processed by astraea
    milliseconds timestamp(0);
    if (response == "accepted") {
        log("accepted: " + request);
        // block sending write requests to astraea
        sendReadRequestsToAstraea = readRequest;
        sendWriteRequestsToAstraea = !readRequest;
        answerReadRequests = readRequest;
        timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    } else if (response == "blocked" &&
               ((sendReadRequestsToAstraea && readRequest) || (sendWriteRequestsToAstraea && !readRequest))) {
        // astraea is generating mc value for this kind of requests
        log("blocked: " + request);
        timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    }

    // push request to queue
    struct request r = {request, ip_client, timestamp, readRequest};
    if (readRequest) {
        readQueue.push(r);
        //printQueue(readQueue);
    } else {
        writeQueue.push(r);
        //printQueue(writeQueue);
    }
}

// get all requests from queue that were sent to astraea
list <request> getRequestsSentToAstraea(double response_ms) {
    // decide with answerReadRequests flag which queue to touch
    queue <request> &queueInQuestion = answerReadRequests ? readQueue : writeQueue;
    queue <request> &otherQueue = answerReadRequests ? writeQueue : readQueue;
    list <request> requestsToHandle;
    list <request> requestsToResend;

    while (!queueInQuestion.empty()) {
        if (queueInQuestion.front().timestamp.count() == 0) {
            // request was not sent to astraea
            //log("never: " + to_string(queueInQuestion.front().timestamp.count()) + ", " + queueInQuestion.front().message);
            request r = queueInQuestion.front();
            requestsToResend.push_back(queueInQuestion.front());
            queueInQuestion.pop();
        } else if (queueInQuestion.front().timestamp.count() < response_ms) {
            // request was sent to astraea before astraea responded
            //log("in time: " + to_string(queueInQuestion.front().timestamp.count()) + ", " + queueInQuestion.front().message);
            requestsToHandle.push_back(queueInQuestion.front());
            queueInQuestion.pop();
        } else {
            // request was sent after response to astraea
            //log("too late: " + to_string(queueInQuestion.front().timestamp.count()) + ", " + queueInQuestion.front().message);
            break;
        }
    }

    // also resend requests that were not yet processed to astraea from other queue
    for (int i = 0; i < otherQueue.size(); i++) {
        if (otherQueue.front().timestamp.count() == 0) {
            // resend
            requestsToResend.push_back(otherQueue.front());
            otherQueue.pop();
        } else {
            // request stays in queue
            otherQueue.push(otherQueue.front());
            otherQueue.pop();
        }
    }

    // resend requests that remained in queue
    for (auto requestToResend : requestsToResend) {
        sendRequestToAstraea(requestToResend.ip_client, requestToResend.message, requestToResend.read_request);
    }

    // reset flags
    sendWriteRequestsToAstraea = sendWriteRequestsToAstraea = true;

    // return requests that were sent to astraea
    return requestsToHandle;
}

auto gateway_api = http_api(

        GET / _hello_world = []() {
            return D(_message = "hello world from the gateway api");
        },

        POST / _read_request_from_client * get_parameters(_request = string(), _ip_client = string()) = [](auto param) {
            log("gateway received read request " + string(param.request) + " from " + param.ip_client);

            // send request to db
            map <string, string> params = {{"request", param.request}};
            string response = restCallGet("database:3306/read_request", params);

            // send request to astraea (asynchronously)
            sendRequestToAstraea(param.ip_client, response, true);
        },

        POST / _write_request_from_client * get_parameters(_request, _ip_client) = [](auto param) {
            log("gateway received write request " + string(param.request) + " from " + param.ip_client);

            // send request to astraea (asynchronously)
            sendRequestToAstraea(param.ip_client, param.request, false);
        },

        POST / _release * get_parameters(_response = string(), _timestamp = double()) = [](auto param) {
            m.lock();
            log("received a response " + string(param.response) + " from astraea");

            // get all requests from queue that were sent to astraea
            list <request> requests = getRequestsSentToAstraea(param.timestamp);
            log("found " + to_string(requests.size()) + " requests to handle");

            // handle requests
            for (list<request>::iterator it = requests.begin(); it != requests.end(); ++it) {
                if (it->read_request) {
                    // return response to client
                    map <string, string> params = {{"response", string(it->message)}};
                    restCallPost(string(it->ip_client) + "/response", params);
                } else {
                    // send request to db
                    map <string, string> params = {{"request", string(it->message)}};
                    string response = restCallGet("database:3306/write_request", params);

                    // return response to client
                    params = {{"response", response}};
                    restCallPost(string(it->ip_client) + "/response", params);

                }
            }
            m.unlock();
        }
);

int main() {
    sl::mhd_json_serve(gateway_api, 4000, _linux_epoll, _nthreads = 20);
}
