// Compile with a game's configured CodeWarrior command and RSTL_VERSION.
// Explicitly instantiate inherited APIs that ordinary game builds may not use.
#include "rstl/algorithm.hpp"
#include "rstl/multimap.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

struct RstlApiBase {
  virtual ~RstlApiBase() {}
};
struct RstlApiDerived : RstlApiBase {};

int RstlInheritedAlgorithms(int* first, int* last, int* other) {
  rstl::pair< int*, int* > result = rstl::mismatch(first, last, other);
#if RSTL_CONSTRUCTION_POLICY
  rstl::pair< int, int > values = rstl::make_pair(1, 2);
  return int(result.first - first) + values.first;
#else
  return int(result.first - first);
#endif
}

int RstlInheritedContainers(const int* first, const int* last) {
  rstl::reserved_vector< int, 8 > values(first, last);
  values.set_size(0);
  rstl::multimap< int, int > map;
  map.insert(rstl::pair< int, int >(1, 2));
  rstl::multimap< int, int >::iterator it = map.begin();
  return map.count(it->first) + map.empty() + values.size();
}

void RstlInheritedPointers() {
  rstl::single_ptr< int > single;
  single.reset(new int(1));
#if RSTL_RC_PTR_PRIVATE_COUNT
  rstl::rc_ptr< RstlApiDerived > derived(new RstlApiDerived);
  rstl::rc_ptr< RstlApiBase > base = derived.Cast< RstlApiBase >();
  rstl::ncrc_ptr< RstlApiBase > converted(derived);
#endif
}
