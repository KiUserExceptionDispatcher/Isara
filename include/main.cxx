#define _CRT_SECURE_NO_WARNINGS

#include <core/mcp/mcp.hxx>
#include <core/endpoint/endpoint.hxx>
#include <core/watch/watch.hxx>
#include <cstdlib>

auto main( ) -> int {
	auto token = std::getenv( "DISCORD_BOT_TOKEN" ); // < either a token for your discord bot, or your user token.
	if ( !token )
		return 1;

	endpoint::initialize( token );
	watch::initialize( { "ki", "leo", "abyss", "claude" } );

	mcp::register_tool( {
		.m_name = "send_message",
		.m_description = "Send a message to a Discord channel",
		.m_params = {
			{ "channel_id", "string", "The channel ID to send the message to" },
			{ "content", "string", "The message content to send" }
		},
		.m_handler = [ ] ( const nlohmann::json& args ) -> nlohmann::json {
			return endpoint::send_message( args[ "channel_id" ].get< std::string >( ), args[ "content" ].get< std::string >( ) );
		}
	});

	mcp::register_tool( {
		.m_name = "edit_message",
		.m_description = "Edit an existing message in a Discord channel",
		.m_params = {
			{ "channel_id", "string", "The channel ID containing the message" },
			{ "message_id", "string", "The message ID to edit" },
			{ "content", "string", "The new message content" }
		},
		.m_handler = [ ] ( const nlohmann::json& args ) -> nlohmann::json {
			return endpoint::edit_message( args[ "channel_id" ].get< std::string >( ), args[ "message_id" ].get< std::string >( ), args[ "content" ].get< std::string >( ) );
		}
	});

	mcp::register_tool( {
		.m_name = "delete_message",
		.m_description = "Delete a message from a Discord channel",
		.m_params = {
			{ "channel_id", "string", "The channel ID containing the message" },
			{ "message_id", "string", "The message ID to delete" }
		},
		.m_handler = [ ] ( const nlohmann::json& args ) -> nlohmann::json {
			return endpoint::delete_message( args[ "channel_id" ].get< std::string >( ), args[ "message_id" ].get< std::string >( ) );
		}
	});

	mcp::register_tool( {
		.m_name = "read_messages",
		.m_description = "Read recent messages from a Discord channel",
		.m_params = {
			{ "channel_id", "string", "The channel ID to read messages from" },
			{ "limit", "number", "Number of messages to fetch (1-100, default 25)", false }
		},
		.m_handler = [ ] ( const nlohmann::json& args ) -> nlohmann::json {
			auto limit = args.value( "limit", 25 );
			return endpoint::read_messages( args[ "channel_id" ].get< std::string >( ), static_cast< std::uint32_t >( limit ) );
		}
	});

	mcp::register_tool( {
		.m_name = "list_guilds",
		.m_description = "List all Discord servers the bot is in",
		.m_params = { },
		.m_handler = [ ] ( const nlohmann::json& ) -> nlohmann::json {
			return endpoint::list_guilds( );
		}
	});

	mcp::register_tool( {
		.m_name = "list_channels",
		.m_description = "List all channels in a Discord server",
		.m_params = {
			{ "guild_id", "string", "The server ID to list channels for" }
		},
		.m_handler = [ ] ( const nlohmann::json& args ) -> nlohmann::json {
			return endpoint::list_channels( args[ "guild_id" ].get< std::string >( ) );
		}
	});

	mcp::register_tool( {
		.m_name = "reply_to_message",
		.m_description = "Reply to a specific message in a Discord channel",
		.m_params = {
			{ "channel_id", "string", "The channel ID containing the message" },
			{ "message_id", "string", "The message ID to reply to" },
			{ "content", "string", "The reply content" }
		},
		.m_handler = [ ] ( const nlohmann::json& args ) -> nlohmann::json {
			return endpoint::reply_to_message( args[ "channel_id" ].get< std::string >( ), args[ "message_id" ].get< std::string >( ), args[ "content" ].get< std::string >( ) );
		}
	});

	mcp::register_tool( {
		.m_name = "react_to_message",
		.m_description = "Add a reaction emoji to a message",
		.m_params = {
			{ "channel_id", "string", "The channel ID containing the message" },
			{ "message_id", "string", "The message ID to react to" },
			{ "emoji", "string", "The emoji to react with (unicode or custom name:id)" }
		},
		.m_handler = [ ] ( const nlohmann::json& args ) -> nlohmann::json {
			return endpoint::react_to_message( args[ "channel_id" ].get< std::string >( ), args[ "message_id" ].get< std::string >( ), args[ "emoji" ].get< std::string >( ) );
		}
	});

	mcp::register_tool( {
		.m_name = "get_mentions",
		.m_description = "Get new messages in a channel since the last check that mention keywords (ki, leo, abyss, claude), ping you, reply to you, or come from someone in an active conversation. Saying 'hey claude' or similar starts a 5-minute conversation window where all their follow-up messages are captured. The first call on a channel only starts watching and returns nothing",
		.m_params = {
			{ "channel_id", "string", "The channel ID to watch" }
		},
		.m_handler = [ ] ( const nlohmann::json& args ) -> nlohmann::json {
			return watch::get_mentions( args[ "channel_id" ].get< std::string >( ) );
		}
	});

	mcp::run( );
	return 0;
}
