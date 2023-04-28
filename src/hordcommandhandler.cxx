#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <constants.hxx>
#include <hordcommandhandler.hxx>

namespace
{
    nlohmann::json loadHordJson()
    {
        std::fstream jsonFile{ std::getenv( Constants::Env::HORD_JSON.c_str() ) };

        if( jsonFile.is_open() )
        {
            std::string jsonFileStr{};
            std::string line{};

            while( std::getline( jsonFile, line ) )
            {
                jsonFileStr += line;
            }

            return nlohmann::json::parse( jsonFileStr );
        }

        return nlohmann::json();
    }

    std::string getBalance( const dpp::user& theUser )
    {
        nlohmann::json hordJson{ loadHordJson() };

        const std::string dndName{ hordJson.at( theUser.username ).at( Constants::Json::Keys::DND_NAME ) };
        const int amount{ hordJson.at( theUser.username ).at( Constants::Json::Keys::AMOUNT_IN_HORD ) };

        return fmt::format<const std::string&, const int&>( Constants::Responses::BALANCE, dndName, amount );
    }
}  // namespace

void CommandHandler::operator()( const dpp::slashcommand_t& theEvent )
{
    std::string responseMessage{};

    try
    {
        if( theEvent.command.get_command_name() == "hord" )
        {
            responseMessage = hordHandler( theEvent.command );
        }
    }
    catch( const std::exception& ex )
    {
        m_bot.log( dpp::loglevel::ll_error, fmt::format( "{}", ex.what() ) );
        responseMessage = Constants::Responses::DEFAULT_ERROR;
    }
    catch( ... )
    {
        responseMessage = Constants::Responses::DEFAULT_ERROR;
    }

    theEvent.reply( dpp::message( responseMessage ) );
    return;
}

std::string CommandHandler::hordHandler( const dpp::interaction& theCommand )
{
    std::string message{};

    if( theCommand.get_command_interaction().options.size() > 0 )
    {
        for( const dpp::command_data_option& elem : theCommand.get_command_interaction().options )
        {
            if( elem.name == "balance" )
            {
                message = getBalance( theCommand.get_issuing_user() );
            }
            else if( elem.name == "deposit" )
            {
                message = deposit( theCommand );
            }
            else if( elem.name == "withdraw" )
            {
                message = withdraw();
            }
        }
    }
    else
    {
        // TODO: Throw an appropriate error message.
        message = "Default message";
    }
    return message;
}

std::string CommandHandler::deposit( const dpp::interaction& theCommand )
{
    // Build transaction
    dpp::command_data_option subCommand{ theCommand.get_command_interaction().options[ 0 ] };

    Transaction newTransaction{};

    newTransaction.issuingGuildID = theCommand.get_guild().id;
    newTransaction.issuingGuildName = theCommand.get_guild().name;

    for( const dpp::command_data_option option : subCommand.options )
    {
        if( option.name == "amount" )
        {
            newTransaction.amount = std::get<double>( subCommand.options[ 0 ].value );
        }
    }

    newTransaction.userName = theCommand.get_issuing_user().username;

    // Build Response
    std::string response{ fmt::format( "Alright {} your deposit request of {} is being processed.",
                                       newTransaction.userName, newTransaction.amount ) };

    // Start new thread to handle
    std::thread processTransactionThread{ &CommandHandler::proccessTransaction, this, std::move( newTransaction ) };
    processTransactionThread.detach();

    return response;
}

std::string CommandHandler::withdraw() { return Constants::Responses::DEFAULT_ERROR; }

void CommandHandler::proccessTransaction( Transaction&& theTransaction )
{
    m_bot.log( dpp::loglevel::ll_debug,
               fmt::format<const std::string&>( Constants::Logging::Debug::DISPLAY_ISSUING_GUILD,
                                                theTransaction.issuingGuildName ) );

    dpp::guild issuingGuild{ m_bot.guild_get_sync( theTransaction.issuingGuildID ) };

    dpp::role_map issuingGuildRoles{ m_bot.roles_get_sync( theTransaction.issuingGuildID ) };

    std::unordered_map<dpp::snowflake, dpp::role>::iterator dmRole =
        std::find_if( std::begin( issuingGuildRoles ), std::end( issuingGuildRoles ),
                      []( std::pair<dpp::snowflake, dpp::role> roleEntry ) -> bool
                      { return roleEntry.second.name == Constants::Role::DM; } );

    if( dmRole == std::end( issuingGuildRoles ) )
    {
        m_bot.log( dpp::loglevel::ll_error, fmt::format<const std::string&, const std::string&>(
                                                Constants::Logging::Error::MISSING_ROLE,
                                                theTransaction.issuingGuildName, Constants::Role::DM ) );
    }
    else
    {
        std::unordered_map<dpp::snowflake, dpp::guild_member> dmMembers = dmRole->second.get_members();

        m_bot.log( dpp::loglevel::ll_debug,
                   fmt::format( "Found: {} member with role: {}", dmMembers.size(), dmRole->second.name ) );

        if( dmMembers.size() == 1 )
        {
            dpp::snowflake dmChannel{ m_bot.get_dm_channel( dmMembers[ 0 ].user_id ) };
        
            if( dmChannel.empty() )
            {
                dpp::channel dmMessageChannel = m_bot.create_dm_channel_sync( dmMembers[ 0 ].user_id );
                dmChannel = dmMessageChannel.id;
            }

            dpp::message messageToSend{
                dmChannel, fmt::format( "Hey DM {}, a PC {}, wants to add {}gp to their stash! Is that ok?",
                                        dmMembers[ 0 ].nickname, theTransaction.issuingUser, theTransaction.amount ) };

            m_bot.message_create_sync( messageToSend );
        }

        return;
    }
}
