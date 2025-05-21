#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string_view>
#include <variant>

#include <yaml-cpp/yaml.h>

namespace Smanager
{
using namespace std;

class Config
{
public:
  explicit Config(std::string path)
      : path {path}
  {
    config = YAML::LoadFile(path);
    if (!config.IsDefined()) {
      cerr << "cant load file\n";
      return;
    }
  }

  std::map<std::string, int> processSettings()
  {
    std::map<std::string, int> map_out {};
    auto settings = config["settings"];
    for (YAML::const_iterator it = settings.begin(); it != settings.end(); ++it)
    {
      YAML::Node key = it->first;
      YAML::Node value = it->second;
      try {
        auto val = value.as<int>();
        std::cout << "key: " << key.as<string>() << " value " << value.as<int>()
                  << "\n";
        map_out[key.as<string>()] = val;
      } catch (const YAML::TypedBadConversion<int>& ex) {
        continue;
      }
    }
    return map_out;
  }
  /*
  std::map<std::string, std::variant<int, std::string>> processSettings()
  {
    std::map<std::string, std::variant<int, std::string>> map_out {};
    auto settings = config["settings"];
    for (YAML::const_iterator it = settings.begin(); it != settings.end(); ++it)
    {
      YAML::Node key = it->first;
      YAML::Node value = it->second;
      try {
        std::cout << "key: " << key.as<string>() << " value "
                  << value.as<string>() << "\n";
        map_out[key.as<string>()] = value.as<string>();
      } catch (const YAML::TypedBadConversion<string>& ex) {
        try {
          auto val = value.as<int>();
          std::cout << "key: " << key.as<int>() << " value " << value.as<int>()
                    << "\n";
          map_out[key.as<string>()] = val;
        } catch (const YAML::TypedBadConversion<int>& ex) {
          continue;
        }
      }
    }
    return map_out;
  }
  */
  void writeSettings(string_view field,
                     std::map<string, variant<string, int32_t>>& mSettings)
  {
    auto settings = config[field];

    auto s_name = ""s;
    for (auto it = settings.begin(); it != settings.end(); ++it) {
      YAML::Node key = it->first;
      YAML::Node value = it->second;
      try {
        s_name = std::move(key.as<string>());
      } catch (const YAML::TypedBadConversion<string>& ex) {
        cout << "error key reading: " << ex.what() << std::endl;
        continue;
      }
      // a value mush me either string or int
      auto a_setting = mSettings.find(s_name);
      if (a_setting == mSettings.end())
        continue;
      try {
        // get current value
        auto val = value.as<int>();
        // setting variant
        auto& setting_variant = a_setting->second;
        if (auto* pval = std::get_if<int32_t>(&setting_variant)) {
          value = *pval;
          continue;
        }
        if (auto* pval = std::get_if<std::string>(&setting_variant)) {
          value = *pval;
        }
      } catch (const YAML::TypedBadConversion<int>& ex) {
        // try string then...
        try {
          auto val = value.as<std::string>();
          auto& setting_variant = a_setting->second;
          if (auto* pval = std::get_if<std::string>(&setting_variant)) {
            value = *pval;
          }
          if (auto* pval = std::get_if<int32_t>(&setting_variant)) {
            value = *pval;
            continue;
          }
        } catch (const YAML::TypedBadConversion<string>& ex) {
          cerr << "error value reading value : " << ex.what() << std::endl;
        }
        continue;
      }
    }
    std::ofstream fout(path);
    fout << config;
  }

private:
  std::string path;
  YAML::Node config;
};
/*
int main(int argc, const char *argv[]) {
  auto cfgfile = "config.yaml"s;
  if (!filesystem::exists(cfgfile)) {
    cerr << "config file not exists!\n";
    return EXIT_FAILURE;
  }
  Config cfg1(cfgfile);
  if (std::string(argv[1]) == "--write") {

    std::map<string, variant<string, int32_t>> test{};
    test["setting1"] = 2511;
    test["setting2"] = 12;
    test["setting3"] = 27;
    test["setting4"] = "xtestval"s;
    test["setting5"] = "val"s;
    cfg1.writeSettings("settings", test);
  } else if (std::string(argv[1]) == "--read") {
    cfg1.processSettings();
    return EXIT_SUCCESS;
  }
  return EXIT_SUCCESS;
}
*/
}  // namespace Smanager
