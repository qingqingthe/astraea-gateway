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
#include <future> 

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <thread>


using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace
using namespace std;



int throughput = 0;
vector<pair<unsigned int, int>> second_throughput;



static const string DATABASE_HOST = "target-db";
static const int DATABASE_PORT = 3306;
static const string DATABASE_PASSWORD = "TEST_PASSWORD";
static const string DATABASE_USER = "root";
static const string DATABASE_NAME = "target";
static const int INT_BUF = 12;
static const int TIMEOUT = 8;
static const int MAX_REQ = 90000;
bool timeout_expired = false;
bool not_done = true;
int n_requests = 0;
int release = 0;


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


string flushToTarget(string query, string ip_client) {
//    string mysql = "mysql -h'" + DATABASE_HOST + "' -P'" + DATABASE_PORT + "' -u'" + DATABASE_USER + "' -p'" + DATABASE_PASSWORD + "' -D'" + DATABASE_NAME + "' -e'" + query + "'"; 
  //  printf("mysql call: %s\n", mysql.c_str());
//    string response;    
//    FILE *fp;
//    char line[PAGE_SIZE];
//    fp = popen(mysql.c_str(), "w");
//    while (fgets(line, PATH_MAX, fp)){
//        response += (string(line) + "\n"); 
//    }
//      return response;
//}
//    try {
//        sql::Driver *driver;
//        sql::Connection *conn;
//        sql::Statement *stmt;
//        sql::ResultSet *res;
//        driver = get_driver_instance();
//        string database_url = "tcp://" + DATABASE_HOST + ":" + DATABASE_PORT;
//        conn = driver->connect(database_url, DATABASE_USER, DATABASE_PASSWORD);
//        conn->setSchema(DATABASE_NAME);
//        stmt = conn->createStatement();
//        res = stmt->executeQuery(query);
//        string response = "";
//        while (res->next()) {
//            response += res->getString(1);
//            response += res->getString(2);
//            response += '\n';
//        }
//        delete res;
//        delete stmt;
//        delete conn;
//        return response;
//    } catch (sql::SQLException &e) {
//         cout << "# ERR: SQLException in " << __FILE__;
//         cout << "# ERR: " << e.what();
//         cout << " (MySQL error code: " << e.getErrorCode();
//         cout << ", SQLState: " << e.getSQLState() << " )" << endl;
//    }
//    mysqlx_session_t  *sess;
//    mysqlx_stmt_t     *crud;
//    mysqlx_result_t   *res;
//    mysqlx_row_t      *row;
//    mysqlx_schema_t   *db;
//    mysqlx_table_t    *table;
//    char str_row[256];
//    string response;
//    size_t buf_len = sizeof(response);
//    
//  
//    string url = "mysqlx://" + DATABASE_USER + ":" +  DATABASE_PASSWORD + "\\" + "@" + DATABASE_HOST + ":" + DATABASE_PORT + "/";
////    string url = "mysqlx%3A%2F%2Froot%3ATEST_PASSWORD%40target-db%3A33062";
//
    char char_database_name[DATABASE_NAME.length() + 1];
    strcpy(char_database_name, (const char *) DATABASE_NAME.c_str());
    const char * const_char_database_name = (const char *) char_database_name;
//
    char char_pass[DATABASE_PASSWORD.length() + 1];
    strcpy(char_pass, DATABASE_PASSWORD.c_str());
    const char * const_char_pass = (const char *) char_pass;
//
    char char_host[DATABASE_HOST.length() + 1];
    strcpy(char_host, DATABASE_HOST.c_str());
    const char * const_char_host = (const char *) char_host;
//    
//
    char char_user[DATABASE_USER.length() + 1];
    strcpy(char_user, DATABASE_USER.c_str());
    const char * const_char_user = (const char *) char_user;
//
//    char char_url[url.length() + 1];
//    strcpy(char_url, (const char *) url.c_str());
//    const char * const_char_url = (const char*) char_url;
//
//    char conn_error[MYSQLX_MAX_ERROR_LEN];
//    int conn_err_code;
////    sess = mysqlx_get_session(const_char_host, DATABASE_PORT, const_char_user, const_char_pass, const_char_database_name, conn_error, &conn_err_code);
//    sess = mysqlx_get_session_from_url(const_char_url, conn_error, &conn_err_code);
//    if (!sess)
//    {
//     printf("\nError! %s. Error Code: %d. %s.\n", conn_error, conn_err_code, url.c_str());
//     return conn_error;
//    }
//    db = mysqlx_get_schema(sess, const_char_database_name, 1);
//    printf("%s", "1.3 - mysql schema\n");
    char char_query[query.length() + 1];
    strcpy(char_query, (const char *) query.c_str());
    const char * const_char_query = (const char *) char_query;
//    res = mysqlx_sql(sess, const_char_query, MYSQLX_NULL_TERMINATED);
//    printf("%s", "1.4 - mysql sql\n");
//     
//    while (( row = mysqlx_row_fetch_one(res) )) {
//        mysqlx_get_bytes(row, 0, 0, str_row, &buf_len);
//        response += str_row;
//    }
//    mysqlx_session_close(sess);

    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL){
        printf("Null conn: %s\n", mysql_error(conn));
        exit(1);
    }
    if(mysql_real_connect(conn, "target-db", "root", "TEST_PASSWORD", "target", 3306, NULL, 0) == NULL) {
        printf("Connection failed: %s - %s %s %s %s %d \n", mysql_error(conn), const_char_host, const_char_user, const_char_pass, const_char_database_name, DATABASE_PORT);
        exit(1);
    }
    if(mysql_query(conn, const_char_query)) {
        printf("query failed: %s\n", mysql_error(conn));
        exit(1);
    }
    MYSQL_RES *result = mysql_store_result(conn);
    int num_fields = mysql_num_fields(result); 
    MYSQL_ROW row;
    string response;
    while((row = mysql_fetch_row(result))) {
       for(int i = 0; i < num_fields; i++) {
           response += i;
       }
    }
    mysql_free_result(result);
    mysql_close(conn);
    struct request request = {response, ip_client, time(nullptr), true, true};
    myQueue.push(request);
   // free((char *) const_char_query);
   // free((char *) const_char_database_name);
   // free((char *) const_char_pass);
   // free((char *) const_char_user);
   // free((char *) const_char_host);
    return response;
}

