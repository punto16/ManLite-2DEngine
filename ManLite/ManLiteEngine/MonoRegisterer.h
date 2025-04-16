#pragma once

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/attrdefs.h"

#include "ScriptingEM.h"

#include "string"
#include "vector"

class MonoRegisterer
{
public:
	static void RegisterFunctions();

    static MonoString* ToMonoString(const std::string& str);
    static std::string ToCppString(MonoString* monoStr);

    template<typename T>
    static MonoArray* VectorToMonoArray(const std::vector<T>& vec, MonoClass* elementClass)
    {
        MonoDomain* domain = ScriptingEM::GetAppDomain();
        MonoArray* monoArray = mono_array_new(domain, elementClass, vec.size());
    
        for (size_t i = 0; i < vec.size(); ++i) {
            mono_array_set(monoArray, T, i, vec[i]);
        }
    
        return monoArray;
    }
    template<typename T>
    static std::vector<T> MonoArrayToVector(MonoArray* monoArray)
    {
        std::vector<T> vec;
        if (!monoArray) return vec;

        uintptr_t length = mono_array_length(monoArray);
        vec.reserve(length);

        for (uintptr_t i = 0; i < length; ++i) {
            vec.push_back(mono_array_get(monoArray, T, i));
        }

        return vec;
    }

    static MonoArray* VectorToMonoArray(const std::vector<std::string>& vec)
    {
        MonoDomain* domain = ScriptingEM::GetAppDomain();
        MonoClass* stringClass = mono_get_string_class();
        MonoArray* monoArray = mono_array_new(domain, stringClass, vec.size());

        for (size_t i = 0; i < vec.size(); ++i) {
            mono_array_set(monoArray, MonoString*, i, ToMonoString(vec[i]));
        }

        return monoArray;
    }

    static std::vector<std::string> MonoArrayToStringVector(MonoArray* monoArray)
    {
        std::vector<std::string> vec;
        if (!monoArray) return vec;

        uintptr_t length = mono_array_length(monoArray);
        vec.reserve(length);

        for (uintptr_t i = 0; i < length; ++i) {
            MonoString* monoStr = mono_array_get(monoArray, MonoString*, i);
            vec.push_back(ToCppString(monoStr));
        }

        return vec;
    }
};