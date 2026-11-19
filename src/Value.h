#pragma once
#include <string>

struct FunctionNode;   // forward decl

struct Value {
    enum class Kind { Null, Int, Float, String, Bool, Function };

    Kind kind = Kind::Null;
    long long intVal = 0;
    double floatVal = 0.0;
    std::string strVal;
    bool boolVal = false;
    const FunctionNode* funcVal = nullptr;

    static Value null()              { return Value{}; }
    static Value makeInt(long long v)   { Value x; x.kind = Kind::Int;    x.intVal = v; return x; }
    static Value makeFloat(double v)    { Value x; x.kind = Kind::Float;  x.floatVal = v; return x; }
    static Value makeString(std::string v) { Value x; x.kind = Kind::String; x.strVal = std::move(v); return x; }
    static Value makeBool(bool v)       { Value x; x.kind = Kind::Bool;   x.boolVal = v; return x; }
    static Value makeFunction(const FunctionNode* f) { Value x; x.kind = Kind::Function; x.funcVal = f; return x; }

    std::string typeName() const;
    std::string toString() const;
    bool isTruthy() const;
    bool isNumber() const;
    double asDouble() const;
};