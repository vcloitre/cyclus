
#include <gtest/gtest.h>

#include "toolkit/matl_sell_policy.h"

#include "composition.h"
#include "material.h"
#include "bid.h"
#include "request.h"
#include "error.h"
#include "pyne.h"
#include "toolkit/resource_buff.h"

#include "test_context.h"
#include "test_agents/test_facility.h"

namespace cyclus {
namespace toolkit {

class TestComp : public Composition {
 public:
  TestComp() {}
};

class MatlSellPolicyTests: public ::testing::Test {
 protected:
  TestContext tc;
  TestFacility* fac1;
  double cap, qty;
  ResourceBuff buff;
  Composition::Ptr comp, comp1;
  Material::Ptr mat, mat1;
  
  virtual void SetUp() {
    fac1 = new TestFacility(tc.get());
    cap = 5;
    qty = 3;
    buff.set_capacity(cap);
    CompMap v;
    v[pyne::nucname::id("H1")] = 1;
    comp = Composition::CreateFromAtom(v);
    mat = Material::CreateUntracked(qty, comp);
    buff.Push(mat);
    comp1 = Composition::CreateFromAtom(v);
    mat1 = Material::CreateUntracked(qty, comp1);
  }

  virtual void TearDown() {
    delete fac1;
  }
};


TEST_F(MatlSellPolicyTests, Init) {
  MatlSellPolicy p;

  // defaults
  p.Init(fac1, &buff, "");
  ASSERT_FALSE(p.Excl());
  ASSERT_FLOAT_EQ(p.Limit(), qty);

  // limit & excl
  p.Init(fac1, &buff, "", 1.5, false, 1);
  ASSERT_TRUE(p.Excl());
  ASSERT_FLOAT_EQ(p.Limit(), 1.5);

  // throughput = 2.5, quantize = 2, limit = 2
  p.Init(fac1, &buff, "", qty - 0.5, false, qty - 1);
  ASSERT_FLOAT_EQ(p.Limit(), qty - 1);
}

TEST_F(MatlSellPolicyTests, StartStop) {
  MatlSellPolicy p;
  p.Init(NULL, &buff, "");
  ASSERT_THROW(p.Start(), ValueError);
  ASSERT_THROW(p.Stop(), ValueError);
}

TEST_F(MatlSellPolicyTests, Bids) {
  MatlSellPolicy p;
  std::string commod("commod");  
  CommodMap<Material>::type reqs;
  reqs[commod] = std::vector<Request<Material>*>();
  Request<Material>* req = Request<Material>::Create(mat1, fac1, commod);
  reqs[commod].push_back(req);
  std::set<BidPortfolio<Material>::Ptr> obs;

  // basic
  p.Init(NULL, &buff, "").Set(commod);
  obs = p.GetMatlBids(reqs);
  ASSERT_EQ(obs.size(), 1);
  ASSERT_EQ((*obs.begin())->bids().size(), 1);
  ASSERT_FLOAT_EQ((*(*obs.begin())->bids().begin())->offer()->quantity(),
                  mat->quantity());
  ASSERT_EQ((*(*obs.begin())->bids().begin())->offer()->comp(), comp);

  // excl and ignore_comp
  p.Init(NULL, &buff, "", qty, true, qty / 2).Set(commod);
  obs = p.GetMatlBids(reqs);
  ASSERT_EQ(obs.size(), 1);
  ASSERT_EQ((*obs.begin())->bids().size(), 2);
  ASSERT_FLOAT_EQ((*(*obs.begin())->bids().begin())->offer()->quantity(),
                  mat->quantity() / 2);
  ASSERT_EQ((*(*obs.begin())->bids().begin())->offer()->comp(), comp1);
  
  delete req;
}

TEST_F(MatlSellPolicyTests, Trades) {
  MatlSellPolicy p;
  std::string commod("commod");  
  std::vector<Trade<Material> > trades;
  std::vector<std::pair<Trade<Material>, Material::Ptr> > obs;
  
  Request<Material>* req = Request<Material>::Create(mat1, fac1, commod);
  Bid<Material>* bid = Bid<Material>::Create(req, mat, fac1);
  Trade<Material> trade(req, bid, 1);
  trades.push_back(trade);
  
  // basic
  p.Init(NULL, &buff, "").Set(commod);
  p.GetMatlTrades(trades, obs);
  ASSERT_EQ(obs.size(), 1);
  ASSERT_EQ(obs.begin()->second->comp(), comp);
  
  // ignore comp
  obs.clear();
  p.Init(NULL, &buff, "", qty, true).Set(commod);
  p.GetMatlTrades(trades, obs);
  ASSERT_EQ(obs.size(), 1);
  ASSERT_EQ(obs.begin()->second->comp(), comp1);

  delete bid;
  delete req;
}

}
}
