#pragma once

#include <dep/nlohmann/json.hpp>
#include <string>
#include <cstdint>

namespace endpoint {
	auto initialize( const std::string& bot_token ) -> void;

	auto send_message( const std::string& channel_id, const std::string& content ) -> nlohmann::json;
	auto edit_message( const std::string& channel_id, const std::string& message_id, const std::string& content ) -> nlohmann::json;
	auto delete_message( const std::string& channel_id, const std::string& message_id ) -> nlohmann::json;
	auto read_messages( const std::string& channel_id, std::uint32_t limit ) -> nlohmann::json;
	auto list_guilds( ) -> nlohmann::json;
	auto list_channels( const std::string& guild_id ) -> nlohmann::json;
	auto reply_to_message( const std::string& channel_id, const std::string& message_id, const std::string& content ) -> nlohmann::json;
	auto react_to_message( const std::string& channel_id, const std::string& message_id, const std::string& emoji ) -> nlohmann::json;
	auto get_self( ) -> nlohmann::json;
	auto read_messages_after( const std::string& channel_id, const std::string& after_id, std::uint32_t limit ) -> nlohmann::json;
}
