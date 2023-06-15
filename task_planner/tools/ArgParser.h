#pragma once

#include <string>
#include <vector>
#include <exception>

#include "tools/Logging.h"

namespace TP {

    template <typename T>
    class DataArgument {
        public:
            void set(const T& arg) {m_arg = arg;}
            void set(T&& arg) {m_arg = std::move(arg);}
            const T& get() const {
                return m_arg;
            }
        private:
            T m_arg;
    }

    class IndicatorArgument {};

    template <typename T, typename BASE = std::conditional<!std::is_same<T, void>::value, DataArgument<T>, IndicatorArgument>::type>
    class Argument : BASE {
        public:
            Argument(bool valid) : m_valid(valid) {}
            operator bool() const {return m_valid;}
        private:
            bool m_valid;
    };

    template <typename T>
    class ArrayArgument {
        public:
            ArrayArgument(const std::vector<T>& args, bool valid) : m_args(args), m_valid(valid) {}
            const std::vector<T>& get() const {return m_args;}
            operator bool() const {return m_valid;}
        private:
            std::vector<T> m_args;
            bool m_valid;
    };

    class ArgParser {
        public:
            ArgParser(int argc, char** argv) : m_argc(argc), m_argv(argv), m_checked(argc, false) {
            }

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
            Argument<T> parse(const std::string& key, char flag, const T& default_value, const std::string& description) {
                static_assert(!std::is_same<T, void>::value, "Do not specify default value with indicator");
                return _parse(&flag, &key, &default_value, &description);
            }

            template <typename T>
            Argument<T> parse(const std::string& key, const T& default_value, const std::string& description) {
                static_assert(!std::is_same<T, void>::value, "Do not specify default value with indicator");
                return _parse(nullptr, &key, &default_value, &description);
            }

            template <typename T>
            Argument<T> parse(const std::string& key, const std::string& description) {
                return _parse(nullptr, &key, &default_value, &description);
            }

            template <typename T>
            Argument<T> parse(const std::string& key, const T& default_value = T{}, const std::string& description = std::string()) {
            }

            template <typename T>
            Argument<T> parse(char flag, const T& default_value = T{}, const std::string& description = std::string()) {
            }

            Argument<void> parse(const std::string& key, const std::string& description = std::string()) {
            }

            Argument<void> parse(const std::string& key, char flag, const std::string& description = std::string()) {
            }

            Argument<void> parse(char flag, const std::string& description = std::string()) {
            }


            // Include this to enable the help function and check for unrecognized arguments
            bool enableHelp() {
                bool has_help = parse<>('h') || parse<>("help");
                if (has_help) {
                    for (auto&[flag, key, def, desc] : m_descriptions) {
                        std::string description = desc + " ";

                        std::string key_and_flag;
                        if (!flag.empty() && !key.empty()) {
                            key_and_flag = key + " or " + flag;
                        } else if (!key.empty()) {
                            key_and_flag = key;
                        } else if (!flag.empty()) {
                            key_and_flag = flag;
                        } else {
                            ASSERT(false, "Empty key and flag");
                        }
                        
                        if (!def.empty()) description += " [Default value: " + def + "]";
                        PRINT_NAMED(key_and_flag, description);
                    }
                    std::exit(0);
                    return false;
                }

                for (uint32_t i=1; i<m_argc; ++i) {
                    if (!m_checked[i]) {
                        ERROR("Unrecognized arg '" << m_argv[i]);
                        std::exit(1);
                        return false;
                    }
                }
                return true;
            }

        private:
            struct Documentation {
                Documentation(const std::string& flag_, const std::string& key_, const std::string& default_value_, const std::string& description_)
                    : flag(flag_)
                    , key(key_)
                    , default_value(default_value_)
                    , description(description_)
                {}

                std::string flag;
                std::string key;
                std::string default_value;
                std::string description;
            };
        private:
            inline std::string getFlag(char flag) const {
                return "-" + std::string{flag};
            }
            inline std::string getKey(const std::string& key) const {return "--" + key;}

            template <typename T>
            T to(const std::string& str);

            template <typename T>
            std::string from(const T& v);

            bool isValue(const std::string& arg) const {
                return arg.find("-") != 0u;
            }
        
            template <typename T>
            Argument<T> _parse(const char* flag, const std::string* key, const T* default_value, const std::string* description) {
                // Determines if the argument is just an indicator, or holding a value in the next slot
                constexpr bool indicator = std::is_same<T, void>::value;

                if (!key && !flag) {
                    throw std::invalid_argument("Both key and flag cannot be unspecified");
                }

                // Flag
                std::string flag_str = std::string();
                if (flag) {
                    flag_str = getFlag(*flag);
                }

                // Key
                std::string key_str = std::string();
                if (key) {
                    ASSERT(isValue(key_str), "Do not use dashes '--' when specifying a key label");
                    key_str = getKey(*key);
                }

                // Default
                std::string default_str = std::string();
                if constexpr (!indicator) {
                    if (default_value)
                        default_str = from<T>(*default_value);
                }

                // Description
                std::string desc_str = std::string();
                if (description)
                    desc_str = *description;

                // Add the necessary descriptions
                m_descriptions.emplace_back(flag_str, key_str, default_str, desc_str);

                for (uint32_t i=1; i<m_argc; ++i) {
                    if (m_checked[i]) continue;

                    std::string arg = m_argv[i];
                    if ((flag && arg == flag_str) || (key && arg == key_str)) {
                        m_checked[i] = true;
                        if constexpr (indicator) {
                            ASSERT(i < m_argc - 1, "Parse key: '" << key << "' expected a value (use type T = void for indicator arguments)");
                            return Argument<T>(true);
                        } else {
                            m_checked[i + 1] = true;
                            T val = to<T>(m_argv[i + 1]);
                            Argument<T> ret(true);
                            ret.set(std::move(val));
                            return ret;
                        }
                    }
                }
                if constexpr (indicator) {
                    return Argument<T>(false);
                } else {
                    if (default_value) {
                        Argument<T> ret(true);
                        ret.set(*default_value);
                        return ret;
                    } else {
                        return Argument<T>(false);
                    }
                }
            }

        private:

            std::vector<bool> m_checked;
            int m_argc;
            char** m_argv;
            std::vector<Documentation> m_descriptions;
    };

    template <>
    std::string ArgParser::to<std::string>(const std::string& str) {return str;}
    template <>
    std::string ArgParser::from<std::string>(const std::string& v) {return v;}

    template <>
    int ArgParser::to<int>(const std::string& str) {return std::stoi(str, std::string::size_type());}
    template <>
    std::string ArgParser::from<int>(const int& v) {return std::to_string(v);}

    template <>
    uint32_t ArgParser::to<uint32_t>(const std::string& str) {return static_cast<uint32_t>(std::stoi(str, std::string::size_type()));}
    template <>
    std::string ArgParser::from<uint32_t>(const uint32_t& v) {return std::to_string(v);}

    template <>
    float ArgParser::to<float>(const std::string& str) {
        std::string::size_type sz;
        return std::stof(str, &sz);
    }
    template <>
    std::string ArgParser::from<float>(const float& v) {return std::to_string(v);}


}