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

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>


using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace
using namespace std;



int throughput = 0;
vector<pair<unsigned int, int>> second_throughput;



static const string DATABASE_HOST = "target-db";
static const string DATABASE_PORT = "3306";
static const string DATABASE_PASSWORD = "TEST_PASSWORD";
static const string DATABASE_USER = "root";
static const string DATABASE_NAME = "target";
static const int INT_BUF = 12;
static const int TIMEOUT = 10;
bool timeout_expired = false;



queue <request> myQueue;
mutex m;


void record_requests() {
     ofstream myfile;
     myfile.open("./data/throughput.csv");
     char str_second[INT_BUF];
     char str_requests_n[INT_BUF];
     if (second_throughput.size() > 0) {
         for (int i = 0; i < second_throughput.size(); i++){
             log("recording requests!");
             unsigned int second = second_throughput[i].first;
             int requests_n  = second_throughput[i].second;
             sprintf(str_second, "%d", second);
             sprintf(str_requests_n, "%d", requests_n);
             log("second being written to file: " + string(str_second));
             myfile << str_second;
             myfile << ",";
             myfile << str_requests_n;
             myfile << "\n";
             memset(str_second, 0, sizeof(str_second));
             memset(str_requests_n, 0, sizeof(str_requests_n));
         }
     }
     myfile.close();
}


string flushToTarget(string query) {
    string mysql = "mysql -h'" + DATABASE_HOST + "' -P'" + DATABASE_PORT + "' -u'" + DATABASE_USER + "' -p'" + DATABASE_PASSWORD + "' -D'" + DATABASE_NAME + "' -e'" + query + "'"; 
  //  printf("mysql call: %s\n", mysql.c_str());
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

void check_timeout(unsigned int last_second, unsigned int current_second) {
    if ((last_second + TIMEOUT) < current_second) {
        record_requests();
        
    }
}

void record_time() {
    std::time_t current_second = std::time(0);
    if (second_throughput.size() == 0) {
        second_throughput.push_back(make_pair(current_second, 0));
    }
    unsigned int last_second = second_throughput.back().first;
    check_timeout(last_second, current_second);
    if(current_second != last_second){
        pair <unsigned int, int> new_record = make_pair(current_second, throughput);
        second_throughput.push_back(new_record);
        char char_throughput[INT_BUF];
        sprintf(char_throughput, "%d", throughput);
        string str_throughput(char_throughput);
        throughput = 0;
    }
    throughput++;
}



auto gateway_api = http_api(

        GET / _hello_world = []() {
            return D(_message = "hello world from the gateway api");
        },



        POST / _read_request_from_client * get_parameters(_request = string(), _ip_client = string()) = [](auto param) {

            // send request to db
            map<string, string> params = {{"request", param.request}};
            string response;
            //db(param.request) >> response;
            response = flushToTarget(param.request);


            // send request to astraea (asynchronously)
//           params = {{"form", createForm(string(param.ip_client), string(param.request))}};
            params = {{"ip_client", param.ip_client}, {"query", param.request}};          
             
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
            if(timeout_expired == true) {
                record_requests();
            }
            log("[INFO] received a response " + string(param.response) + " from astraea");
            // get all requests from queue that were sent to astraea
            list <request> requests = getRequestsSentToAstraea();

            // handle requests
            for (list<request>::iterator it = requests.begin(); it != requests.end(); ++it) {
                
                if (it->read_request) {
                    // return response to client
//                    map<string, string> params = {{"response", string(it->message)}};
                    map<string, string> params = {{"response", "OK"}};
                    restCallPost(string(it->ip_client) + "/response", params);
                    record_time();
                } else {
                    // send request to db
                    string response;
                    
                    //db(it->message) >> response;
//                    response = flushToTarget(param.request);
                    // return response to client
                    map<string, string> params = {{"response", response}};
                    restCallPost(string(it->ip_client) + "/response", params);
                    record_time();             

                }
            }
            m.unlock();
            return D(_message = "OK\n");
        }
);




int main() {
    

    sl::mhd_json_serve(gateway_api, 4000, _linux_epoll, _nthreads = 20);
}
