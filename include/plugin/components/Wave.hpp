#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "libs/Vector2D.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

enum class WavePatternType : std::uint8_t
{
  LINE,
};

inline const std::unordered_map<std::string, WavePatternType> WAVE_PATTERN_TYPE_MAP {
    {"line", WavePatternType::LINE},
};

inline WavePatternType parse_wave_pattern_type(const std::string& str)
{
  auto it = WAVE_PATTERN_TYPE_MAP.find(str);
  if (it != WAVE_PATTERN_TYPE_MAP.end()) {
    return it->second;
  }
  return WavePatternType::LINE;
}

struct WavePattern
{
  WavePattern() = default;

  WavePattern(WavePatternType t, JsonObject p)
      : type(t)
      , params(std::move(p))
  {
  }

  HOOKABLE(WavePattern, HOOK(type), HOOK(params))

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      WavePattern,
      ([](WavePatternType t, JsonObject p) { return WavePattern {t, std::move(p)}; }),
      parseByte<WavePatternType>(),
      parseByteJsonObject())

  DEFAULT_SERIALIZE(type_to_byte(type), json_object_to_byte(params))

  WavePatternType type;
  JsonObject params;
};

inline Parser<WavePattern> parseWavePattern()
{
  return apply(
      [](WavePatternType t, JsonObject p) { return WavePattern {t, std::move(p)}; },
      parseByte<WavePatternType>(),
      parseByteJsonObject());
}

struct Wave
{
  Wave() = default;

  Wave(std::string tmpl, int cnt, WavePattern pat)
      : entity_template(std::move(tmpl))
      , count(cnt)
      , pattern(std::move(pat))
      , spawned_entities()
      , active(false)
  {
  }

  Wave(std::string tmpl,
       int cnt,
       WavePattern pat,
       std::vector<std::size_t> spawned,
       bool act)
      : entity_template(std::move(tmpl))
      , count(cnt)
      , pattern(std::move(pat))
      , spawned_entities(std::move(spawned))
      , active(act)
  {
  }

  HOOKABLE(Wave,
           HOOK(entity_template),
           HOOK(count),
           HOOK(pattern),
           HOOK(spawned_entities),
           HOOK(active))

  CHANGE_ENTITY(result.spawned_entities = MAP_ENTITY_VECTOR(spawned_entities);)

  DEFAULT_BYTE_CONSTRUCTOR(
      Wave,
      ([](std::string tmpl,
          int cnt,
          WavePattern pat,
          std::vector<std::size_t> spawned,
          bool act)
       {
         return (Wave) {std::move(tmpl), cnt, std::move(pat), std::move(spawned), act};
       }),
      parseByteString(),
      parseByte<int>(),
      parseWavePattern(),
      parseByteArray(parseByte<std::size_t>()),
      parseByte<bool>())

  DEFAULT_SERIALIZE(string_to_byte(entity_template),
                    type_to_byte(count),
                    pattern.to_bytes(),
                    vector_to_byte(spawned_entities,
                                   TTB_FUNCTION<std::size_t>()),
                    type_to_byte(active))

  std::string entity_template;
  int count;
  WavePattern pattern;
  std::vector<Registry::Entity> spawned_entities;
  bool active;
};
