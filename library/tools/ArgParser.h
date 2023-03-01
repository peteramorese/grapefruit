#pragma once

#include <string>
#include <vector>

#include "tools/Logging.h"

namespace TP {

    class ArgParser {
        public:
            ArgParser(int argc, char** argv) : m_argc(argc), m_argv(argv), m_checked(argc, false) {}

            bool hasFlag(char flag) {
                std::string flag_str = "-" + std::string{flag};
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

            std::string parseAsString(const std::string& key, const std::string& default_value = std::string()) {
                std::string key_str = "--" + key;
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

            uint32_t parseAsUnsignedInt(const std::string& key, uint32_t default_value = 0) {
                std::string key_str = "--" + key;
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

        private:
            std::vector<bool> m_checked;
            int m_argc;
            char** m_argv;
    };
}