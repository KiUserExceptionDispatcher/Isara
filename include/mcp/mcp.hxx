#pragma once

#include <dep/nlohmann/json.hpp>
#include <string>
#include <vector>
#include <functional>

namespace mcp {
	struct tool_param_t {
		std::string m_name;
		std::string m_type;
		std::string m_description;
		bool        m_required{ true };
	};

	struct tool_t {
		std::string                  m_name;
		std::string                  m_description;
		std::vector< tool_param_t > m_params;
		std::function< nlohmann::json( const nlohmann::json& ) > m_handler;
	};

	auto register_tool( const tool_t& tool ) -> void;
	auto run( ) -> void;
}
