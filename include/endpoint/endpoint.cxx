#include <core/endpoint/endpoint.hxx>
#include <core/http/http.hxx>

namespace endpoint {
	static std::string m_token;
	static constexpr auto m_base = "/api/v10";

	auto initialize( const std::string& bot_token ) -> void {
		m_token = bot_token;
	}

	static auto parse_response( const http::response_t& response ) -> nlohmann::json {
		if ( response.m_status_code == 0 )
			return { { "error", "request failed" } };

		if ( response.m_body.empty( ) )
			return { { "success", response.m_status_code >= 200 && response.m_status_code < 300 } };

		auto parsed = nlohmann::json::parse( response.m_body, nullptr, false );
		if ( parsed.is_discarded( ) )
			return { { "error", "invalid json response" }, { "status", response.m_status_code } };

		if ( response.m_status_code >= 400 ) {
			parsed[ "status" ] = response.m_status_code;
			parsed[ "error" ] = true;
		}
		return parsed;
	}

	auto send_message( const std::string& channel_id, const std::string& content ) -> nlohmann::json {
		auto body = nlohmann::json{ { "content", content } };
		auto path = std::string( m_base ) + "/channels/" + channel_id + "/messages";
		return parse_response( http::post( path, body.dump( ), m_token ) );
	}

	auto edit_message( const std::string& channel_id, const std::string& message_id, const std::string& content ) -> nlohmann::json {
		auto body = nlohmann::json{ { "content", content } };
		auto path = std::string( m_base ) + "/channels/" + channel_id + "/messages/" + message_id;
		return parse_response( http::patch( path, body.dump( ), m_token ) );
	}

	auto delete_message( const std::string& channel_id, const std::string& message_id ) -> nlohmann::json {
		auto path = std::string( m_base ) + "/channels/" + channel_id + "/messages/" + message_id;
		return parse_response( http::del( path, m_token ) );
	}

	auto read_messages( const std::string& channel_id, std::uint32_t limit ) -> nlohmann::json {
		auto path = std::string( m_base ) + "/channels/" + channel_id + "/messages?limit=" + std::to_string( limit );
		return parse_response( http::get( path, m_token ) );
	}

	auto list_guilds( ) -> nlohmann::json {
		auto path = std::string( m_base ) + "/users/@me/guilds";
		return parse_response( http::get( path, m_token ) );
	}

	auto list_channels( const std::string& guild_id ) -> nlohmann::json {
		auto path = std::string( m_base ) + "/guilds/" + guild_id + "/channels";
		return parse_response( http::get( path, m_token ) );
	}

	auto reply_to_message( const std::string& channel_id, const std::string& message_id, const std::string& content ) -> nlohmann::json {
		auto body = nlohmann::json{
			{ "content", content },
			{ "message_reference", { { "message_id", message_id } } }
		};
		auto path = std::string( m_base ) + "/channels/" + channel_id + "/messages";
		return parse_response( http::post( path, body.dump( ), m_token ) );
	}

	auto react_to_message( const std::string& channel_id, const std::string& message_id, const std::string& emoji ) -> nlohmann::json {
		auto encoded = http::url_encode( emoji );
		auto path = std::string( m_base ) + "/channels/" + channel_id + "/messages/" + message_id + "/reactions/" + encoded + "/@me";
		return parse_response( http::put_empty( path, m_token ) );
	}

	auto get_self( ) -> nlohmann::json {
		auto path = std::string( m_base ) + "/users/@me";
		return parse_response( http::get( path, m_token ) );
	}

	auto read_messages_after( const std::string& channel_id, const std::string& after_id, std::uint32_t limit ) -> nlohmann::json {
		auto path = std::string( m_base ) + "/channels/" + channel_id + "/messages?after=" + after_id + "&limit=" + std::to_string( limit );
		return parse_response( http::get( path, m_token ) );
	}
}
