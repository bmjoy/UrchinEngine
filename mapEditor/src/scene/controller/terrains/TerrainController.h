#ifndef URCHINENGINE_MAPEDITOR_TERRAINCONTROLLER_H
#define URCHINENGINE_MAPEDITOR_TERRAINCONTROLLER_H

#include <list>
#include "UrchinCommon.h"
#include "UrchinMapHandler.h"
#include "Urchin3dEngine.h"

namespace urchin
{

    class TerrainController
    {
        public:
            explicit TerrainController(MapHandler *);

            bool isModified() const;
            void resetModified();

            std::list<const SceneTerrain *> getSceneTerrains() const;
            void addSceneTerrain(SceneTerrain *);
            void removeSceneTerrain(const SceneTerrain *);

            const SceneTerrain *updateSceneTerrainGeneralProperties(const SceneTerrain *, const Point3<float> &, float);
            const SceneTerrain *updateSceneTerrainMesh(const SceneTerrain *, float, float);
            const SceneTerrain *updateSceneTerrainMaterial(const SceneTerrain *, float, float, std::string, const std::vector<std::string> &);
            const SceneTerrain *updateSceneTerrainGrass(const SceneTerrain *, std::string, std::string, unsigned int, float, float, float, const Vector3<float> &, float);

        private:
            void markModified();
            SceneTerrain *findSceneTerrain(const SceneTerrain *);

            bool bIsModified;
            MapHandler *mapHandler;
    };

}

#endif
