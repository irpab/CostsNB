#ifndef INI_CONFIG_H
#define INI_CONFIG_H

#include <string>

#include "SimpleIni.h"

#include "costs_nb_core.h"

class IniConfig : public AbstractConfig {
public:
  IniConfig(const std::string& ini_file_name);
  ~IniConfig();

  std::string GetValue(const std::string& section, const std::string& key);
  void SetValue(const std::string& section, const std::string& key, const std::string& value);
private:
  CSimpleIniA ini_cfg;
  std::string ini_file_name;
};

#endif
