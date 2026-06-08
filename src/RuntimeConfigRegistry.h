/** @file
 * Runtime configuration registry used by persistence.
 */

#ifndef CTHUGHA_RUNTIME_CONFIG_REGISTRY_H
#define CTHUGHA_RUNTIME_CONFIG_REGISTRY_H

#include "Configuration.h"

#include <vector>

class AutoChangeSettings;
class AudioProcessingState;
class Option;

/**
 * Adds one runtime owner's current state to a Config snapshot.
 */
class RuntimeConfigContributor {
public:
    /** Destroys the contributor interface. */
    virtual ~RuntimeConfigContributor();

    /**
     * Overlays current runtime state onto a mutable Config snapshot.
     *
     * @param config Snapshot being prepared for persistence.
     */
    virtual void contribute(Config& config) const = 0;
};

/**
 * Builds current runtime Config snapshots from a startup baseline.
 */
class RuntimeConfigRegistry {
    Config baselineValue;
    std::vector<const RuntimeConfigContributor*> contributors;

public:
    /**
     * Creates a registry with the startup configuration as its baseline.
     *
     * @param baseline Startup Config used for values that are not runtime-owned.
     */
    explicit RuntimeConfigRegistry(const Config& baseline);

    /**
     * Replaces the startup baseline used for future currentConfig() calls.
     *
     * @param baseline New baseline Config.
     */
    void setBaseline(const Config& baseline);

    /**
     * Adds a contributor that can overlay live runtime state.
     *
     * @param contributor Contributor retained by reference; caller owns it.
     */
    void addContributor(const RuntimeConfigContributor& contributor);

    /**
     * Builds a Config snapshot suitable for persistence.
     *
     * @return Baseline Config after all contributors have overlaid live state.
     */
    Config currentConfig() const;
};

/** Transitional contributor for runtime values not yet split by subsystem. */
class LegacyRuntimeConfigContributor : public RuntimeConfigContributor {
    const AutoChangeSettings& autoChangeSettings;
    const AudioProcessingState& audioProcessingState;
    const Option& quietMessageOption;

public:
    /**
     * Creates a contributor backed by current runtime owners.
     *
     * @param autoChangeSettings_ Runtime-owned automatic scene-change settings.
     * @param audioProcessingState_ Runtime-owned audio processing state.
     * @param quietMessageOption_ Runtime quiet-message threshold option.
     */
    LegacyRuntimeConfigContributor(const AutoChangeSettings& autoChangeSettings_,
        const AudioProcessingState& audioProcessingState_,
        const Option& quietMessageOption_);

    /**
     * Overlays display, audio-processing, auto-change, and policy values that
     * have not moved to module serializers yet.
     *
     * @param config Snapshot being prepared for persistence.
     */
    virtual void contribute(Config& config) const;
};

#endif
