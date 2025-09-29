#ifndef DB_H
#define DB_H

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <ctime>
#include <limits>
#include <sstream>
#include <iomanip>
#include <optional>
#include <dotenv.h>
#include <postgresql/libpq-fe.h>

// Transaction structure

struct Transaction
{
    enum class TransactionType {
    Income,
    Expense,
    COUNT,
    NOTSET
    };

    enum class Method {
        Cash,
        UPI,
        Card,
        BankTransfer,
        COUNT, //just to get the length, bruh cpp doesn't have have a method for this!??
        NOTSET
    };

    std::string id;
    double amount;
    std::string label;
    std::string date;
    Method method = Method::NOTSET;
    std::string category = "Miscellaneous";
    TransactionType type = TransactionType::NOTSET;

    static std::string toString(TransactionType type) {
        switch(type) {
            case TransactionType::Expense : return "Expense";
            case TransactionType::Income : return "Income";
            case TransactionType::NOTSET : return "NOT SET";
        }
        throw std::invalid_argument("Invalid transaction type (toString).");
    }

    static std::string toString(Method method) {
        switch(method) {
            case Method::Cash : return "Cash";
            case Method::UPI : return "UPI";
            case Method::Card : return "Card";
            case Method::BankTransfer : return "Bank Transfer";
            case Method::NOTSET : return "NOT SET";
        }
        throw std::invalid_argument("Invalid method type (toString).");
    }

    static TransactionType fromStringToTransactionType(std::string type) {
        if(type == "Income") return TransactionType::Income;
        else if(type == "Expense") return TransactionType::Expense;
        else {throw std::invalid_argument("Invalid transaction type (fromString)." + type); }
    }

    static Method fromStringToMethod(std::string method) {
        if(method == "Cash") return Method::Cash;
        else if(method == "UPI") return Method::UPI;
        else if(method == "Card") return Method::Card;
        else if(method == "Bank Transfer") return Method::BankTransfer;

        else {throw std::invalid_argument("Invalid method type (fromString)." + method); }
    }

};

PGconn *connectDB(std::string host, std::string dbName, std::string user, std::string password);
void closeDB(PGconn *conn);

bool addTransactionToDB(double amount, const std::string &label, const std::string &method,const std::string &category, Transaction::TransactionType type, std::optional<std::string> date = std::nullopt);
std::vector<Transaction> listTransactions();
std::string getTotalAmountSpentFromDB();
bool deleteTransactionFromDB(std::string id);
std::vector<Transaction> searchTransactions(std::string &property, std::string &value);
std::map<int, std::unordered_map<std::string, double>> getMonthlySpendings();


#endif