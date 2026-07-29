#include "gtest/gtest.h"
#include "state.h"
#include "property.h"

namespace native::ui {

TEST(PropertyTest, DefaultValue) {
  Property<int> prop(nullptr);
  EXPECT_EQ(prop.value(), 0);
}

TEST(PropertyTest, AssignAndRead) {
  Property<int> prop(nullptr);
  prop = 42;
  EXPECT_EQ(prop.value(), 42);
}

TEST(PropertyTest, ImplicitConversion) {
  Property<int> prop(nullptr);
  prop = 99;
  int val = prop;
  EXPECT_EQ(val, 99);
}

TEST(PropertyTest, MultipleAssignments) {
  Property<int> prop(nullptr);
  prop = 1;
  prop = 2;
  prop = 3;
  EXPECT_EQ(prop.value(), 3);
}

TEST(PropertyTest, StringProperty) {
  Property<std::string> prop(nullptr);
  prop = "hello";
  EXPECT_EQ(prop.value(), "hello");
}

TEST(PropertyTest, OnBeforeSetHook) {
  Property<int> prop(nullptr);
  int intercepted = 0;
  prop.OnBeforeSet([&](const int& val) { intercepted = val * 2; });
  prop = 10;
  EXPECT_EQ(intercepted, 20);
  EXPECT_EQ(prop.value(), 10);
}

TEST(PropertyTest, OnAfterSetHook) {
  Property<int> prop(nullptr);
  int logged = 0;
  prop.OnAfterSet([&](const int& val) { logged = val + 1; });
  prop = 7;
  EXPECT_EQ(logged, 8);
  EXPECT_EQ(prop.value(), 7);
}

TEST(PropertyTest, BothHooks) {
  Property<int> prop(nullptr);
  std::string log;
  prop.OnBeforeSet([&](const int& val) { log += "before:" + std::to_string(val) + " "; });
  prop.OnAfterSet([&](const int& val) { log += "after:" + std::to_string(val); });
  prop = 5;
  EXPECT_EQ(log, "before:5 after:5");
}

}  // namespace native::ui
