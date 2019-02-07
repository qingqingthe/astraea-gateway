#!/usr/bin/python
#import asyncio
import json
import logging
import os
import requests
import threading
import time
from amcs.MonoCounter import MonoCounter
from broker.MariaDBBroker import MariaDBBroker
from ctypes import c_char_p
from encrypt.CryptoHasher import CryptoHasher 
from flask import Flask
from flask import request
from multiprocessing import Process, Pipe, Manager
from spool.MariaDBSpool import MariaDBSpool
from util import conf_parser



manager = Manager()
app = Flask(__name__)
#read maria db connection info from a yaml file (mariadb.yml)
target_pars = conf_parser.get_mariadb_conf("/target-db.yml")
log_pars = conf_parser.get_mariadb_conf("/log-db.yml")
gateway_pars = conf_parser.get_gateway_conf("/gateway.yml")
amcs_pars = conf_parser.get_amcs_conf("/amcs.yml")
astraea_pars = conf_parser.get_astraea_conf("/astraea.yml")
#c-like array for process communication
batch = manager.list()
assert type(target_pars) == tuple
assert type(log_pars) == tuple
AUTH_SIZE = 16
hasher = CryptoHasher(16)
spool = MariaDBSpool(log_pars)
monoCounter = MonoCounter(amcs_pars)
is_mono_created = False
#broker = MariaDBBroker(target_pars)
read_queue = []
write_queue = 




@app.route('/')
def hello_audit():
    return 'Hello audit\n'

@app.route('/read_request_from_client/<string:ip_client>/<string:request>', methods=['POST'])
def read_request_from_client(ip_client, request):
    app.logger.warning("gateway received request from %s" % ip_client)
    spool.runQuery(request)
    host, port, endpoint = astraea_pars
    url = "http://%s:%i%s" % (host, port, endpoint)
    data = {"ip_client": ip_client, "request": resquest}
    r = requests.post(url, json=data ,allow_redirects=True)
    app.logger.warning("response is %s" % r.text)
    read_queue.append((ip_client, request))
    return "OK", 200

@app.route('/write_request_from_client/<string:ip_client>/<string:request>', methods=['POST'])
def write_request_fromClient(ip_client, request):
    app.logger.warning("gateway received request from %s" % ip_client)
    host, port, endpoint = astraea_pars
    url = "http://%s:%i%s" % (host, port, endpoint)
    data = {"ip_client": ip_client, "request": resquest}
    r = requests.post(url, json=data ,allow_redirects=True)
    app.logger.warning("response is %s" % r.text)
    read_queue.append((ip_client, request))
    spool.runQuery(request)
    return "OK", 200

#def send_data(spool, 
    

def clear():
    global batch
    batch = manager.list()
    p1.join() 
    app.logger.warning("batch after reassignment %s" % str(batch))
    return "OK - Batch cleared\n", 200


def flush(batch, spool):
    #it would come in handy to check the time before and after this.
    number = int(monoCounter.inc())
    if number == -1:
        app.logger.error("inc'd mc is invalid: %d" % number)
        return False
    logging.warning("logging number is %d" % number)
    c_hash = hasher.sign(batch, number)
    success_hash = spool.flushBatchHash(spool, c_hash, number)
    success_batch = [spool.flushNumberedQuery(spool, query, number) for query in batch] 
    return c_hash


     
def confirm_commit(gateway_pars):
    host, port, endpoint = gateway_pars[0], str(gateway_pars[1]), gateway_pars[2]
    url = "http://" +  host + ":" + port + endpoint
    response = {"response": "OK"}
    r = requests.post(url, params=response ,allow_redirects=True)
    app.logger.warning("response is %s" % r.text)
    return r.text 

def run(batch, gateway_pars, spool):
    app.logger.warning("[INFO] acquiring inc'd value...")
    with open(os.getcwd() + "/flush.log", 'a+') as flush_log:
        flush_log.write("starting commit")
    while True:
        if len(batch) > 0:
            is_commit_finished = flush(batch, spool)
            if is_commit_finished: 
                while len(batch) > 0:
                    batch.pop()
                ack = confirm_commit(gateway_pars)
                logging.warning("[INFO] batch value after clear is %s" % str(batch))
        else:
            logging.warning("[INFO] batch was empty: %s" % str(batch))        
            while len(batch) == 0:
                time.sleep(1)
                logging.warning("[INFO] waiting for new batch")
                  


if __name__ == '__main__':
#    monoCounter.create() 
#    parent_conn, child_conn = Pipe()
#    p1 = Process(target=run, args=(child_conn, batch, gateway_host, spool))
#    app.logger.warning("Starting p1")
#    p1.start()
    app.run(debug=True)
    print "[INFO] Joining processes..."
    p1.join()
     
