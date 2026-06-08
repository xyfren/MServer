#include <iostream>
#include <boost/signals2.hpp>
#include <nlohmann/json.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

#include "chatserver/ChatServer.h"
#include "turnserver/TurnServer.h"

using namespace std;

using namespace boost::placeholders;

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
// сделать свой поток ошибок
int main() {
    try {
        shared_ptr<DBManager> dbManager = make_shared<DBManager>();
        
        dbManager->createTable();
        // Добавляем тестового пользователя
        ordered_json param;
        param["un"] = "admin";
        param["pw"] = "admin";
        dbManager->insert("INSERT INTO users (username,password) VALUES (:un,:pw)", param);
        param["un"] = "misha";
        param["pw"] = "12345";
        dbManager->insert("INSERT INTO users (username,password) VALUES (:un,:pw)", param);
        cout << dbManager->getUsers().dump() << endl;
        
        ChatServer chatServer(dbManager);
        TurnServer turnServer(dbManager);

        chatServer.m_sessionServer->createMediaSession.connect(boost::bind(&TurnServer::createMediaSession, &turnServer,_1,_2));
        chatServer.m_sessionServer->deleteMediaSession.connect(boost::bind(&TurnServer::deleteMediaSession, &turnServer,_1));
        chatServer.m_sessionServer->addCaller.connect(boost::bind(&TurnServer::addCaller, &turnServer,_1,_2,_3));
        chatServer.m_sessionServer->addCallee.connect(boost::bind(&TurnServer::addCallee, &turnServer,_1,_2,_3));
        chatServer.m_sessionServer->startMediaSession.connect(boost::bind(&TurnServer::startMediaSession, &turnServer,_1));
        chatServer.m_sessionServer->getSsrc.connect(boost::bind(&TurnServer::getSsrc, &turnServer));
        chatServer.m_sessionServer->getMediaSessionId.connect(boost::bind(&TurnServer::getMediaSessionId, &turnServer));
        chatServer.m_sessionServer->getMediaSession.connect(boost::bind(&TurnServer::getMediaSession,&turnServer,_1));

        turnServer.sessionStarted.connect(boost::bind(&SSPServer::onSessionStarted,chatServer.m_sessionServer.get(),_1));
        
        chatServer.run(12345);
        turnServer.run(12346,12347);

    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << endl;
        return 1;
    }
}