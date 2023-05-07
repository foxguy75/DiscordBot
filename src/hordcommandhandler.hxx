#ifndef HORDCOMMANDHANDLER_HXX
#define HORDCOMMANDHANDLER_HXX

#include <mutex>
#include <vector>

#include <dpp/dpp.h>

#include <transaction_decl.hxx>

class CommandHandler
{
   public:
    CommandHandler( dpp::cluster& theBot ) : m_bot{ theBot } {};
    void operator()( const dpp::slashcommand_t& theEvent );

   private:
    dpp::cluster& m_bot;

    std::string hordHandler( const dpp::interaction& theCommand );
    std::string deposit( const dpp::interaction& theEvent );
    std::string withdraw();

    std::vector<Transaction> pendingTransactions{};
    std::vector<Transaction> rejectedTransactions{};
    std::vector<Transaction> approvedTransactions{};

    void proccessTransaction( Transaction&& theTransaction );
};

#endif  // !HORDCOMMANDHANDLER_HXX
