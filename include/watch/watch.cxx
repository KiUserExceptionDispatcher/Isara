#include <core/watch/watch.hxx>
#include <core/endpoint/endpoint.hxx>

#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <chrono>

namespace watch {
	static std::vector< std::string > m_keywords;
	static std::unordered_map< std::string, std::string > m_last_seen;
	static std::string m_self_id;

	static constexpr auto m_conversation_timeout = std::chrono::minutes( 5 );
	static std::unordered_map< std::string, std::chrono::steady_clock::time_point > m_conversations;

	static auto to_lower( std::string str ) -> std::string {
		std::transform( str.begin( ), str.end( ), str.begin( ), [ ] ( unsigned char c ) { return static_cast< char >( std::tolower( c ) ); } );
		return str;
	}

	static auto is_word_char( char c ) -> bool {
		return std::isalnum( static_cast< unsigned char >( c ) ) || c == '_';
	}

	static auto snowflake( const std::string& id ) -> std::uint64_t {
		return id.empty( ) ? 0 : std::stoull( id );
	}

	auto initialize( const std::vector< std::string >& keywords ) -> void {
		for ( const auto& keyword : keywords )
			m_keywords.push_back( to_lower( keyword ) );
	}

	static auto conversation_key( const std::string& channel_id, const std::string& author_id ) -> std::string {
		return channel_id + ":" + author_id;
	}

	static auto is_in_conversation( const std::string& channel_id, const std::string& author_id ) -> bool {
		auto it = m_conversations.find( conversation_key( channel_id, author_id ) );
		if ( it == m_conversations.end( ) )
			return false;

		if ( std::chrono::steady_clock::now( ) - it->second > m_conversation_timeout ) {
			m_conversations.erase( it );
			return false;
		}
		return true;
	}

	static auto start_conversation( const std::string& channel_id, const std::string& author_id ) -> void {
		m_conversations[ conversation_key( channel_id, author_id ) ] = std::chrono::steady_clock::now( );
	}

	static auto prune_conversations( ) -> void {
		auto now = std::chrono::steady_clock::now( );
		for ( auto it = m_conversations.begin( ); it != m_conversations.end( ); ) {
			if ( now - it->second > m_conversation_timeout )
				it = m_conversations.erase( it );
			else
				++it;
		}
	}

	static auto find_keyword( const std::string& content ) -> std::string {
		auto lowered = to_lower( content );

		for ( const auto& keyword : m_keywords ) {
			auto pos = lowered.find( keyword );
			while ( pos != std::string::npos ) {
				auto end = pos + keyword.size( );
				auto start_ok = pos == 0 || !is_word_char( lowered[ pos - 1 ] );
				auto end_ok = end >= lowered.size( ) || !is_word_char( lowered[ end ] );
				if ( start_ok && end_ok )
					return keyword;

				pos = lowered.find( keyword, pos + 1 );
			}
		}
		return { };
	}

	static auto is_directed_at_self( const nlohmann::json& message ) -> bool {
		for ( const auto& mention : message.value( "mentions", nlohmann::json::array( ) ) ) {
			if ( mention.value( "id", std::string{ } ) == m_self_id )
				return true;
		}

		auto referenced = message.find( "referenced_message" );
		if ( referenced == message.end( ) || !referenced->is_object( ) )
			return false;

		return referenced->value( "author", nlohmann::json::object( ) ).value( "id", std::string{ } ) == m_self_id;
	}

	auto get_mentions( const std::string& channel_id ) -> nlohmann::json {
		if ( m_self_id.empty( ) ) {
			auto self = endpoint::get_self( );
			if ( !self.is_object( ) || !self.contains( "id" ) )
				return { { "error", "could not fetch self" } };

			m_self_id = self[ "id" ].get< std::string >( );
		}

		auto last = m_last_seen.find( channel_id );
		if ( last == m_last_seen.end( ) ) {
			auto latest = endpoint::read_messages( channel_id, 1 );
			if ( !latest.is_array( ) )
				return latest;

			m_last_seen[ channel_id ] = latest.empty( ) ? "0" : latest[ 0 ].value( "id", std::string{ "0" } );
			return { { "watching", channel_id }, { "mentions", nlohmann::json::array( ) } };
		}

		prune_conversations( );

		auto messages = endpoint::read_messages_after( channel_id, last->second, 100 );
		if ( !messages.is_array( ) )
			return messages;

		std::sort( messages.begin( ), messages.end( ), [ ] ( const nlohmann::json& a, const nlohmann::json& b ) {
			return snowflake( a.value( "id", std::string{ } ) ) < snowflake( b.value( "id", std::string{ } ) );
		} );

		auto mentions = nlohmann::json::array( );
		for ( const auto& message : messages ) {
			auto id = message.value( "id", std::string{ } );
			if ( snowflake( id ) > snowflake( last->second ) )
				last->second = id;

			auto author = message.value( "author", nlohmann::json::object( ) );
			auto author_id = author.value( "id", std::string{ } );
			if ( author_id == m_self_id )
				continue;

			auto content = message.value( "content", std::string{ } );
			auto keyword = find_keyword( content );
			auto directed = is_directed_at_self( message );
			auto in_convo = is_in_conversation( channel_id, author_id );

			if ( keyword.empty( ) && !directed && !in_convo )
				continue;

			if ( keyword == "claude" || directed || in_convo )
				start_conversation( channel_id, author_id );

			auto name = author.contains( "global_name" ) && author[ "global_name" ].is_string( )
				? author[ "global_name" ].get< std::string >( )
				: author.value( "username", std::string{ } );

			mentions.push_back( nlohmann::json{
				{ "message_id", id },
				{ "author", name },
				{ "author_id", author_id },
				{ "content", content },
				{ "keyword", keyword },
				{ "directed_at_you", directed },
				{ "conversation", in_convo }
			} );
		}

		return { { "mentions", mentions } };
	}
}
