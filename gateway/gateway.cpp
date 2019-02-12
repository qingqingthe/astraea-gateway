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
map<long, bool> astraeaFlags;
long astraeaId = 1;
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
mutex m2;

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

    // block so that not two threads can set the flags in parallel
    m2.lock();

    // get current timestamp
    milliseconds now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    // send request to astraea
    map <string, string> params = {{"form", createForm(now.count() % 10000, string(ip_client), string(request))}};
    string response = restCallGet("astraea:5000/write", params);

    // if request is successfully sent to astraea, timestamp is now, else 0
    milliseconds timestamp(0);

    // evaluate response from astraea
    if (response == "accepted") {
        log("accepted: " + request + ", batch: " + to_string(now.count() % 10000));
        // block sending (!readRequest) requests to astraea
        sendReadRequestsToAstraea = readRequest;
        sendWriteRequestsToAstraea = !readRequest;

        // new astraea batch is created
        /**
         * the approach to use milliseconds as ids assumes that one process
         * (send request to astraea, receive response and get all request that were sent before from queue)
         * doesn't take longer than 10 seconds -> adjustable
         */
        astraeaId = now.count() % 10000;
        astraeaFlags[astraeaId] = readRequest;
        timestamp = now;
    } else if (response == "blocked" &&
               ((sendReadRequestsToAstraea && readRequest) || (sendWriteRequestsToAstraea && !readRequest))) {
        // astraea is generating mc value for this kind of requests
        log("blocked: " + request + ", batch: " + to_string(astraeaId));
        timestamp = now;
    }

    // push request to queue
    struct request r = {astraeaId, request, ip_client, timestamp, readRequest};
    if (readRequest) {
        readQueue.push(r);
        if (timestamp.count() == 0)
            log(string(readRequest ? "read" : "write") + " request was not sent: " + request);
    } else {
        writeQueue.push(r);
        if (timestamp.count() == 0)
            log(string(readRequest ? "read" : "write") + " request was not sent: " + request);
    }
    m2.unlock();
}

void resendRequestsAsync(list <request> requests) {
    for (auto request : requests) {
        sendRequestToAstraea(request.ip_client, request.message, request.read_request);
    }
}

// get all requests from queue that were sent to astraea
list <request> getRequestsSentToAstraea(double response_ms, long response_id) {
    // decide with id and flags map which queue to iterate for requests
    map<long, bool>::iterator iter = astraeaFlags.find(response_id);
    if (iter == astraeaFlags.end())
        return list<request>();
    bool readRequest = iter->second;

    queue <request> &queueInQuestion = readRequest ? readQueue : writeQueue;
    queue <request> &otherQueue = readRequest ? writeQueue : readQueue;

    list <request> requestsToHandle;
    list <request> requestsToResend;

    // iterate through queue with kind of requests processed by astraea
    int queueSize = queueInQuestion.size();
    log("queue size: " + to_string(queueSize));
    for (int i = 0; i < queueSize; i++) {
        if (queueInQuestion.front().timestamp.count() == 0) {
            // request was not sent to astraea -> resend
            log("never: " + to_string(queueInQuestion.front().timestamp.count()) + ", " +
                queueInQuestion.front().message);
            request r = queueInQuestion.front();
            requestsToResend.push_back(queueInQuestion.front());
            queueInQuestion.pop();
        } else if (queueInQuestion.front().astraeaId == response_id) {
            // request was in same batch
            log("right batch: " + to_string(queueInQuestion.front().astraeaId) + " " + queueInQuestion.front().message);
            requestsToHandle.push_back(queueInQuestion.front());
            queueInQuestion.pop();
        } else {
            // request is in wrong batch -> put at the end of the queue
            log("wrong batch: " + to_string(queueInQuestion.front().astraeaId) + " " + queueInQuestion.front().message);
            queueInQuestion.push(queueInQuestion.front());
            queueInQuestion.pop();
        }
    }

    // also resend requests that were not yet processed to astraea from other queue
    queueSize = otherQueue.size();
    for (int i = 0; i < queueSize; i++) {
        if (otherQueue.front().timestamp.count() == 0) {
            // resend
            requestsToResend.push_back(otherQueue.front());
            log("other kind of request was not sent: " + otherQueue.front().message);
            otherQueue.pop();
        } else {
            // request stays in queue
            otherQueue.push(otherQueue.front());
            otherQueue.pop();
        }
    }

    // resend requests that remained in queue
    // async because it might take a while
    thread t0(resendRequestsAsync, requestsToResend);
    t0.detach();

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

        POST / _release * get_parameters(_response = string(), _timestamp = double(), _id = long()) = [](auto param) {
            log("received a response " + string(param.response) + " from astraea");

            m.lock();
            // get all requests from queue that were sent to astraea
            list <request> requests = getRequestsSentToAstraea(param.timestamp, param.id);
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
    sl::mhd_json_serve(gateway_api, 4000);
}
