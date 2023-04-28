#ifndef HORDCOMMANDHANDLER_HXX
#define HORDCOMMANDHANDLER_HXX

#include <mutex>
#include <vector>

#include <dpp/dpp.h>

class CommandHandler
{
   public:
    CommandHandler( dpp::cluster& theBot ) : m_bot{ theBot } {};
    void operator()( const dpp::slashcommand_t& theEvent );

   private:
    dpp::cluster& m_bot;

    enum class TransactionStatus
    {
        pending,
        approved,
        rejected
    };

    struct Transaction
    {
        double amount;
        dpp::snowflake issuingUser;
        std::string userName;
        std::string issuingGuildName;
        dpp::snowflake issuingGuildID;
        TransactionStatus status{ TransactionStatus::pending };
    };

    std::string hordHandler( const dpp::interaction& theCommand );
    std::string deposit( const dpp::interaction& theEvent );
    std::string withdraw();

    void proccessTransaction( Transaction&& theTransaction );
};

#endif  // !HORDCOMMANDHANDLER_HXX
