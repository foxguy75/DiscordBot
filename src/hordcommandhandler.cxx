#include <cstdlib>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <constants.hxx>
#include <hordcommandhandler.hxx>
#include <transaction.hxx>

using namespace dpp;
using namespace nlohmann;

namespace
{
    json loadHordJson()
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

            return json::parse( jsonFileStr );
        }

        return json();
    }

    std::string getBalance( const user& theUser )
    {
        json hordJson{ loadHordJson() };

        const std::string dndName{ hordJson.at( theUser.username ).at( Constants::Json::Keys::DND_NAME ) };
        const int amount{ hordJson.at( theUser.username ).at( Constants::Json::Keys::AMOUNT_IN_HORD ) };

        return fmt::format<const std::string&, const int&>( Constants::Responses::BALANCE, dndName, amount );
    }

    std::string generateUUID( const unsigned int theLength )
    {
        std::stringstream ss;

        std::random_device random_device;
        std::mt19937 generator( random_device() );
        std::uniform_int_distribution<> distribution( 0, 255 );

        for( int i = 0; i < theLength; ++i )
        {
            ss << std::hex << int( distribution( generator ) );
        }

        return ss.str();
    }
}  // namespace

void CommandHandler::operator()( const slashcommand_t& theEvent )
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
        m_bot.log( loglevel::ll_error, fmt::format( "{}", ex.what() ) );
        responseMessage = Constants::Responses::DEFAULT_ERROR;
    }
    catch( ... )
    {
        responseMessage = Constants::Responses::DEFAULT_ERROR;
    }

    theEvent.reply( message( responseMessage ) );
    return;
}

std::string CommandHandler::hordHandler( const interaction& theCommand )
{
    std::string message{};

    if( theCommand.get_command_interaction().options.size() > 0 )
    {
        for( const command_data_option& elem : theCommand.get_command_interaction().options )
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

std::string CommandHandler::deposit( const interaction& theCommand )
{
    // Build transaction
    command_data_option subCommand{ theCommand.get_command_interaction().options[ 0 ] };

    Transaction newTransaction{};

    newTransaction.issuingGuildID = theCommand.get_guild().id;
    newTransaction.issuingGuildName = theCommand.get_guild().name;

    for( const command_data_option option : subCommand.options )
    {
        if( option.name == "amount" )
        {
            newTransaction.amount = std::get<double>( subCommand.options[ 0 ].value );
        }
    }

    newTransaction.userName = theCommand.get_issuing_user().username;

    newTransaction.uuid = generateUUID( 6 );

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
    guild issuingGuild{ m_bot.guild_get_sync( theTransaction.issuingGuildID ) };

    role_map issuingGuildRoles{ m_bot.roles_get_sync( theTransaction.issuingGuildID ) };

    role_map::iterator dmRole = std::find_if( std::begin( issuingGuildRoles ), std::end( issuingGuildRoles ),
                                              []( std::pair<snowflake, role> roleEntry ) -> bool
                                              { return roleEntry.second.name == Constants::Role::DM; } );

    if( dmRole == std::end( issuingGuildRoles ) )
    {
        m_bot.log( loglevel::ll_error, fmt::format<const std::string&, const std::string&>(
                                           Constants::Logging::Error::MISSING_ROLE, theTransaction.issuingGuildName,
                                           Constants::Role::DM ) );
    }
    else
    {
        guild_member_map members{ m_bot.guild_get_members_sync( theTransaction.issuingGuildID, 1000, snowflake{} ) };

        guild_member_map::iterator dmMember = std::find_if(
            std::begin( members ), std::end( members ),
            [ &dmRole ]( const std::pair<snowflake, guild_member>& member )
            {
                return std::find_if( std::begin( member.second.roles ), std::end( member.second.roles ),
                                     [ &dmRole ]( snowflake role )
                                     { return role == dmRole->second.id; } ) != std::end( member.second.roles );
            } );

        if( dmMember != std::end( members ) )
        {
            message messageToSend{ fmt::format( "Hey {}, wants to add {}gp to their stash! Is that ok?",
                                                members[ theTransaction.issuingUser ].nickname,
                                                theTransaction.amount ) };

            component actionRow{};

            component approveButton{};
            approveButton.set_label( "Approve" )
                .set_type( cot_button )
                .set_style( cos_success )
                .set_id( fmt::format( "Transaction_Approve_{}", theTransaction.uuid ) );

            actionRow.add_component( approveButton );

            component rejectButton{};
            rejectButton.set_label( "Reject" )
                .set_type( cot_button )
                .set_style( cos_danger )
                .set_id( fmt::format( "Transaction_Reject_{}", theTransaction.uuid ) );

            actionRow.add_component( rejectButton );

            messageToSend.add_component( actionRow );

            theTransaction.dmMessage = m_bot.direct_message_create_sync( dmMember->second.user_id, messageToSend );

            pendingTransactions.push_back( std::move( theTransaction ) );
        }

        return;
    }
}
