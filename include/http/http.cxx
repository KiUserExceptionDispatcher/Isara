#include <core/http/http.hxx>

#include <Windows.h>
#include <winhttp.h>
#include <vector>
#include <sstream>
#include <iomanip>

#pragma comment( lib, "winhttp.lib" )

namespace http {
	static auto to_wide( const std::string& str ) -> std::wstring {
		if ( str.empty( ) )
			return { };

		auto size = MultiByteToWideChar( CP_UTF8, 0, str.data( ), static_cast< int >( str.size( ) ), nullptr, 0 );
		auto result = std::wstring( size, L'\0' );
		MultiByteToWideChar( CP_UTF8, 0, str.data( ), static_cast< int >( str.size( ) ), result.data( ), size );
		return result;
	}

	static auto send_request( const std::wstring& method, const std::string& path,
		const std::string& body, const std::string& token ) -> response_t {
		response_t response{ };

		auto session = WinHttpOpen( L"isara/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );
		if ( !session )
			return response;

		auto connection = WinHttpConnect( session, L"discord.com", INTERNET_DEFAULT_HTTPS_PORT, 0 );
		if ( !connection ) {
			WinHttpCloseHandle( session );
			return response;
		}

		auto wide_path = to_wide( path );
		auto request = WinHttpOpenRequest( connection, method.c_str( ), wide_path.c_str( ), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE );
		if ( !request ) {
			WinHttpCloseHandle( connection );
			WinHttpCloseHandle( session );
			return response;
		}

		auto auth_header = to_wide( "Authorization: " + token );
		WinHttpAddRequestHeaders( request, auth_header.c_str( ), static_cast< DWORD >( -1 ), WINHTTP_ADDREQ_FLAG_ADD );

		if ( !body.empty( ) )
			WinHttpAddRequestHeaders( request, L"Content-Type: application/json", static_cast< DWORD >( -1 ), WINHTTP_ADDREQ_FLAG_ADD );

		auto sent = WinHttpSendRequest( request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, body.empty( ) ? nullptr : const_cast< char* >( body.data( ) ), static_cast< DWORD >( body.size( ) ), static_cast< DWORD >( body.size( ) ), 0 );
		if ( !sent ) {
			WinHttpCloseHandle( request );
			WinHttpCloseHandle( connection );
			WinHttpCloseHandle( session );
			return response;
		}

		if ( !WinHttpReceiveResponse( request, nullptr ) ) {
			WinHttpCloseHandle( request );
			WinHttpCloseHandle( connection );
			WinHttpCloseHandle( session );
			return response;
		}

		DWORD status_code = 0;
		DWORD status_size = sizeof( status_code );
		WinHttpQueryHeaders( request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status_code, &status_size, WINHTTP_NO_HEADER_INDEX );
		response.m_status_code = status_code;

		std::string result_body;
		DWORD bytes_available = 0;
		while ( WinHttpQueryDataAvailable( request, &bytes_available ) && bytes_available > 0 ) {
			auto buffer = std::vector< char >( bytes_available );
			DWORD bytes_read = 0;
			if ( WinHttpReadData( request, buffer.data( ), bytes_available, &bytes_read ) )
				result_body.append( buffer.data( ), bytes_read );
		}
		response.m_body = std::move( result_body );

		WinHttpCloseHandle( request );
		WinHttpCloseHandle( connection );
		WinHttpCloseHandle( session );
		return response;
	}

	auto get( const std::string& path, const std::string& token ) -> response_t {
		return send_request( L"GET", path, { }, token );
	}

	auto post( const std::string& path, const std::string& body, const std::string& token ) -> response_t {
		return send_request( L"POST", path, body, token );
	}

	auto patch( const std::string& path, const std::string& body, const std::string& token ) -> response_t {
		return send_request( L"PATCH", path, body, token );
	}

	auto del( const std::string& path, const std::string& token ) -> response_t {
		return send_request( L"DELETE", path, { }, token );
	}

	auto put_empty( const std::string& path, const std::string& token ) -> response_t {
		return send_request( L"PUT", path, { }, token );
	}

	auto url_encode( const std::string& str ) -> std::string {
		auto stream = std::ostringstream{ };
		stream.fill( '0' );
		stream << std::hex;

		for ( auto c : str ) {
			if ( std::isalnum( static_cast< unsigned char >( c ) ) || c == '-' || c == '_' || c == '.' || c == '~' )
				stream << c;
			else
				stream << '%' << std::setw( 2 ) << std::uppercase << static_cast< int >( static_cast< unsigned char >( c ) );
		}
		return stream.str( );
	}
}
