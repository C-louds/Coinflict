#pragma once


#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <iostream>
#include <memory>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>


#include "../db/db.h"

std::vector<Transaction> parsePDF(const std::string &filePath);