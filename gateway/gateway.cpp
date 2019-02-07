#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include "symbols_gateway.hh"
#include <stdio.h>
#include <mysql/mysql.h>
#include "http_request_helper.h"
#include <list>
#include <iterator>
#include <mutex>
#include <queue>
#include <stdlib.h>
#include <sstream>


using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace
using namespace std;


static const string DATABASE_HOST = "database";
static const string DATABASE_PORT = "3306";
static const string DATABASE_PASSWORD = "TEST_PASSWORD";
static const string DATABASE_USER = "root";
static const string DATABASE_NAME = "target";
//sql::Driver *driver = get_driver_instance();
//sql::Connection *conn;
//sql::Statement *stmt;
//ResultSet *res;

queue <request> myQueue;
mutex m;


// for debugging reasons
void printQueue() {
    queue <request> duplicate = myQueue;
    while (!duplicate.empty()) {
        cout << &duplicate.front() << endl;
        cout << duplicate.front().toString() << endl;
        duplicate.pop();
    }
}

string flushToTarget(string query) {
    string mysql = "mysql -h'" + DATABASE_HOST + "' -P'" + DATABASE_PORT + "' -u'" + DATABASE_USER + "' -p'" + DATABASE_PASSWORD + "' -e'" + DATABASE_NAME + "'";
    string response;    
    FILE *fp;
    char line[PAGE_SIZE];
    fp = popen(mysql.c_str(), "w");
    while (fgets(line, PATH_MAX, fp)){
        response += (string(line) + "\n");
        
    }
      return response;
}
 //   string database_url = "tcp://" + DATABASE_HOST + ":" + DATABASE_PORT;
 //   conn = driver->connect(database_url, DATABASE_USER, DATABASE_PASSWORD);
 //   conn->setSchema(DATABASE_NAME);
 //   stmt = conn->createStatement();
 //   res = stmt->executeQuery(query);
 //   string response = "";
 //   while (res->next()) {
 //       response += res->getString(1);
 //       response += res->getString(2);
 //       response += '\n';
 //   }
 //   delete res;
 //   delete stmt;
 //   delete conn;
 //   return response;
//}

// get all requests from queue that were sent to astraea
list <request> getRequestsSentToAstraea() {
    list <request> requests;
    if (myQueue.empty()) 
        log("myQueue is empty :(");
    queue <request> duplicate = myQueue;
    int countPops = 0;
    if (duplicate.empty())
	    return requests;

    while (!duplicate.empty() && (duplicate.front().is_sent_to_astraea || duplicate.front().timestamp < time(nullptr) - 5)) {
        // delete outdated requests
        if (duplicate.front().timestamp < time(nullptr) - 5) {
            log("queued request is outdated: " + duplicate.front().message);
            duplicate.pop();
            countPops++;
        }

        // store valid requests in list
        requests.push_back(duplicate.front());
        duplicate.pop();
        countPops++;

        // break if queue is empty
        if (duplicate.empty()) {
            log("queue is empty!");
            break;
        }
    }

    // pop outdated and responded requests from actual list
    for (int i = 0; i < countPops; i++) {
        if (!myQueue.empty())
            myQueue.pop();
    }

    return requests;
}

auto gateway_api = http_api(

        GET / _hello_world = []() {
            return D(_message = "hello world from the gateway api");
        },



        POST / _read_request_from_client * get_parameters(_request = string(), _ip_client = string()) = [](auto param) {

            log("gateway received read request " + string(param.request) + " from " + param.ip_client);
            cout << "gateway received read request " + string(param.request) + " from " + param.ip_client << endl;
            // send request to db
            map<string, string> params = {{"request", param.request}};
            string response;
            //db(param.request) >> response;
            response = flushToTarget(param.request);
            log("response from mysql: " + response);
            // send request to astraea (asynchronously)
//           params = {{"form", createForm(string(param.ip_client), string(param.request))}};
            params = {{"ip_client", param.ip_client}, {"request", param.request}};           
            restCallPostJsonAsync("astraea:5000/write", params);
            // push request to queue
            struct request request = {response, param.ip_client, time(nullptr), true, true};
            myQueue.push(request);
        },

        POST / _write_request_from_client * get_parameters(_request = string(), _ip_client = string()) = [](auto param) {
            log("gateway received write request " + string(param.request) + " from " + param.ip_client);
            // send request to astraea
            //map<string, string> params = {{"form", createForm(string(param.ip_client), string(param.request))}};
            map <string, string> params = {{"ip_client", param.ip_client}, {"request", param.request}};
            restCallPostJson("astraea:5000/write", params);
            // push request to queue
            struct request request = {param.request, param.ip_client, time(nullptr), true, false};
            myQueue.push(request);
        },


        POST / _release * get_parameters(_response) = [](auto param){
            m.lock();
            log("received a response " + string(param.response) + " from astraea");

            // get all requests from queue that were sent to astraea
            list <request> requests = getRequestsSentToAstraea();
            log("found " + to_string(requests.size()) + " requests to handle");

            // handle requests
            for (list<request>::iterator it = requests.begin(); it != requests.end(); ++it) {
                if (it->read_request) {
                    // return response to client
                    map<string, string> params = {{"response", string(it->message)}};
                    restCallPost(string(it->ip_client) + "/response", params);
                } else {
                    // send request to db
                    string response;
                    
                    //db(it->message) >> response;
//                    response = flushToTarget(param.request);
                    // return response to client
                    map<string, string> params = {{"response", response}};
                    restCallPost(string(it->ip_client) + "/response", params);

                }
            }
            m.unlock();
            return D(_message = "OK\n");
        }
);

int main() {

    sl::mhd_json_serve(gateway_api, 4000, _linux_epoll, _nthreads = 20);
}
