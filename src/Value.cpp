#include "Value.h"
#include <sstream>

std::string Value::typeName() const {
    switch (kind) {
        case Kind::Null:     return "null";
        case Kind::Int:      return "int";
        case Kind::Float:    return "float";
        case Kind::String:   return "string";
        case Kind::Bool:     return "bool";
        case Kind::Function: return "function";
    }
    return "?";
}

std::string Value::toString() const {
    switch (kind) {
        case Kind::Null:   return "null";
        case Kind::Int:    return std::to_string(intVal);
        case Kind::Float:  {
            std::ostringstream oss;
            oss << floatVal;
            return oss.str();
        }
        case Kind::String: return strVal;
        case Kind::Bool:   return boolVal ? "true" : "false";
        case Kind::Function: return "<function>";
    }
    return "";
}

bool Value::isTruthy() const {
    switch (kind) {
        case Kind::Null:     return false;
        case Kind::Int:      return intVal != 0;
        case Kind::Float:    return floatVal != 0.0;
        case Kind::String:   return !strVal.empty();
        case Kind::Bool:     return boolVal;
        case Kind::Function: return true;
    }
    return false;
}

bool Value::isNumber() const {
    return kind == Kind::Int || kind == Kind::Float;
}

double Value::asDouble() const {
    if (kind == Kind::Float) return floatVal;
    if (kind == Kind::Int)   return static_cast<double>(intVal);
    return 0.0;
}