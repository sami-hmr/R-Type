#include <optional>

#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

enum ActivationEffect : std::uint8_t
{
  NOACTIVATION,
  CONSUMPTHROW,
  CONSUMPTION,
  THROW
};

template<typename T>
struct TemporalEffect
{
  std::optional<Registry::Entity> possessor;
  ActivationEffect activate_on;
  double effective_time;
  std::string name;
  bool consumable;
  bool throwable;
  double points;

  TemporalEffect(std::optional<Registry::Entity> possessor,
                 ActivationEffect activate_on,
                 double effective_time,
                 std::string name,
                 bool consumable,
                 bool throwable,
                 double points)
      : possessor(possessor)
      , activate_on(activate_on)
      , effective_time(effective_time)
      , name(std::move(name))
      , consumable(consumable)
      , throwable(throwable)
      , points(points)
  {
  }

  TemporalEffect(std::optional<Registry::Entity> possessor,
                 double effective_time,
                 std::string name,
                 double points)
      : possessor(possessor)
      , activate_on(CONSUMPTHROW)
      , effective_time(effective_time)
      , name(std::move(name))
      , consumable(true)
      , throwable(true)
      , points(points)
  {
  }

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(TemporalEffect<T>,
           HOOK(possessor),
           HOOK(activate_on),
           HOOK(effective_time),
           HOOK(name),
           HOOK(consumable),
           HOOK(throwable),
           HOOK(points))

  DEFAULT_BYTE_CONSTRUCTOR(TemporalEffect,
                           (
                               [](std::optional<Registry::Entity> possessor,
                                  ActivationEffect activate_on,
                                  double effective_time,
                                  std::string name,
                                  bool consumable,
                                  bool throwable,
                                  double points)
                               {
                                 return TemporalEffect(possessor,
                                                       activate_on,
                                                       effective_time,
                                                       name,
                                                       consumable,
                                                       throwable,
                                                       points);
                               }),
                           parseByteOptional(parseByte<Registry::Entity>()),
                           parseByte<ActivationEffect>(),
                           parseByte<double>(),
                           parseByteString(),
                           parseByte<bool>(),
                           parseByte<bool>(),
                           parseByte<double>())

  DEFAULT_SERIALIZE(optional_to_byte(this->possessor,
                                     std::function<ByteArray(Registry::Entity)>(
                                         [](Registry::Entity b)
                                         { return type_to_byte(b); })),
                    type_to_byte(this->activate_on),
                    type_to_byte(this->effective_time),
                    string_to_byte(this->name),
                    type_to_byte(this->consumable),
                    type_to_byte(this->throwable),
                    type_to_byte(this->points))
};

struct Revitalizing
{
};

struct Poisonous
{
};

struct Speed
{
};

using HealArtefact = TemporalEffect<Revitalizing>;
using PoisonArtefact = TemporalEffect<Poisonous>;
using SpeedArtefact = TemporalEffect<Speed>;
