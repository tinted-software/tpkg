module;

export module tpkg.derivation;

import std;

export namespace tpkg {

class Derivation {
public:
  Derivation(const std::string &name, const std::vector<std::string> &builder)
      : name(name), builder(builder) {}
  std::string name;
  std::vector<std::string> builder;
};

} // namespace tpkg
