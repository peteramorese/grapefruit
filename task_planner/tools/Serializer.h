#pragma once

#include <yaml-cpp/yaml.h>
#include <fstream>

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

}