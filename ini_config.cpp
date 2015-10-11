#include "ini_config.h"

IniConfig::IniConfig(const std::string& ini_file_name_)
    : ini_file_name(ini_file_name_) {
  ini_cfg.SetUnicode();
  ini_cfg.LoadFile(ini_file_name.c_str());
}

IniConfig::~IniConfig() {
  ini_cfg.SaveFile(ini_file_name.c_str());
}

std::string IniConfig::GetValue(const std::string& section, const std::string& key) {
  return ini_cfg.GetValue(section.c_str(), key.c_str(), "");
}

void IniConfig::SetValue(const std::string& section, const std::string& key, const std::string& value) {
  ini_cfg.SetValue(section.c_str(), key.c_str(), value.c_str());
}
