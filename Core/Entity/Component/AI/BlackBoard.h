#pragma once
#include <any>
#include "Core/Entity/Component/JActorComponent.h"

class BlackBoard
{
public:
    template <typename T>
    void setValue(const std::string& key, T value)
    {
        data[key] = std::make_any<T>(std::move(value));
    }
	
    template <typename T>
    T getValue(const std::string& key) const
    {
        return std::any_cast<T> (data.at(key));
    }

    template <typename T>
    T& getValueRef(const std::string& key)
    {
        return std::any_cast<T&>(data.at(key));
    }
	
private:
    std::unordered_map<std::string, std::any> data;
};
