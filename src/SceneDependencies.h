// Explicit collaborators used by SceneCommands while legacy catalogs migrate.

#ifndef CTHUGHA_SCENE_DEPENDENCIES_H
#define CTHUGHA_SCENE_DEPENDENCIES_H

#include "Wave.h"

class EffectControl;
class RandomSource;

/**
 * Provides the currently selected wave object for scene wave configuration.
 */
class SceneWaveObjectSource {
public:
    virtual ~SceneWaveObjectSource();
    virtual WObject* currentObject() = 0;
};

/**
 * Registry operations over scene-editable effect controls.
 *
 * The Scene module uses this as an explicit port until EffectControl registry
 * state is owned by a real EffectRegistry instead of the legacy static list.
 */
class SceneEffectRegistry {
public:
    virtual ~SceneEffectRegistry();
    virtual void saveAll() = 0;
    virtual void restoreAll() = 0;
    virtual void changeAll(RandomSource& randomSource) = 0;
    virtual EffectControl* changeOne(RandomSource& randomSource) = 0;
};

/**
 * Palette catalog operations used by palette randomization commands.
 */
class ScenePaletteRandomizer {
public:
    virtual ~ScenePaletteRandomizer();
    virtual int randomizeLast(RandomSource& randomSource) = 0;
    virtual int addRandom(RandomSource& randomSource) = 0;
};

/**
 * Compatibility adapter for the legacy global wave object catalog.
 */
class LegacySceneWaveObjectSource : public SceneWaveObjectSource {
public:
    virtual WObject* currentObject();
};

/**
 * Compatibility adapter for the legacy EffectControl static registry.
 */
class LegacySceneEffectRegistry : public SceneEffectRegistry {
public:
    virtual void saveAll();
    virtual void restoreAll();
    virtual void changeAll(RandomSource& randomSource);
    virtual EffectControl* changeOne(RandomSource& randomSource);
};

/**
 * Compatibility adapter for the legacy palette-entry static catalog commands.
 */
class LegacyScenePaletteRandomizer : public ScenePaletteRandomizer {
public:
    virtual int randomizeLast(RandomSource& randomSource);
    virtual int addRandom(RandomSource& randomSource);
};

#endif
