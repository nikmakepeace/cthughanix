// Native Scene choice catalogs for fixed built-in visual selections.

#ifndef CTHUGHA_SCENE_BUILT_IN_CHOICE_CATALOGS_H
#define CTHUGHA_SCENE_BUILT_IN_CHOICE_CATALOGS_H

class SceneChoiceCatalog;
class SceneChoiceLock;

/**
 * Creates the fixed wave-scale Scene choice catalog.
 *
 * The returned catalog owns its choices and the supplied lock.
 *
 * @param catalogName Stable Scene catalog name.
 * @param lock Owned lock state for this selection.
 * @return Owned catalog with scale0, scale1, and scale2 choices.
 */
SceneChoiceCatalog* createSceneWaveScaleChoiceCatalog(
    const char* catalogName, SceneChoiceLock* lock);

/**
 * Creates the fixed table Scene choice catalog.
 *
 * @param catalogName Stable Scene catalog name.
 * @param lock Owned lock state for this selection.
 * @return Owned catalog with table0 through table9 choices.
 */
SceneChoiceCatalog* createSceneTableChoiceCatalog(
    const char* catalogName, SceneChoiceLock* lock);

/**
 * Creates the fixed border Scene choice catalog.
 *
 * @param catalogName Stable Scene catalog name.
 * @param lock Owned lock state for this selection.
 * @return Owned catalog with border0 through border3 choices.
 */
SceneChoiceCatalog* createSceneBorderChoiceCatalog(
    const char* catalogName, SceneChoiceLock* lock);

/**
 * Creates the fixed flashlight Scene choice catalog.
 *
 * @param catalogName Stable Scene catalog name.
 * @param lock Owned lock state for this selection.
 * @return Owned catalog with off/on choices and yes/no/0/1 aliases.
 */
SceneChoiceCatalog* createSceneFlashlightChoiceCatalog(
    const char* catalogName, SceneChoiceLock* lock);

#endif
