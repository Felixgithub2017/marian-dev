#include "catch.hpp"

#include "functional/tensor.h"
#include <vector>
#include <algorithm>

using namespace marian::functional;

TEST_CASE("View tests (cpu)", "[operator]") {
  SECTION("slices") {

    std::vector<float> v(2*2*5);
    std::iota(v.begin(), v.end(), 0);

    View<float, 3> out(v.data(), {{2, 2, 5}});

    std::vector<float> vt;
    for(int i = 0; i < out.size(); ++i)
      vt.push_back(out[i]);

    CHECK( v == vt );

    std::cerr << out.debug() << std::endl;
    std::cerr << out[{0, 0, 3}] << std::endl;
    std::cerr << out[{0, 1, 3}] << std::endl;

    float o1 = out[{0, 0, 3}],
    o2 = out[{0, 1, 4}],
    o3 = out[{1, 1, 1}];

    CHECK( o1 ==  3 );
    CHECK( o2 ==  9 );
    CHECK( o3 == 16 );

    int ind  = out.shape().index({0, 1, 3});
    float o4 = out[{0, 1, 3}];

    CHECK( o4 == out[ind] );
    CHECK( o4 == 8 );

    std::cerr << out[{0, 1, 3}] << " == " << out[ind] << " ind: " << ind << std::endl;

    auto s1 = slice(out, {1}, {1}, {1, 5, 2}).shape();
    ConstantShape<3> sm1({1, 1, 2});

    std::cerr <<  s1 << std::endl;
    std::cerr << sm1 << std::endl;

    CHECK( s1.shape_ == sm1.shape_ );

    auto out2 = slice(out, {}, {1}, {1, 5, 2});

    CHECK( out2.size() == 4 );

    std::cerr << out2.debug() << std::endl;

    float o5 = out2[{0, 0, 0}];
    float o6 = out2[{1, 0, 1}];

    CHECK( o5 ==  6 );
    CHECK( o6 == 18 );
    CHECK( out2[0] == o5 );
    CHECK( out2[3] == o6 );

    auto outs1 = slice(out, {1}, {}, {});
    std::cerr << outs1.debug() << std::endl;

    auto outs2 = slice(outs1, {}, {}, {2, 5});
    std::cerr << outs2.debug() << std::endl;

    auto outs3 = slice(outs2, {0}, {1}, {1});
    std::cerr << outs3.debug() << std::endl;

    CHECK( outs3.shape().size() == 3 );

    outs2[2] = 1;
    outs2[4] = 4;

    float v5 = outs2[{0,0,2}] + outs2[{0,1,1}];

    CHECK( v5 == 5 );

    std::cerr << outs2.debug() << std::endl;
    std::cerr << out.debug() << std::endl;

  }
}