#pragma once

#include <string>
#include <vector>

#include "tools/Logging.h"

namespace TP {

    class ArgParser {
        public:
            ArgParser(int argc, char** argv) : m_argc(argc), m_argv(argv), m_checked(argc, false) {}

            bool hasFlag(char flag, const std::string& description = std::string()) {
                std::string flag_str = getFlag(flag);
                m_descriptions.emplace_back(flag_str, std::string(), description);
                for (uint32_t i=1; i<m_argc; ++i) {
                    if (m_checked[i]) continue;

                    std::string arg = m_argv[i];
                    if (arg == flag_str) {
                        m_checked[i] = true;
                        return true;
                    }
                }
                return false;
            }

            template <typename T>
            T parse(const std::string& key, T default_value = T{}, const std::string& description = std::string());

            bool hasKey(const std::string& key, const std::string& description = std::string()) {
                std::string key_str = getKey(key);
                if (!description.empty()) m_descriptions.emplace_back(key_str, std::string(), description);
                for (uint32_t i=1; i<m_argc; ++i) {
                    std::string arg = m_argv[i];
                    if (arg == key_str) return true;
                }
                return false;
            }

            // Include this to enable the help function, and terminate the main()
            // For terminating, use: if (parser.enableHelp()) return 0;
            bool enableHelp() {
                bool has_help = hasKey("help");
                if (has_help) {
                    for (auto&[key, def, desc] : m_descriptions) {
                        std::string description = desc + " ";
                        if (!def.empty()) description += " [Default value: " + def + "]";
                        PRINT_NAMED(key, description);
                    }
                    return true;
                }
                return false;
            }

        private:
            struct Documentation {
                Documentation(const std::string& key_, const std::string& default_value_, const std::string& description_)
                    : key(key_)
                    , default_value(default_value_)
                    , description(description_)
                {}

                std::string key;
                std::string default_value;
                std::string description;
            };
        private:
            inline std::string getFlag(char flag) const {
                return "-" + std::string{flag};
            }
            inline std::string getKey(const std::string& key) const {return "--" + key;}

            std::vector<bool> m_checked;
            int m_argc;
            char** m_argv;
            std::vector<Documentation> m_descriptions;
    };

    template <>
    std::string ArgParser::parse<std::string>(const std::string& key, std::string default_value, const std::string& description) {
        std::string key_str = getKey(key);
        m_descriptions.emplace_back(key_str, default_value, description);
        for (uint32_t i=1; i<m_argc; ++i) {
            if (m_checked[i]) continue;

            std::string arg = m_argv[i];
            if (arg == key_str) {
                ASSERT(i < m_argc - 1, "Parse key: '" << key << "' given with no string value");
                m_checked[i] = true;
                m_checked[i + 1] = true;
                return m_argv[i + 1];
            }
        }
        return default_value;
    }

    template <>
    int ArgParser::parse<int>(const std::string& key, int default_value, const std::string& description) {
        std::string key_str = getKey(key);
        m_descriptions.emplace_back(key_str, std::to_string(default_value), description);
        for (uint32_t i=1; i<m_argc; ++i) {
            if (m_checked[i]) continue;

            std::string arg = m_argv[i];
            if (arg == key_str) {
                ASSERT(i < m_argc - 1, "Parse key: '" << key << "' given with no int value");
                m_checked[i] = true;
                m_checked[i + 1] = true;
                return std::stoi(m_argv[i + 1], std::string::size_type());
            }
        }
        return default_value;
    }

    template <>
    uint32_t ArgParser::parse<uint32_t>(const std::string& key, uint32_t default_value, const std::string& description) {
        return static_cast<uint32_t>(parse<int>(key, default_value, description));
    }


}