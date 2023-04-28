#ifndef CONSTANTS_HXX
#define CONSTANTS_HXX

#include <string>

#include <fmt/format.h>

namespace Constants
{
    namespace Env
    {
        const std::string HORD_JSON{ "HordJson" };
        const std::string DISCORD_BOT_TOKEN{ "DiscordBotToken" };
    }  // namespace Env

    namespace Json::Keys
    {
        const std::string DND_NAME{ "DnDName" };
        const std::string AMOUNT_IN_HORD{ "AmountInHord" };
    }  // namespace Json::Keys

    namespace Responses
    {
        constexpr fmt::format_string<const std::string&, const int&> BALANCE{
            "Greeting {}! There is current {} Gold Pieces in your hord." };
        const std::string DEFAULT_ERROR{
            "Oopsie Woopsie! UwU I made a fucky wucky! A "
            "wittle fucko boingo! The code monkey know as "
            "Bork is working VEWY HAWD to fix this!!" };
    }  // namespace Responses

    namespace Role
    {
        const std::string DM{ "Admin" };
    }

    namespace Logging
    {
        namespace Debug
        {
            constexpr fmt::format_string<const std::string&> DISPLAY_ISSUING_GUILD{ "Called from Guild: {}" };

        }  // namespace Debug

        namespace Error
        {
            constexpr fmt::format_string<const std::string&, const std::string&> MISSING_ROLE{
                "IssuingGuild: {} is missing a member with the following role: {}. At least one member with this role "
                "is need. Please give a member this role and try again." };
        }  // namespace Error
    }      // namespace Logging

}  // namespace Constants

#endif  // CONSTANTS_HXX
