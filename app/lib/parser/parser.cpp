#include "parser.h"

bool hasAlphabet(const std::string &s)
{
    for (char c : s)
    {
        if (std::isalpha(c))
        {
            return true;
        }
    }
    return false;
}

bool isUnwanted(const std::string &word)
{
    bool hasSlash = false;
    bool hasDash = false;
    for (char c : word)
    {
        if (c == '/')
        {
            hasSlash = true;
        }
        else if (c == '-')
        {
            hasDash = true;
        }
    }
    return hasSlash && hasDash;
}

    using namespace poppler;

    std::vector<Transaction> parsePDF(const std::string &filePath)
    {
        std::vector<Transaction> parsedTransactions = {};
        std::unique_ptr<document> doc(document::load_from_file(filePath));
        std::string rawContent;
        if (!doc)
        {
            std::cout << "Failed to load the doc" << std::endl;
            return {};
        }

        for (int i = 0; i < doc->pages(); i++)
        {
            std::unique_ptr<page> p(doc->create_page(i));
            rawContent = p->text().to_latin1();

            // rawContent.erase(std::remove(rawContent.begin(), rawContent.end(), ' '), rawContent.end());
            // std::cout << rawContent << std::endl;
            std::istringstream iss(rawContent);
            std::string line;
            while (getline(iss, line))
            {
                // if (line.empty()) continue;

                int n = 0;
                int wordNum = 0;
                int skip = 0;
                std::string date, amount, word, next;
                while (iss >> word)
                {
                    wordNum++;
                    if (i == 0 && wordNum < 31)
                        continue; // skip the header stuff
                    // skip the footer stuff
                    if (skip > 0)
                    {
                        skip--;
                        continue;
                    }
                    if (word == "Date")
                    {
                        if (iss >> next && next == "and")
                        {
                            skip = 10;
                            continue;
                        }
                    }
                    Transaction t;
                    if (!hasAlphabet(word) && !isUnwanted(word)) // only want numbers. TODO: THE CREDIT STUFF FAILS CUZ THAT MF got NO LETTERS AND '-' & '/' & NUMS SO FIGURE THAT OUT LMAO
                    {
                        if (n == 0)
                        {
                            std::cout << word << std::endl;
                            date = word;
                            std::cout << "The date is : " << date << " when n is: " << n << std::endl;
                            n++;
                        }
                        else if (n == 1)
                        {
                            std::cout << word << std::endl;
                            amount = word;
                            std::cout << "The amount is : " << amount << " when n is: " << n << std::endl;
                            n++;
                        }
                        else
                        {
                            n = 0;
                            t.date = date; 
                            double signedAmount = std::stod(amount);
                            t.type = ( signedAmount < 0) ? Transaction::TransactionType::Expense : Transaction::TransactionType::Income;
                            t.amount = std::abs(signedAmount);
                            parsedTransactions.push_back(t);
                            continue;
                        }
                    }
                    // parsedTransactions[txCount].amount = stod(amount);
                    // parsedTransactions[txCount].date = date;
                }

                // std::cout << date << "_______" << amount << "_______" << balance << "_______" << std::endl;
            }
        }
        int txs = 0;
        for (auto &t : parsedTransactions)
        {
            std::cout << t.amount << "  " << t.date << std::endl;
            txs++;
        }
        std::cout << "===========================" << txs << std::endl;
        return parsedTransactions;
    }
