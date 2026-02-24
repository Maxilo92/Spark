#pragma once

#include <sol/sol.hpp>

namespace Spark {

	class ScriptGlue
	{
	public:
		static void RegisterComponents(sol::state& lua);
	};

}
