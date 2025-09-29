#include "FinancialTracker.h"
#include "../db/db.h"

FinancialTracker::FinancialTracker(PGconn *conn) {
    conn = conn;
}

int FinancialTracker::getIntInput(string prompt)
{
    int val;
    while (true)
    {
        cout << prompt;
        if (cin >> val)
        {
            return val;
        }
        cout << "\033[1;31mInvalid input, please enter a number!\033[0m\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        continue;
    }
}

// Function to pretty print transactions
void FinancialTracker::printTransactions()
{

    Transaction t;

    while (true)
    {
        cout << "\n\033[1;33m--- Transactions ---\033[0m\n"; // Yellow
        cout << "1. View all transactions \n2. Search Transaction \n3. Go back to Transactions Tracker" << endl;
        int viewChoice;
        viewChoice = getIntInput("Choose an option: ");

        if (viewChoice == 2)
        {
            Transaction searchResult;
            while (true)
            {
                // cout << "\033[1;33m--- Search Transactions---\033[0m\n"; // Yellow
                cout << "\n\033[1;33m--- Search Transactions---\033[0m \nSearch with: \n1. Amount \n2. Label \n3. Go back to Transactions menu" << endl;
                int searchChoice;
                searchChoice = getIntInput("Choose an option: ");

                if (searchChoice == 1)
                {

                    double amount;
                    string strAmount;

                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    while (true)
                    {

                        string line;
                        cout << "\nEnter Amount: ";
                        getline(cin, line);
                        stringstream isstr(line);
                        if (isstr >> amount && isstr.eof())
                        {
                            break;
                        }
                        else
                        {
                            cout << "\033[1;31mInvalid input, please enter a valid amount!\033[0m\n"; // Red
                            isstr.clear();
                        }
                    }
                    strAmount = to_string(amount);

                    vector<Transaction> getTransactions = searchTransaction("amount", strAmount);

                    if (getTransactions.size() != 0)
                    {
                        cout << "\033[1;34m"; // Blue headers
                        cout << left << setw(5) << "Id" << setw(10) << "Amount"
                             << setw(50) << "Label"
                             << setw(40) << "date" << "\n";
                        cout << string(100, '-') << "\n";
                        cout << "\033[0m"; // Reset color

                        for (const auto &t : getTransactions)
                        {
                            cout << left << setw(5) << t.id << setw(10) << t.amount
                                 << setw(50) << t.label
                                 << setw(40) << t.date << "\n";
                        }
                    }
                    getTransactions.clear();
                }

                else if (searchChoice == 2)
                {
                    string label;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Enter label: ";
                    getline(cin >> ws, label);

                    vector<Transaction> getTransactions = searchTransaction("label", label);
                    if (getTransactions.size() != 0)
                    {
                        cout << "\033[1;34m"; // Blue headers
                        cout << left << setw(5) << "Id" << setw(10) << "Amount"
                             << setw(50) << "Label"
                             << setw(40) << "date" << "\n";
                        cout << string(100, '-') << "\n";
                        cout << "\033[0m"; // Reset color

                        for (const auto &t : getTransactions)
                        {
                            cout << left << setw(5) << t.id << setw(10) << t.amount
                                 << setw(50) << t.label
                                 << setw(40) << t.date << "\n";
                        }
                    }
                    getTransactions.clear();
                }
                else if (searchChoice == 3)
                {
                    break;
                }
            }
        }

        else if (viewChoice == 3)
        {
            return;
        }

        else if (viewChoice == 1)
        {
            vector<Transaction> printTransactions = listTransactions();

            cout << "\033[1;34m"; // Blue headers
            cout << left << setw(5) << "Id" << setw(10) << "Amount"
                 << setw(50) << "Label"
                 << setw(40) << "date" << "\n";
            cout << string(100, '-') << "\n";
            cout << "\033[0m"; // Reset color

            for (const auto &t : printTransactions)
            {
                cout << left << setw(5) << t.id << setw(10) << t.amount
                     << setw(50) << t.label
                     << setw(40) << t.date << "\n";
            }
        }
    }
}

// Add a transaction
void FinancialTracker::addTransaction()
{
    double amount;
    string label;

    cout << "\033[1;33m--- Add Transaction ---\033[0m\n"; // Yellow
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    while (true)
    {
        string line;
        cout << "Enter amount: ";
        getline(cin, line);
        stringstream isstr(line);
        if (isstr >> amount && isstr.eof() && amount > 0)
        {
            break;
        }
        else
        {
            cout << "\033[1;31mInvalid input, please enter a valid amount!\033[0m\n"; // Red
            isstr.clear();
        }
    }
    cout << "Enter label: ";
    getline(cin >> ws, label);

    addTransactionToDB( amount, label);
}

