#pragma once

#include "Bindable.h"
#include <type_traits>
#include <memory>
#include <unordered_map>

namespace Bind
{
	// Stores bindables by string guids
	/*class Codex
	{
	public:
		static std::shared_ptr<Bindable> Resolve(const std::string& key)
		{
			return GetInstance().InternalResolve(key);
		}
		static void Store(std::shared_ptr<Bindable> bind)
		{
			return GetInstance().InternalStore(bind);
		}
	private:
		std::shared_ptr<Bindable> InternalResolve(const std::string& key) const
		{
			const auto i = binds.find(key);
			if (i == binds.end())
			{
				return {};
			}
			else
			{
				return i->second;
			}
		}
		void InternalStore(std::shared_ptr<Bindable> bind)
		{
			binds[bind->GetUID()] = std::move(bind);
		}
		static Codex& GetInstance()
		{
			static Codex codex;
			return codex;
		}
	private:
		std::unordered_map<std::string, std::shared_ptr<Bindable>> binds;
	};*/
	
	class Codex
	{
	public:
		///
		/// Return bindable type, creating it if needed
		///
		template<class T,typename...Params>
		static std::shared_ptr<T> Resolve( Graphics& gfx, Params&&...p )
		{
			static_assert( std::is_base_of<Bindable,T>::value, "Can only resolve classes derived from Bindable" );
			return GetInstance().InternalResolve<T>( gfx,std::forward<Params>( p )... );
		}
	private:
		template<class T,typename...Params>
		std::shared_ptr<T> InternalResolve( Graphics& gfx, Params&&...p )
		{
			// Pass all params to GenerateUID
			const auto key = T::GenerateUID( std::forward<Params>( p )... );
			const auto i = binds.find( key );
			if( i == binds.end() )
			{
				// Create Bindable
				auto bind = std::make_shared<T>( gfx,std::forward<Params>( p )... );
				binds[key] = bind;
				return bind;
			}
			else
			{
				return std::static_pointer_cast<T>( i->second );
			}
		}
		static Codex& GetInstance()
		{
			static Codex codex;
			return codex;
		}
	private:
		std::unordered_map<std::string,std::shared_ptr<Bindable>> binds;
	};
}