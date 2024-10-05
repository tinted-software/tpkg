import std;
import tpkg.derivation;
import tpkg.builder;

int main() {
  std::vector<tpkg::TaskStatus> tasks = {
      tpkg::TaskStatus{tpkg::Derivation{"llvm", {"nu", "-c", "sleep 5sec"}}},
  };

  tpkg::build_derivations(tasks);

  return 0;
}
