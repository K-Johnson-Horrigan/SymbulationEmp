#ifndef SGPSYMBIONT_H
#define SGPSYMBIONT_H

#include "../default_mode/Symbiont.h"
#include "CPU.h"
#include "SGPHost.h"
#include "SGPWorld.h"
#include "emp/Evolve/World_structure.hpp"

class SGPSymbiont : public Symbiont {
private:
  CPU cpu;
  const emp::Ptr<SGPWorld> my_world;

  /**
    * Purpose: to store records of this particular symbiont's behavior 
    * towards hosts (donation & stealing)
  */
  double behavior_points = 0;

public:
  /**
   * Constructs a new SGPSymbiont as an ancestor organism, with either a random
   * genome or a blank genome that knows how to do a simple task depending on
   * the config setting RANDOM_ANCESTOR.
   */
  SGPSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
              emp::Ptr<SymConfigBase> _config, double _intval = 0.0,
              double _points = 0.0)
      : Symbiont(_random, _world, _config, _intval, _points), cpu(this, _world),
        my_world(_world) {}

  /**
   * Constructs an SGPSymbiont with a copy of the provided genome.
   */
  SGPSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
              emp::Ptr<SymConfigBase> _config,
              const sgpl::Program<Spec> &genome, double _intval = 0.0,
              double _points = 0.0)
      : Symbiont(_random, _world, _config, _intval, _points),
        cpu(this, _world, genome), my_world(_world) {}

  SGPSymbiont(const SGPSymbiont &symbiont)
      : Symbiont(symbiont),
        cpu(this, symbiont.my_world, symbiont.cpu.GetProgram()),
        my_world(symbiont.my_world) {}

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Perform necessary cleanup when a symbiont dies, freeing
   * heap-allocated state and canceling any in-progress reproduction.
   */
  ~SGPSymbiont() {
    if (!my_host) {
      cpu.state.used_resources.Delete();
      cpu.state.shared_available_dependencies.Delete();
    }
    // Invalidate any in-progress reproduction
    if (cpu.state.in_progress_repro != -1) {
      my_world->to_reproduce[cpu.state.in_progress_repro].second =
          emp::WorldPosition::invalid_id;
    }
  }

  bool operator<(const Organism &other) const {
    if (const SGPSymbiont *sgp = dynamic_cast<const SGPSymbiont *>(&other)) {
      return cpu.GetProgram() < sgp->cpu.GetProgram();
    } else {
      return false;
    }
  }

  bool operator==(const Organism &other) const {
    if (const SGPSymbiont *sgp = dynamic_cast<const SGPSymbiont *>(&other)) {
      return cpu.GetProgram() == sgp->cpu.GetProgram();
    } else {
      return false;
    }
  }

  /**
   * Input: The pointer to an organism that will be set as the symbinot's host
   *
   * Output: None
   *
   * Purpose: To set a symbiont's host
   */
  void SetHost(emp::Ptr<Organism> host) {
    Symbiont::SetHost(host);
    if (my_host) {
      if (cpu.state.used_resources) { cpu.state.used_resources.Delete(); }
      if (cpu.state.shared_available_dependencies) {
        cpu.state.shared_available_dependencies.Delete(); }
      if (cpu.state.internalEnvironment) { cpu.state.internalEnvironment.Delete(); }

      cpu.state.used_resources =
        host.DynamicCast<SGPHost>()->GetCPU().state.used_resources;
      cpu.state.shared_available_dependencies =
        host.DynamicCast<SGPHost>()->GetCPU().state.shared_available_dependencies;
      cpu.state.internalEnvironment =
        host.DynamicCast<SGPHost>()->GetCPU().state.internalEnvironment;
    }
  }

  /**
   * Input: None
   *
   * Output: The CPU associated with this symbiont.
   *
   * Purpose: Allows accessing the symbiont's CPU.
   */
  CPU &GetCPU() { return cpu; }

  /**
   * Input: The number of points donated or stolen by the symbiont.
   *
   * Output: None.
   *
   * Purpose: To track the number of points donated or stolen by the symbiont; 
   * stolen points decrement the total, and donated points increment it.
   */
  void TrackBehaviorPoints(double _in) { behavior_points += _in; }
  
  /**
   * Input: None.
   *
   * Output: The number of points donated or stolen by the symbiont.
   *
   * Purpose: To get the number of points donated or stolen by the symbiont;
   * stolen points decrement the total, and donated points increment it.
   */
  double GetBehaviorVal() { return behavior_points; }

  /**
   * Input: The index of the ecto host to be linked with.
   *
   * Output: None.
   *
   * Purpose: Sets the organism's internal environment up for ectosymbiosis
   */
  void LinkEctoHost(int pos) {
    emp::Ptr<Organism> host = my_world->GetPop()[pos];

    cpu.state.used_resources =
      host.DynamicCast<SGPHost>()->GetCPU().state.used_resources;
    cpu.state.shared_available_dependencies =
      host.DynamicCast<SGPHost>()->GetCPU().state.shared_available_dependencies;
    cpu.state.internalEnvironment =
      host.DynamicCast<SGPHost>()->GetCPU().state.internalEnvironment;
  }

  /**
   * Input: The location of the symbiont, which includes the symbiont's position
   * in the host (default -1 if it doesn't have a host)
   *
   * Output: None
   *
   * Purpose: To process a symbiont, meaning running its program code, which
   * can include reproduction and acquisition of resources; and to allow for
   * movement
   */
  void Process(emp::WorldPosition pos) {
    behavior_points = 0;
    if (my_host == nullptr && my_world->GetUpdate() % my_config->LIMITED_TASK_RESET_INTERVAL() == 0)
      cpu.state.used_resources->reset();
    // Instead of calling Host::Process, do the important stuff here
    // Our instruction handles reproduction
    if (GetDead()) {
      return;
    }

    cpu.RunCPUStep(pos, my_config->CYCLES_PER_UPDATE());
    // The parts of Symbiont::Process that don't use resources or reproduction

    // Age the organism
    GrowOlder();
  }

  /**
   * Input: The pointer to the organism that is the new host baby
   *
   * Output: None
   *
   * Purpose: To allow for vertical transmission to occur. This performs extra
   * bookkeeping on top of `Symbiont::VerticalTransmission()` to avoid messing
   * with the reproduction queue which is used for horizontal transmission.
   */
  void VerticalTransmission(emp::Ptr<Organism> host_baby) {
    // Save and restore the in-progress reproduction, since Reproduce() will be
    // called but it will still be on the queue for horizontal transmission
    size_t old = cpu.state.in_progress_repro;
    Symbiont::VerticalTransmission(host_baby);
    cpu.state.in_progress_repro = old;
  }

  /**
   * Input: None
   *
   * Output: The pointer to the newly created organism
   *
   * Purpose: To produce a new symbiont, identical to the original
   */
  emp::Ptr<Organism> MakeNew() {
    emp::Ptr<SGPSymbiont> sym_baby = emp::NewPtr<SGPSymbiont>(
        random, my_world, my_config, cpu.GetProgram(), GetIntVal());
    // This organism is reproducing, so it must have gotten off the queue
    cpu.state.in_progress_repro = -1;
    return sym_baby;
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To mutate the code in the genome of this symbiont.
   */
  void Mutate() {
    Symbiont::Mutate();

    cpu.Mutate();
  }
};

#endif