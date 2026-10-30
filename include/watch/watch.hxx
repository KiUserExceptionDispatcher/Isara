#pragma once

#include <dep/nlohmann/json.hpp>
#include <string>
#include <vector>

namespace watch {
	auto initialize( const std::vector< std::string >& keywords ) -> void;
	auto get_mentions( const std::string& channel_id ) -> nlohmann::json;
}
