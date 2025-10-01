#include "db.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

PGconn *conn;

PGconn *connectDB(std::string host, std::string dbName, std::string user, std::string password)
{
    std::string connStr = "host=" + host + " dbname=" + dbName + " user=" + user + " password=" + password; 
    conn = PQconnectdb(connStr.c_str());
    if (PQstatus(conn) != CONNECTION_OK)
    {
        cerr << "\033[1;31mConnection to database failed: " << PQerrorMessage(conn) << "\033[0m\n"; // Red
        PQfinish(conn);
        return nullptr;
    }
    cout << "\033[1;32mConnected to the database successfully!\033[0m\n"; // Green

    PQprepare(conn, "addPaymentWithDate", "INSERT INTO payments (amount, label, date, method, category, cashflow) VALUES ($1, $2, $3, $4, $5, $6)", 6, nullptr);
    PQprepare(conn, "addPaymentWithoutDate", "INSERT INTO payments (amount, label, method, category, cashflow) VALUES ($1, $2, $3, $4, $5)", 5, nullptr);
    PQprepare(conn, "getAllPayments", "SELECT * FROM payments", 0, nullptr);
    PQprepare(conn, "getPaymentByAmount", "SELECT * FROM payments WHERE amount = $1", 1, nullptr);
    PQprepare(conn, "getPaymentByLabel", "SELECT * FROM payments WHERE label = $1", 1, nullptr);
    PQprepare(conn, "deletetransaction", "DELETE FROM payments WHERE id = $1", 1, nullptr);
    PQprepare(conn, "getTotalAmountSpent", "SELECT COALESCE(SUM(amount)) as totalAmountSpent FROM payments", 0, nullptr);
    PQprepare(conn, "getMonthlySpendings", "SELECT to_char(date, 'MMYYYY') AS month, category, SUM(amount) AS totalAmount FROM payments WHERE cashflow='Expense' GROUP BY month, category ORDER BY month ASC, totalAmount DESC", 0, nullptr);

    return conn;
}

void closeDB(PGconn *conn)
{
    PQfinish(conn);
}

bool addTransactionToDB(double amount, const string &label, const string &method, const string &category, Transaction::TransactionType type, std::optional<std::string> date)
{

    string strAmount = to_string(amount);
    string strType = Transaction::toString(type);
    PGresult *res = nullptr;
    if (date.has_value())
    {
        string strDate = date.value();
        std::cout << strDate << std::endl;
        const char *inputValues[6] = {strAmount.c_str(), label.c_str(), strDate.c_str(), method.c_str(), category.c_str(), strType.c_str()};
        res = PQexecPrepared(conn, "addPaymentWithDate", 6, inputValues, nullptr, nullptr, 0);
    }
    else
    {
        std::cout << method << std::endl;
        const char *inputValuesWithoutDate[5] = {strAmount.c_str(), label.c_str(), method.c_str(), category.c_str(), strType.c_str()};
        res = PQexecPrepared(conn, "addPaymentWithoutDate", 5, inputValuesWithoutDate, nullptr, nullptr, 0);
    }
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        cerr << "\033[1;31mError adding transaction: " << PQerrorMessage(conn) << "\033[0m\n"; // Red
        PQclear(res);
        return false;
    }
    PQclear(res);

    cout << "\033[1;32mTransaction added successfully!\033[0m\n"; // Green
    return true;
}

vector<Transaction> listTransactions()
{
    vector<Transaction> transactions;
    Transaction t;

    PGresult *res = PQexecPrepared(conn, "getAllPayments", 0, nullptr, nullptr, nullptr, 0);

    int rows = PQntuples(res);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        cerr << "\033[1;31mError getting transaction: " << PQresultErrorMessage(res) << "\033[0m\n";
        PQclear(res);
        return transactions;
    }

    if (rows == 0)
    {
        // cout << "\033[1;31mNo transactions found in the database!\033[0m\n"; // Red
        return transactions;
    }

    // cout << "\033[1;33m--- View All Transactions---\033[0m\n"; // Yellow
    for (int i = 0; i < rows; i++)
    {
        t.id = PQgetvalue(res, i, 0);
        t.amount = stod(PQgetvalue(res, i, 2));
        t.label = PQgetvalue(res, i, 3);
        t.date = PQgetvalue(res, i, 4);
        t.method = Transaction::fromStringToMethod(PQgetvalue(res, i, 5));
        t.category = PQgetvalue(res, i, 6);
        t.type = Transaction::fromStringToTransactionType(PQgetvalue(res, i, 7));
        transactions.push_back(t);
    }

    PQclear(res);
    return transactions;
}

