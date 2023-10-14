#pragma once

#include "tools/Serializer.h"

namespace GF {
namespace DiscreteModel {

struct TransitionSystemModelProperties {
    virtual bool isEqual(const TransitionSystemModelProperties&) const = 0;
    virtual void serialize(GF::Serializer&) const = 0;
    virtual void deserialize(const GF::Deserializer&) = 0;
    virtual ~TransitionSystemModelProperties() {}
};

}
}