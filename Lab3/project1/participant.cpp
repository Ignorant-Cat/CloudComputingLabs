//
// Created by wenkai on 2021/6/16.
//

#include "participant.h"
#include "headSet.h"
#include "participant.h"

int Participant::keepAlive()
{
    std::cout << getTime() << "CONNECTING..." << std::endl;
    _net.close();

    // try to connect with the coordinator
    while (_net.reaccept())
    {
        // sleep(1);
    }

    std::cout << getTime() << "CONNECTED..." << std::endl;


    return KV_OK;
}

int Participant::Working()
{
    std::cout << getTime() << "WORKING..." << std::endl;
    int rc = KV_OK;
    std::string content;
    std::string rmsg;
    std::vector<std::string> comds;

    // if network close will return error
    rc = _net.recv(content);
    if(rc) goto done;

    // std::cout << content << std::endl;

    // parse and send
    rmsg = pWorker(content);
    _net.send(rmsg);

    done:
    return rc;
}

std::string Participant::pWorker(std::string task)
{
    std::string rc = de_en.getErrorMessage();
    txid tid;
    // parser the task
    Log l(task);

    switch(l.ID)
    {
        case SUPER_TXID:
            return eventParser(l.event);
            break;
        case RECOVERY_TXID:
            // recovery request, the p should send the max txid
            return std::to_string(_TXID - 1);
            break;
        case ASK_DATA_TXID:
            tid = strtol(l.event.c_str(), nullptr, 10);
            if(tid >= _TXID) {
                return "";
            } else {
                return _lg.getLogByTXID(tid).logToStr();
            }
            break;
        default:
            // std::cout << "info: " << l.ID << " " << l.state << std::endl;
            if(l.ID == _TXID && l.state == LOG_PRE) {
                return "PRE";
            } else if(l.ID == _TXID && l.state == LOG_COMMIT) {
                rc = eventParser(l.event);
                if(rc != de_en.getErrorMessage()) {
                    _lg.writeLog(l);
                    _TXID++;
                }
                return rc;
            }
            break;
    }


    return rc;
}

std::string Participant::eventParser(std::string event)
{
    std::string rstr = de_en.getErrorMessage();
    int delnumber = 0;

    std::vector<std::string> comds;
    // parse and send
    de_en.de_encodeRESPArry((char *)event.c_str(), comds);

    if(comds.size() < 1) {
        rstr = de_en.getErrorMessage();
        goto done;
    }

    switch(comds[0][0]) {
        case 'D': // DEL CS06142 CS162
            comds.erase(comds.begin());
            delnumber = _kvdb.kvDELArry(comds);
            rstr = de_en.getIntegerMessage(delnumber);
            break;

        case 'S': // SET key value
            if(comds.size() != 3) {
                rstr = de_en.getErrorMessage();
            } else {
                _kvdb.kvSET(comds[1], comds[2]);
                rstr = de_en.getSuccessMessage();
            }
            break;
        case 'G': // GET key
            std::cout << "GET " << comds.size() << std::endl;
            if(comds.size() != 2) {
                rstr = de_en.getErrorMessage();
            } else {
                rstr = de_en.getRESPArry( _kvdb.kvGET(comds[1]) );
            }
            break;
        case 'P': // P
            rstr = "PRE";
            break;
        default:
            rstr = de_en.getErrorMessage();
            break;

    }

    done:
    return rstr;
}

void Participant::Init(NodeInfo info)
{
    _TXID   = TXID_START;
    _net    = Network(info.port);
}

int Participant::Launch()
{
    keepAlive();
    while(1) {
        if(Working()) {
            keepAlive();
        }
    }
    return KV_OK;
}