vector<Transaction> searchTransaction(string searchWith, string value)
{

    vector<Transaction> transactions;
    const char *searchValue[1];
    searchValue[0] = {value.c_str()};
    Transaction searchRes;

    if (searchWith == "amount")
    {

        PGresult *res = PQexecPrepared(conn, "getPaymentByAmount", 1, searchValue, nullptr, nullptr, 0);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            cerr << "\033[1;31mError getting transaction: " << PQresultErrorMessage(res) << "\033[0m\n";
            PQclear(res);
            return transactions;
        }

        int rows = PQntuples(res);

        if (rows == 0)
        {
            cout << "\033[1;31mNo transactions found in the database!\033[0m\n"; // Red
            return transactions;
        }

        for (int i = 0; i < rows; i++)
        {
            searchRes.id = PQgetvalue(res, i, 0);
            searchRes.amount = stod(PQgetvalue(res, i, 2));
            searchRes.label = PQgetvalue(res, i, 3);
            searchRes.date = PQgetvalue(res, i, 4);
            transactions.push_back(searchRes);
        }
        PQclear(res);
    }

    else if (searchWith == "label")
    {
        PGresult *res = PQexecPrepared(conn, "getPaymentByLabel", 1, searchValue, nullptr, nullptr, 0);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            cerr << "\033[1;31mError getting transaction: " << PQresultErrorMessage(res) << "\033[0m\n";
            PQclear(res);
            return transactions;
        }

        int rows = PQntuples(res);

        if (rows == 0)
        {
            cout << "\033[1;31mNo transactions found in the database!\033[0m\n"; // Red
            return transactions;
        }

        for (int i = 0; i < rows; i++)
        {
            searchRes.id = PQgetvalue(res, i, 0);
            searchRes.amount = stod(PQgetvalue(res, i, 2));
            searchRes.label = PQgetvalue(res, i, 3);
            searchRes.date = PQgetvalue(res, i, 4);
            transactions.push_back(searchRes);
        }
        PQclear(res);
    }

    return transactions;
}

bool deleteTransactionFromDB(string id)
{
    const char *delValue[1];
    delValue[0] = {id.c_str()};
    PGresult *res = PQexecPrepared(conn, "deleteTransaction", 1, delValue, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        cerr << "\033[1;31mError deleting transaction: " << PQerrorMessage(conn) << "\033[0m\n"; // Red
        PQclear(res);
        return false;
    }

    char *rowsAffected = PQcmdTuples(res);
    if (rowsAffected && std::string(rowsAffected) == "0")
    {
        std::cout << "\033[1;33mNo transaction with that ID found.\033[0m\n";
        PQclear(res);
        return false;
    }

    std::cout << "\033[1;32mTransaction deleted successfully.\033[0m\n";
    PQclear(res);
    return true;
}

bool updateTransaction([[maybe_unused]] string id)
{
    return false;
}

string getTotalAmountSpentFromDB()
{

    PGresult *res = PQexecPrepared(conn, "getTotalAmountSpent", 0, nullptr, nullptr, nullptr, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << "Execution failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return "0.0";
    }
    string amountSpent = PQgetvalue(res, 0, 0);
    PQclear(res);
    return amountSpent;
}

std::vector<Transaction> searchTransactions(std::string &property, std::string &value)
{
    vector<Transaction> transactions;
    const char *searchValue[1];
    searchValue[0] = {value.c_str()};
    Transaction searchRes;

    value = "%" + value + "%";

    static std::unordered_map<std::string, std::string> propertyMap = {
        {"Amount", "amount::text"},
        {"Label", "label"},
        {"Category", "category"},
        {"Date", "date::text"},
        {"Method", "method::text"},
        {"Type", "cashflow::text"}};

    if (propertyMap.find(property) == propertyMap.end())
    {
        std::cerr << "Invalid field: " << property << std::endl;
        return {}; // FOR NOW, LATER HANDLE THE ERRORS BRUV
    }
    
    std::string sqlQuery = "SELECT * FROM payments WHERE " + propertyMap[property] + " ILIKE $1";
    const char *paramValues[1] = {value.c_str()};
    PGresult *res = PQexecParams(conn, sqlQuery.c_str(), 1, nullptr, paramValues, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << "Query failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return {};
    }

    if (res)
    {
        int rows = PQntuples(res);

        if (rows == 0)
        {
            cout << "\033[1;31mNo transactions found in the database!\033[0m\n"; // Red
            return transactions;
        }
        for (int i = 0; i < rows; i++)
        {
            searchRes.id = PQgetvalue(res, i, 0);
            searchRes.amount = stod(PQgetvalue(res, i, 2));
            searchRes.label = PQgetvalue(res, i, 3);
            searchRes.date = PQgetvalue(res, i, 4);
            searchRes.method = Transaction::fromStringToMethod(PQgetvalue(res, i, 5));
            searchRes.category = PQgetvalue(res, i, 6);
            searchRes.type = Transaction::fromStringToTransactionType(PQgetvalue(res, i, 7));
            transactions.push_back(searchRes);
        }
        PQclear(res);
    }
    return transactions;
}

std::map<int, std::unordered_map<std::string, double>> getMonthlySpendings()
{
    std::map<int, std::unordered_map<std::string, double>> monthlySpendings;

    PGresult *res = PQexecPrepared(conn, "getMonthlySpendings", 0, nullptr, nullptr, nullptr, 0);
    int rows = PQntuples(res);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << "Query failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return {};
    }
    if (rows == 0)
    {
        cout << "\033[1;31mNo transactions found in the database!\033[0m\n"; // Red
        return {};
    }

    for(int i = 0; i < rows; i++) {
       //monthlySpendings.insert({std::stoi(PQgetvalue(res, i, 0)), {PQgetvalue(res, i, 1), std::stod(PQgetvalue(res, i, 2))}});

        int month = std::stoi(PQgetvalue(res, i, 0));
        std::string category = PQgetvalue(res, i, 1);
        double amt = std::stod(PQgetvalue(res, i, 2));

        monthlySpendings[month][category] += amt;
    }

    return monthlySpendings;
}