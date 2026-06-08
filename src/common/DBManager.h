#ifndef __DBMANAGER_H__
#define __DBMANAGER_H__

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/Transaction.h>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

class DBManager {
public:
    DBManager(const string& dbPath = "db/test.db") {
        try {
            db = make_unique<SQLite::Database>(dbPath, 
                  SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            cout << "DB created/opened successfully: " << dbPath << endl;
        }
        catch (const SQLite::Exception& e) {
            cerr << "SQLite error: " << e.what() << endl;
            throw;
        }
    }

    // Автоматическое управление транзакциями с помощью RAII
    void createTable() {
        try {
            db->exec(R"(
                DROP TABLE IF EXISTS history; 
                CREATE TABLE history(
                    id INTEGER PRIMARY KEY AUTOINCREMENT, 
                    message TEXT,
                    sender TEXT,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                );
            )");
            db->exec(R"(
                DROP TABLE IF EXISTS users; 
                CREATE TABLE users(
                    id INTEGER PRIMARY KEY AUTOINCREMENT, 
                    username TEXT,
                    password TEXT,
                    online BOOLEAN DEFAULT false
                );
            )");
            cout << "Table created successfully" << endl;
        }
        catch (const SQLite::Exception& e) {
            cerr << "SQLite error in createTable: " << e.what() << endl;
            throw runtime_error(string("Table creation failed: ") + e.what());
        }
    }

    // Упрощенный метод для запросов с автоматическим преобразованием в JSON
    ordered_json get(const string& sql) {
        ordered_json data = ordered_json::array();
        
        try {
            SQLite::Statement query(*db, sql);
            const int column_count = query.getColumnCount(); 
            
            while (query.executeStep()) {
                ordered_json row;
                for (int i = 0; i < column_count; i++) {
                    row[query.getColumnName(i)] = getColumnValue(query, i);
                }
                data.push_back(row);
            }
            
            return data;
        }
        catch (const SQLite::Exception& e) {
            cerr << "SQLite error in get: " << e.what() << endl;
            throw runtime_error(string("Query execution failed: ") + e.what());
        }
    }

    ordered_json get(const string& sql, const ordered_json& params) {
        ordered_json data = ordered_json::array();
        
        try {
            SQLite::Statement query(*db, sql);
            bindParameters(query, params);
            const int column_count = query.getColumnCount(); 
            
            while (query.executeStep()) {
                ordered_json row;
                for (int i = 0; i < column_count; i++) {
                    row[query.getColumnName(i)] = getColumnValue(query, i);
                }
                data.push_back(row);
            }
            
            return data;
        }
        catch (const SQLite::Exception& e) {
            cerr << "SQLite error in get: " << e.what() << endl;
            throw runtime_error(string("Query execution failed: ") + e.what());
        }
    }

    // Перегруженные методы для разных сценариев вставки
    void insert(const string& sql, const ordered_json& params = ordered_json::object()) {
        try {
            SQLite::Statement query(*db, sql);
            bindParameters(query, params);
            query.exec();
        }
        catch (const SQLite::Exception& e) {
            cerr << "SQLite error in insert: " << e.what() << endl;
            throw runtime_error(string("Insert failed: ") + e.what());
        }
    }

    int update(const string& sql, const ordered_json& params = ordered_json::object()) {
        try {
            SQLite::Statement query(*db, sql);
            bindParameters(query, params);
            query.exec();
            
            return getChanges();
        }
        catch (const SQLite::Exception& e) {
            cerr << "SQLite error in update: " << e.what() << endl;
            throw runtime_error(string("Update failed: ") + e.what());
        }
    }

    // Пакетная вставка с использованием транзакций RAII
    void insertBatch(const string& sql, const ordered_json& rows) {
        if (!rows.is_array()) {
            throw invalid_argument("insertBatch requires JSON array");
        }

        try {
            SQLite::Statement query(*db, sql);
            SQLite::Transaction transaction(*db);

            for (const auto& row : rows) {
                query.reset();
                bindParameters(query, row);
                query.exec();
            }
            
            transaction.commit();
        }
        catch (const SQLite::Exception& e) {
            cerr << "SQLite error in insertBatch: " << e.what() << endl;
            throw runtime_error(string("Batch insert failed: ") + e.what());
        }
    }

    // Упрощенный метод для вставки одной записи с именованными параметрами
    ordered_json insertHistory(const string& message,const string& username) {
        ordered_json param;
        param["msg"] = message;
        param["un"] = username;
        insert("INSERT INTO history (message,sender) VALUES (:msg,:un)",param);
        
        int64_t lastId = getLastInsertId();
        
        
        ordered_json idParam;
        idParam["id"] = lastId;
        ordered_json result = get("SELECT * FROM history WHERE id = :id", idParam);
        
        if (!result.empty()) {
            return result[0]; 
        }
        
        return ordered_json::object();
    }

    // Получение всей истории с автоматическим преобразованием
    ordered_json getHistory() {
        return get("SELECT * FROM history ORDER BY created_at ASC");
    }

    ordered_json getUsers() {
        return get("SELECT * FROM users");
    }

    ordered_json getOnlineUsers() {
        return get("SELECT username, online FROM users");
    }

    bool isUserExists(const string & username){
        ordered_json params;
        params["un"] = username;
        ordered_json res = get("SELECT 1 FROM users WHERE username = :un",params);
        cout << res << endl;
        if (res.size() == 0){
            return false;
        }
        return true;
    }

    void registerUser(const string & username,const string & password){
        ordered_json param;
        param["un"] = username;
        param["pw"] = password;
        try {
            insert("INSERT INTO users (username,password) VALUES (:un,:pw)",param);
        }
        catch (const exception& ex) {
            cout << ex.what() << endl;
        }
    }

    bool verifyUser(const string & username,const string & password){
        ordered_json param;
        param["un"] = username;
        param["pw"] = password;
        ordered_json res = get("SELECT 1 FROM users WHERE username = :un AND password = :pw",param);
        cout << res << endl;
        if (res.size() == 0){
            return false;
        }
        return true;
    }

    void setOnline(const string & username,bool online){
        ordered_json param;
        param["on"] = online;
        param["un"] = username;

        update("UPDATE users SET online = :on WHERE username = :un", param);
    }

    ordered_json getData(int count, int offset, string table, vector<string> cols) {
        // Проверка на пустую таблицу
        if (table.empty()) {
            return ordered_json::array();
        }
        
        int total_count = getRowCount(table);
        
        // Если таблица пустая, возвращаем пустой массив
        if (total_count == 0) {
            return ordered_json::array();
        }
        
        // Если count = 0, возвращаем всех пользователей
        if (count == 0 && table == "users") {
            string columns_str;
            if (cols.empty()) {
                columns_str = "*"; 
            } else {
                for (size_t i = 0; i < cols.size(); ++i) {
                    columns_str += cols[i];
                    if (i < cols.size() - 1) {
                        columns_str += ", ";
                    }
                }
            }
            
            string sql = "SELECT " + columns_str + " FROM " + table + ";";
            cout << sql << endl;
            
            return get(sql);
        }
        
        // Обычная логика для других случаев
        if (offset >= total_count) {
            return ordered_json::array();
        }
        
        string columns_str;
        if (cols.empty()) {
            columns_str = "*"; 
        } else {
            for (size_t i = 0; i < cols.size(); ++i) {
                columns_str += cols[i];
                if (i < cols.size() - 1) {
                    columns_str += ", ";
                }
            }
        }

        // Корректируем count, если запрашивается больше, чем есть
        int actual_count = count;
        if (offset + count > total_count) {
            actual_count = total_count - offset;
        }
        
        int reverse_offset = max(0, total_count - offset - actual_count);
        
        string sql = "SELECT " + columns_str + " FROM " + table + 
                    " LIMIT " + to_string(actual_count) + 
                    " OFFSET " + to_string(reverse_offset) + ";";
        
        cout << sql << endl;

        return get(sql);
    }
    void close() {
        db.reset();
    }

    // Простые методы для выполнения запросов
    void execute(const string& sql) {
        try {
            db->exec(sql);
        }
        catch (const SQLite::Exception& e) {
            cerr << "SQLite error in execute: " << e.what() << endl;
            throw runtime_error(string("Execute failed: ") + e.what());
        }
    }

    int getRowCount(const string& tableName) {
        try {
            SQLite::Statement query(*db, "SELECT COUNT(*) FROM " + tableName);
            
            if (query.executeStep()) {
                return query.getColumn(0).getInt();
            }
            return 0;
            
        } catch (const exception& e) {
            cerr << "Ошибка при получении количества записей: " << e.what() << endl;
            return -1;
        }
    }   

    int64_t getLastInsertId() const {
        return db->getLastInsertRowid();
    }

    int getChanges() const {
        return db->getChanges();
    }

private:
    ordered_json getColumnValue(const SQLite::Statement& query, int column_index) const {
        const SQLite::Column column = query.getColumn(column_index);
        const int columnType = column.getType();
        
        if (columnType == SQLite::INTEGER) {
            return column.getInt64();
        } else if (columnType == SQLite::FLOAT) {
            return column.getDouble();
        } else if (columnType == SQLite::TEXT) {
            return column.getString();
        } else if (columnType == SQLite::BLOB) {
            return "[BLOB: " + to_string(column.getBytes()) + " bytes]";
        } else if (columnType == SQLite::Null) {
            return nullptr;
        } else {
            return nullptr;
        }
    }

    void bindParameters(SQLite::Statement& query, const ordered_json& params) const {
        if (params.is_array()) {
            // Позиционные параметры
            for (size_t i = 0; i < params.size(); ++i) {
                bindJsonValue(query, static_cast<int>(i + 1), params[i]);
            }
        } else if (params.is_object()) {
            // Именованные параметры (требуют использования :param в SQL)
             for (auto& [paramName, value] : params.items()) {
                // Используем только синтаксис :name
                string sqlParamName = ":" + paramName;
                try {
                    bindJsonValue(query, sqlParamName.c_str(), value);
                }
                catch (const SQLite::Exception& e) {
                    throw runtime_error("Cannot bind parameter '" + paramName + "': " + e.what());
                }
            }
        }
    }

    void bindJsonValue(SQLite::Statement& query, int index, const ordered_json& value) const {
        if (value.is_null()) {
            query.bind(index);
        } else if (value.is_string()) {
            query.bind(index, value.get<string>());
        } else if (value.is_number_integer()) {
            query.bind(index, static_cast<int64_t>(value.get<int64_t>()));
        } else if (value.is_number_float()) {
            query.bind(index, value.get<double>());
        } else if (value.is_boolean()) {
            query.bind(index, value.get<bool>() ? 1 : 0);
        } else {
            throw SQLite::Exception("Unsupported JSON type for binding");
        }
    }

    void bindJsonValue(SQLite::Statement& query, const char* name, const ordered_json& value) const {
        if (value.is_null()) {
            query.bind(name);
        } else if (value.is_string()) {
            query.bind(name, value.get<string>());
        } else if (value.is_number_integer()) {
            query.bind(name, static_cast<int64_t>(value.get<int64_t>()));
        } else if (value.is_number_float()) {
            query.bind(name, value.get<double>());
        } else if (value.is_boolean()) {
            query.bind(name, value.get<bool>() ? 1 : 0);
        } else {
            throw SQLite::Exception("Unsupported JSON type for binding");
        }
    }

    unique_ptr<SQLite::Database> db;
};

#endif