// get all requests from queue that were sent to astraea
list <request> getRequestsSentToAstraea() {
    list <request> requests;
    int size = myQueue.size();
    stringstream ss;
    ss << size;
    log("allocating queue with size " + ss.str());
    queue <request> duplicate = myQueue;
    int countPops = 0;
    if (duplicate.empty())
	    return requests;
    std::thread::id this_id = std::this_thread::get_id();
    stringstream st;
    st << this_id;
    log("parsing through queue - id = " + st.str());
    
    while (!duplicate.empty() && (duplicate.front().is_sent_to_astraea || duplicate.front().timestamp < time(nullptr) - 5)) {
    log("in the while : id = " + st.str());
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
    stringstream sq;
    sq << countPops;
    log("right before emptying my queue - removing " + sq.str());
    for (int i = 0; i < countPops; i++) {
        if (!myQueue.empty()){
            stringstream se;
            se << myQueue.size();
            log("right before poping - queue size: " + se.str() + " " + sq.str());
            myQueue.pop();
        }
    }
    
    log("right before return");
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
            char s_release[INT_BUF];
            sprintf(s_release, "%d", release);
            log("releases: " + string(s_release));
            n_requests++;
            if(n_requests % 50 == 0) {
                stringstream ss;
                ss << n_requests;
                log("\n" + ss.str() + " read...\n" );
            }

            // send request to db
            map<string, string> params = {{"request", param.request}};
            
            
            //db(param.request) >> response;
            std::future<string> future_response = std::async(flushToTarget, param.request, param.ip_client);
            
            // send request to astraea (asynchronously)
//           params = {{"form", createForm(string(param.ip_client), string(param.request))}};
            params = {{"ip_client", param.ip_client}, {"query", param.request}}; 
            restCallPostJsonAsync("astraea:5000/write", params);
            // push request to queue
            if(n_requests > MAX_REQ && not_done) {
                not_done = false;
                record_requests();
            }
          
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

        POST / _release = [](auto param){
            release++;
            log("Gone through the lock");
            // get all requests from queue that were sent to astraea
            list <request> requests = getRequestsSentToAstraea();
            log("Requests acquired");

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
            log("release done");
            return D(_message = "OK\n");
        }
);


int main() {
    sl::mhd_json_serve(gateway_api, 4000, _linux_epoll, _nthread=7);
}
