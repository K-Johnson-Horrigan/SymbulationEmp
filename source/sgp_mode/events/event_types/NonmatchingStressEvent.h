#pragma once

#include "Event.h"

namespace sgpmode {

/**
 * Purpose: Non-matching stress event class. Hosts are killed to a certain number, without regard for their symbiont trait-matching status. 
 */
class NonmatchingStressEvent : public Event {
public:
  using json_t = nlohmann::json;

  /**
   * Purpose: Event specification object shared by all instances of this event
   *          type.
   */
  static const EventTypeDefinitionSpecs event_specs;

  /**
   * Purpose: All event type classes should define a "LoadEventFromJSON" function
   *          that configures an instance of this event type.
   *          See TaskValueEvent for another example.
   *
   * Input: Json object to load event information from, reference to the world.
   *
   * Output: Pointer to new event instance created and configured by this load
   *         function.
   */
  template <typename WORLD_T>
  static emp::Ptr<NonmatchingStressEvent> LoadEventFromJSON(
    json_t& event_json,
    WORLD_T& world
  ) {
    // This should be the correct event type
    emp_assert(event_json["event_type"] == event_specs.event_type);
    // NOTE: Caller is responsible for event deletion.
    emp::Ptr<NonmatchingStressEvent> event = emp::NewPtr<NonmatchingStressEvent>();
    
    // Get event parameters out of json

    // --- Set the maximum number of hosts to allow ---
    event->target_host_count = sym_json::GetVal<size_t>(event_json, "target_host_count");

    // --- Set the timing ---
    SetEventTimingFromJSON(event_json, event, world);

    return event;
  }

protected:
  // The maximum number of hosts we want to leave alive after this event
  size_t target_host_count; 

public:
  /**
   * Purpose: Constructor. Set the base class's event_type based on this event
   *          type's event specs.
   */
  NonmatchingStressEvent() {
    // Set event type in base class.
    event_type = event_specs.event_type;
  }

  /**
   * Purpose: Process this event. Called by the event manager when this event should
   *          be triggered based on its timing.
   *
   * Input: Reference to the world (will likely be modified by the event)
   *
   * Output: None.
   */
  template<typename WORLD_T>
  void Process(WORLD_T& world) {
    // if the world has more hosts than desired, randomly kill hosts until target population size is reached
    if (world.GetNumOrgs() > target_host_count) {
      size_t to_make_dead_count = world.GetNumOrgs() - target_host_count;
      emp::vector<size_t> occupied_cells = world.GetValidOrgIDs();
      emp::vector<size_t> schedule = emp::GetPermutation(world.GetRandom(), occupied_cells.size());
      for (size_t i = 0; i < to_make_dead_count; i++) {
        world.DoDeath(occupied_cells[schedule[i]]);
      }
    }
  }
};

// Every event type needs to have its event specs defined.
const EventTypeDefinitionSpecs NonmatchingStressEvent::event_specs = EventTypeDefinitionSpecs{
  "nonmatching_stress_event",
  "Run a non-matching stress event, killing hosts to a certain population size without regard for symbiont trait-matching.",
  {"target_host_count", "timing"}
};

}