#include <config/definition.hpp>

#include <catch2/catch.hpp>

using namespace llarp::config;

TEST_CASE("ConfigDefinition simple generate test", "[config]")
{
  llarp::ConfigDefinition config{true};

  config.defineOption<int>("foo", "bar", Required);
  config.defineOption<int>("foo", "baz", Default{2});
  config.defineOption(
      std::make_unique<llarp::OptionDefinition<std::string>>("foo", "quux", Default{"hello"}));

  config.defineOption<int>("argle", "bar", RelayOnly, Required);
  config.defineOption<int>("argle", "baz", Default{4});
  config.defineOption<std::string>("argle", "quux", Default{"the quick brown fox"});

  config.defineOption<int>("not", "for-routers", ClientOnly, Default{1});

  std::string output = config.generateINIConfig();

  CHECK(output == R"raw([foo]


#bar=

#baz=2

#quux=hello


[argle]


#bar=

#baz=4

#quux=the quick brown fox
)raw");
}

TEST_CASE("ConfigDefinition useValue test", "[config]")
{
  llarp::ConfigDefinition config{true};

  config.defineOption<int>("foo", "bar", Default{1});

  constexpr auto expected = R"raw([foo]


#bar=1
)raw";

  CHECK(config.generateINIConfig(false) == expected);
  CHECK(config.generateINIConfig(true) == expected);

  config.addConfigValue("foo", "bar", "2");

  constexpr auto expectedWhenValueProvided = R"raw([foo]


bar=2
)raw";

  CHECK(config.generateINIConfig(false) == expected);
  CHECK(config.generateINIConfig(true) == expectedWhenValueProvided);
}

TEST_CASE("ConfigDefinition section comments test")
{
  llarp::ConfigDefinition config{true};

  config.addSectionComments("foo", {"test comment"});
  config.addSectionComments("foo", {"test comment 2"});
  config.defineOption(std::make_unique<llarp::OptionDefinition<int>>("foo", "bar", Default{1}));

  std::string output = config.generateINIConfig();

  CHECK(output == R"raw([foo]
# test comment
# test comment 2


#bar=1
)raw");
}

TEST_CASE("ConfigDefinition option comments test")
{
  llarp::ConfigDefinition config{true};

  config.addOptionComments("foo", "bar", {"test comment 1"});
  config.addOptionComments("foo", "bar", {"test comment 2"});
  config.defineOption<int>("foo", "bar", Default{1});

  config.defineOption<std::string>(
      "foo",
      "far",
      Default{"abc"},
      Comment{
          "Fill in the missing values:",
          "___defg",
      });

  config.defineOption<int>("client", "omg", ClientOnly, Default{1}, Comment{"hi"});
  config.defineOption<int>("relay", "ftw", RelayOnly, Default{1}, Comment{"bye"});

  // has comment, but is hidden: we show only the comment but not the value/default.
  config.defineOption<int>("foo", "old-bar", Hidden, Default{456});
  config.addOptionComments("foo", "old-bar", {"old bar option"});

  // no comment, should be omitted.
  config.defineOption<int>("foo", "older-bar", Hidden);

  std::string output = config.generateINIConfig();

  CHECK(output == R"raw([foo]


# test comment 1
# test comment 2
#bar=1

# Fill in the missing values:
# ___defg
#far=abc

# old bar option


[relay]


# bye
#ftw=1
)raw");
}

TEST_CASE("ConfigDefinition empty comments test")
{
  llarp::ConfigDefinition config{true};

  config.addSectionComments("foo", {"section comment"});
  config.addSectionComments("foo", {""});

  config.addOptionComments("foo", "bar", {"option comment"});
  config.addOptionComments("foo", "bar", {""});
  config.defineOption<int>("foo", "bar", Default{1});

  std::string output = config.generateINIConfig();

  CHECK(output == R"raw([foo]
# section comment
# 


# option comment
# 
#bar=1
)raw");
}

TEST_CASE("ConfigDefinition multi option comments")
{
  llarp::ConfigDefinition config{true};

  config.addSectionComments("foo", {"foo section comment"});

  config.addOptionComments("foo", "bar", {"foo bar option comment"});
  config.defineOption<int>("foo", "bar", Default{1});

  config.addOptionComments("foo", "baz", {"foo baz option comment"});
  config.defineOption<int>("foo", "baz", Default{1});

  std::string output = config.generateINIConfig();

  CHECK(output == R"raw([foo]
# foo section comment


# foo bar option comment
#bar=1

# foo baz option comment
#baz=1
)raw");
}
