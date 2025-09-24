#include "engine.h"
#include "abstract_set.h"
#include <cstddef>
#include <functional>
#include <vector>
//                            ▲
// descend into   │           │ ascend out
//                │           │
//                ▼
//               ┌─────────────┐
//               │             │
//               │    State    │
//               │             │
//               └─────────────┘
//                            ▲
//                │           │ ascend into
//  descend out   │           │
//                ▼

void Engine::run() {
    std::vector<std::reference_wrapper<const AbstractSet>> ncap_input_buffer;


    std::size_t pc = 0;

    // descend into
    std::vector<std::reference_wrapper<const AbstractSet>> sets;

    State& current_state = states[pc];

    for (std::uint8_t reg : current_state.indices) {
        auto index = indices[reg];

        ncap_input_buffer.push_back(index.project());
    }

    intersect(current_state.candidates, ncap_input_buffer);
}
