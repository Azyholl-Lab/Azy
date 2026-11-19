#pragma once
#include <unordered_map>
#include <string>
#include "Value.h"

struct Environment {
    Environment* parent = nullptr;
    std::unordered_map<std::string, Value> vars;

    Environment() = default;
    explicit Environment(Environment* p) : parent(p) {}

    void define(const std::string& name, Value v) {
        vars[name] = std::move(v);
    }

    bool assign(const std::string& name, Value v) {
        auto it = vars.find(name);
        if (it != vars.end()) { it->second = std::move(v); return true; }
        if (parent) return parent->assign(name, std::move(v));
        return false;
    }

    Value* lookup(const std::string& name) {
        auto it = vars.find(name);
        if (it != vars.end()) return &it->second;
        if (parent) return parent->lookup(name);
        return nullptr;
    }
};