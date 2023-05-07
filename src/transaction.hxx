#ifndef TRANSACTION_HXX
#define TRANSACTION_HXX

#include <string>

#include <dpp/dpp.h>

enum class TransactionStatus
{
    pending,
    approved,
    rejected
};

struct Transaction
{
    std::string uuid;
    double amount;
    dpp::snowflake issuingUser;
    std::string userName;
    std::string issuingGuildName;
    dpp::snowflake issuingGuildID;
    dpp::message dmMessage;
    TransactionStatus status{ TransactionStatus::pending };
};
#endif