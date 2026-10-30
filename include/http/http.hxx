#pragma once

#include <string>
#include <cstdint>

namespace http {
	struct response_t {
		std::uint32_t m_status_code{ };
		std::string   m_body;
	};

	auto get( const std::string& path, const std::string& token ) -> response_t;
	auto post( const std::string& path, const std::string& body, const std::string& token ) -> response_t;
	auto patch( const std::string& path, const std::string& body, const std::string& token ) -> response_t;
	auto del( const std::string& path, const std::string& token ) -> response_t;
	auto put_empty( const std::string& path, const std::string& token ) -> response_t;
	auto url_encode( const std::string& str ) -> std::string;
}
