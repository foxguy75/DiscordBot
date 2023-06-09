#include <cstdlib>
#include <string>

#include <dpp/dpp.h>

#include <constants.hxx>
#include <hordcommandhandler.hxx>

using namespace dpp;

void cleanUpCommand( dpp::cluster& theBot )
{
    try
    {
        dpp::slashcommand_map commandMaps{ theBot.global_commands_get_sync() };

        for( const auto& commands : commandMaps )
        {
            theBot.global_command_delete_sync( commands.first );
        }
    }
    catch( std::exception& ex )
    {
        theBot.log( dpp::loglevel::ll_error, ex.what() );
    }
    return;
}

int main()
{
    // Discord API Key:
    const std::string botToken{ std::getenv( Constants::Env::DISCORD_BOT_TOKEN.c_str() ) };

    uint32_t intents = dpp::i_all_intents;

    dpp::cluster bot{ botToken, intents };

    bot.on_log( dpp::utility::cout_logger() );

    CommandHandler commandHandler{ bot };

    bot.on_slashcommand( commandHandler );

    bot.on_button_click(
        [ &bot ]( const button_click_t& event )
        {
            event.delete_original_response();
            event.reply( fmt::format( "You clicked: {}", event.custom_id ) );
        } );

    bot.on_ready(
        [ &bot, &commandHandler ]( const dpp::ready_t& event )
        {
            if( dpp::run_once<struct register_bot_commands>() )
            {
                cleanUpCommand( bot );
                /* Create a new global command on ready event */
                dpp::slashcommand hordCommand( "hord", "Command for all things in your hord!", bot.me.id );

                dpp::command_option hordCommandBalanceOption{ dpp::co_sub_command, "balance",
                                                              "returns the balance of your personal hord.", false };

                dpp::command_option hordCommandDepositOption{ dpp::co_sub_command, "deposit",
                                                              "add an amount to your hord.", false };

                dpp::command_option depositOption{ dpp::co_number, "amount", "the amount to be added", true };

                hordCommandDepositOption.add_option( depositOption );

                hordCommand.add_option( hordCommandBalanceOption ).add_option( hordCommandDepositOption );

                /* Register the command */
                bot.global_command_create( hordCommand );
            }
        } );

    bot.start( false );

    return 0;
}