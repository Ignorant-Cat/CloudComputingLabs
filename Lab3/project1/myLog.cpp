//
// Created by wenkai on 2021/6/16.
//

#include "myLog.h"
#include "headSet.h"

#include "util.h"

void Log::strToLog(std::string str)
{
    size_t begin = str.find("TXID: ");
    size_t end = str.find("\r\n");
    assert(begin != std::string::npos && end != std::string::npos);
    std::string sID = str.substr(begin + 6, end - begin - 6);
    ID = strtol(sID.c_str(), nullptr, 10);

    str = str.substr(end + 2);
    begin = str.find("S: ");
    end = str.find("\r\n");
    assert(begin != std::string::npos && end != std::string::npos);
    std::string sState = str.substr(begin + 3, end - begin - 3);
    state = (logState)strtol(sState.c_str(), nullptr, 10);

    str = str.substr(end + 4);
    event = str;
}

std::string Log::logToStr()
{
    std::string rc = "";
    rc += "TXID: ";
    rc += std::to_string(ID);
    rc += "\r\n";
    rc += "S: ";
    rc += std::to_string(state);
    rc += "\r\n\r\n";
    rc += event;
    return rc;
}


void Mylog::writeLog(txid ID, logState state, std::string event)
{
    _l[ID] = Log(ID, state, event);
}

void Mylog::changeLogState(txid ID, logState newState)
{
    assert(_l.count(ID) == 1);
    _l[ID].changeState(newState);
}

Log Mylog::getLogByTXID(txid ID)
{
    assert(_l.count(ID) == 1);
    return _l[ID];
}

void Mylog::writeLog(std::string lg)
{
    Log l;
    l.strToLog(lg);
    _l[l.ID] = l;
}

void Mylog::writeLog(Log lg)
{
    _l[lg.ID] = lg;
}
