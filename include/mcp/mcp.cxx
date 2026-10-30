#include <core/mcp/mcp.hxx>
#include <iostream>
#include <string>

namespace mcp {
	static std::vector< tool_t > m_tools;

	auto register_tool( const tool_t& tool ) -> void {
		m_tools.push_back( tool );
	}

	static auto write_response( const nlohmann::json& response ) -> void {
		std::cout << response.dump( ) << "\n";
		std::cout.flush( );
	}

	static auto build_tool_schema( const tool_t& tool ) -> nlohmann::json {
		auto properties = nlohmann::json::object( );
		auto required = nlohmann::json::array( );

		for ( const auto& param : tool.m_params ) {
			properties[ param.m_name ] = {
				{ "type", param.m_type },
				{ "description", param.m_description }
			};
			if ( param.m_required )
				required.push_back( param.m_name );
		}

		return {
			{ "name", tool.m_name },
			{ "description", tool.m_description },
			{ "inputSchema", {
				{ "type", "object" },
				{ "properties", properties },
				{ "required", required }
			}}
		};
	}

	static auto handle_initialize( const nlohmann::json& request ) -> void {
		write_response( {
			{ "jsonrpc", "2.0" },
			{ "id", request[ "id" ] },
			{ "result", {
				{ "protocolVersion", "2024-11-05" },
				{ "capabilities", { { "tools", nlohmann::json::object( ) } } },
				{ "serverInfo", { { "name", "isara" }, { "version", "1.0.0" } } }
			}}
		});
	}

	static auto handle_tools_list( const nlohmann::json& request ) -> void {
		auto tools = nlohmann::json::array( );
		for ( const auto& tool : m_tools )
			tools.push_back( build_tool_schema( tool ) );

		write_response( {
			{ "jsonrpc", "2.0" },
			{ "id", request[ "id" ] },
			{ "result", { { "tools", tools } } }
		});
	}

	static auto handle_tools_call( const nlohmann::json& request ) -> void {
		auto tool_name = request[ "params" ][ "name" ].get< std::string >( );
		auto arguments = request[ "params" ].value( "arguments", nlohmann::json::object( ) );

		for ( const auto& tool : m_tools ) {
			if ( tool.m_name != tool_name )
				continue;

			auto result = tool.m_handler( arguments );
			write_response( {
				{ "jsonrpc", "2.0" },
				{ "id", request[ "id" ] },
				{ "result", {
					{ "content", nlohmann::json::array( { { { "type", "text" }, { "text", result.dump( 2 ) } } } ) }
				}}
			});
			return;
		}

		write_response( {
			{ "jsonrpc", "2.0" },
			{ "id", request[ "id" ] },
			{ "error", { { "code", -32601 }, { "message", "tool not found: " + tool_name } } }
		});
	}

	static auto handle_ping( const nlohmann::json& request ) -> void {
		write_response( {
			{ "jsonrpc", "2.0" },
			{ "id", request[ "id" ] },
			{ "result", nlohmann::json::object( ) }
		});
	}

	auto run( ) -> void {
		std::string line;
		while ( std::getline( std::cin, line ) ) {
			if ( line.empty( ) )
				continue;

			auto request = nlohmann::json::parse( line, nullptr, false );
			if ( request.is_discarded( ) )
				continue;

			auto method = request.value( "method", std::string{ } );

			if ( method == "initialize" )
				handle_initialize( request );
			else if ( method == "notifications/initialized" )
				continue;
			else if ( method == "tools/list" )
				handle_tools_list( request );
			else if ( method == "tools/call" )
				handle_tools_call( request );
			else if ( method == "ping" )
				handle_ping( request );
		}
	}
}
