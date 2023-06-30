#pragma once

#include <yaml-cpp/yaml.h>
#include <fstream>

#include "tools/Logging.h"

namespace TP {

class Serializer {
    public:
        Serializer(const std::string& filepath) : m_filepath(filepath) {
            m_emitter << YAML::BeginMap;
        }
        YAML::Emitter& get() {return m_emitter;}
        void done() {
            m_emitter << YAML::EndMap;
            std::ofstream fout(m_filepath);
            fout << m_emitter.c_str();
        }
    private:
        std::string m_filepath;
        YAML::Emitter m_emitter;
};

class Deserializer {
    public:
        Deserializer() : m_valid(false) {}
        Deserializer(const std::string& filepath) : m_valid(true) {
            try {
                m_node = YAML::LoadFile(filepath);
            } catch (YAML::ParserException e) {
                ERROR("Failed to load file" << filepath << " ("<< e.what() <<")");
            }
        }
        Deserializer(const YAML::Node& node) : m_valid(true), m_node(node) {}
        const YAML::Node& get() const {return m_node;}

        operator bool() const {return m_valid;}
    private:
        bool m_valid = false;
        YAML::Node m_node;
};

}