#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <limits>
#include <sstream>
#include <iomanip>
#include "../db/db.h"
#include <postgresql/libpq-fe.h>

using namespace std;
class FinancialTracker {
    public:
        FinancialTracker(PGconn *conn);
        int getIntInput(string prompt);
        void printMenu();
        void printTransactions();
        void addTransaction();

    private:
        PGconn* conn;
};