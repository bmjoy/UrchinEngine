#include <stdexcept>
#include <random>
#include <stack>
#include <cassert>
#include <thread>
#include <algorithm>
#include <functional>
#include <mutex>

#include "TerrainGrass.h"
#include "resources/MediaManager.h"
#include "utils/shader/ShaderManager.h"

#define DEFAULT_NUM_GRASS_IN_TEX 1
#define DEFAULT_GRASS_HEIGHT 1.0
#define DEFAULT_GRASS_LENGTH 1.0
#define DEFAULT_GRASS_QUANTITY 0.1
#define DEFAULT_WIND_DIRECTION Vector3<float>(1.0f, 0.0f, 0.0f)
#define DEFAULT_WIND_STRENGTH 1.0

namespace urchin
{

    TerrainGrass::TerrainGrass(const std::string &grassTextureFilename) :
            grassPositionRandomPercentage(ConfigService::instance()->getFloatValue("terrain.grassPositionRandomPercentage")),
            grassPatchSize(ConfigService::instance()->getFloatValue("terrain.grassPatchSize")),
            grassQuadtreeDepth(ConfigService::instance()->getUnsignedIntValue("terrain.grassQuadtreeDepth")),
            sumTimeStep(0.0f),
            grassTexture(nullptr),
            grassMaskTexture(nullptr),
            mainGrassQuadtree(nullptr),
            grassDisplayDistance(0.0f)
    {
        std::map<std::string, std::string> tokens;
        tokens["GRASS_ALPHA_TEST"] = ConfigService::instance()->getStringValue("terrain.grassAlphaTest");
        terrainGrassShader = ShaderManager::instance()->createProgram("terrainGrass.vert", "terrainGrass.frag", tokens);
        ShaderManager::instance()->setGeometryShader(terrainGrassShader, "terrainGrass.geo");
        ShaderManager::instance()->bind(terrainGrassShader);

        mProjectionLoc = glGetUniformLocation(terrainGrassShader, "mProjection");
        mViewLoc = glGetUniformLocation(terrainGrassShader, "mView");
        cameraPositionLoc = glGetUniformLocation(terrainGrassShader, "cameraPosition");
        sumTimeStepLoc = glGetUniformLocation(terrainGrassShader, "sumTimeStep");

        terrainMinPointLoc = glGetUniformLocation(terrainGrassShader, "terrainMinPoint");
        terrainMaxPointLoc = glGetUniformLocation(terrainGrassShader, "terrainMaxPoint");
        terrainAmbientLoc = glGetUniformLocation(terrainGrassShader, "ambient");

        grassDisplayDistanceLoc = glGetUniformLocation(terrainGrassShader, "grassDisplayDistance");
        grassHeightLoc = glGetUniformLocation(terrainGrassShader, "grassHeight");
        grassHalfLengthLoc = glGetUniformLocation(terrainGrassShader, "grassHalfLength");
        numGrassInTexLoc = glGetUniformLocation(terrainGrassShader, "numGrassInTex");

        windDirectionLoc = glGetUniformLocation(terrainGrassShader, "windDirection");
        windStrengthLoc = glGetUniformLocation(terrainGrassShader, "windStrength");

        int terrainGrassTexLoc = glGetUniformLocation(terrainGrassShader, "grassTex");
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(terrainGrassTexLoc, 0);

        int terrainGrassMaskTexLoc = glGetUniformLocation(terrainGrassShader, "grassMaskTex");
        glActiveTexture(GL_TEXTURE1);
        glUniform1i(terrainGrassMaskTexLoc, 1);

        setGrassTexture(grassTextureFilename);
        setMaskTexture("");
        setNumGrassInTexture(DEFAULT_NUM_GRASS_IN_TEX);
        setGrassHeight(DEFAULT_GRASS_HEIGHT);
        setGrassLength(DEFAULT_GRASS_LENGTH);
        setGrassQuantity(DEFAULT_GRASS_QUANTITY);
        setWindDirection(DEFAULT_WIND_DIRECTION);
        setWindStrength(DEFAULT_WIND_STRENGTH);
    }

    TerrainGrass::~TerrainGrass()
    {
        clearVBO();
        ShaderManager::instance()->removeProgram(terrainGrassShader);

        grassTexture->release();
        grassMaskTexture->release();
        delete mainGrassQuadtree;
    }

    void TerrainGrass::onCameraProjectionUpdate(const Matrix4<float> &projectionMatrix)
    {
        this->projectionMatrix = projectionMatrix;

        ShaderManager::instance()->bind(terrainGrassShader);
        glUniformMatrix4fv(mProjectionLoc, 1, GL_FALSE, (const float*)projectionMatrix);
    }

    void TerrainGrass::refreshWith(const std::shared_ptr<TerrainMesh> &mesh, const Point3<float> &terrainPosition)
    {
        generateGrass(mesh, terrainPosition);

        ShaderManager::instance()->bind(terrainGrassShader);
        glUniform3fv(terrainMinPointLoc, 1, (const float *)mesh->getVertices()[0]);
        glUniform3fv(terrainMaxPointLoc, 1, (const float *)mesh->getVertices()[mesh->getXSize()*mesh->getZSize()-1]);
    }

    void TerrainGrass::refreshWith(float ambient)
    {
        ShaderManager::instance()->bind(terrainGrassShader);
        glUniform1f(terrainAmbientLoc, ambient);
    }

    void TerrainGrass::generateGrass(const std::shared_ptr<TerrainMesh> &mesh, const Point3<float> &terrainPosition)
    {
        const unsigned int NUM_THREADS = std::max(2u, std::thread::hardware_concurrency());

        if(mesh!=nullptr)
        {
            this->mesh = mesh;
            this->terrainPosition = terrainPosition;

            std::default_random_engine generator;
            std::uniform_real_distribution<float> distribution(-grassPositionRandomPercentage / grassQuantity, grassPositionRandomPercentage/grassQuantity);

            unsigned int grassXQuantity = mesh->getXZScale() * mesh->getXSize() * grassQuantity;
            unsigned int grassZQuantity = mesh->getXZScale() * mesh->getZSize() * grassQuantity;

            unsigned int patchQuantityX = mesh->getXZScale() * mesh->getXSize() / grassPatchSize;
            unsigned int patchQuantityZ = mesh->getXZScale() * mesh->getZSize() / grassPatchSize;
            float adjustedPatchSizeX = mesh->getXZScale() * mesh->getXSize() / patchQuantityX;
            float adjustedPatchSizeZ = mesh->getXZScale() * mesh->getZSize() / patchQuantityZ;

            std::vector<TerrainGrassQuadtree *> leafGrassPatches;
            leafGrassPatches.reserve(patchQuantityX * patchQuantityZ);
            for (unsigned int i = 0; i < patchQuantityX * patchQuantityZ; ++i)
            {
                leafGrassPatches.push_back(new TerrainGrassQuadtree());
            }

            float startX = mesh->getVertices()[0].X;
            float startZ = mesh->getVertices()[0].Z;

            std::vector<std::thread> threads(NUM_THREADS);
            for(unsigned int threadI=0; threadI<NUM_THREADS; threadI++)
            {
                unsigned int beginX = threadI * grassXQuantity / NUM_THREADS;
                unsigned int endX = (threadI + 1) == NUM_THREADS ? grassXQuantity : (threadI + 1) * grassXQuantity / NUM_THREADS;

                threads[threadI] = std::thread(std::bind([&](unsigned int beginX, unsigned int endX)
                {
                    for (unsigned int xIndex = beginX; xIndex < endX; ++xIndex)
                    {
                        const float xFixedValue = startX + xIndex / grassQuantity;

                        for (unsigned int zIndex = 0; zIndex < grassZQuantity; ++zIndex)
                        {
                            float xValue = xFixedValue + distribution(generator);
                            float zValue = (startZ + zIndex / grassQuantity) + distribution(generator);
                            unsigned int vertexIndex = retrieveVertexIndex(Point2<float>(xValue, zValue));
                            float yValue = (mesh->getVertices()[vertexIndex] + terrainPosition).Y;

                            Point3<float> globalGrassVertex(xValue + terrainPosition.X, yValue, zValue + terrainPosition.Z);
                            Vector3<float> grassNormal = (mesh->getNormals()[vertexIndex] / 2.0f) + Vector3<float>(0.5f, 0.5f, 0.5f);

                            unsigned int patchXIndex = std::min(static_cast<unsigned int>((xValue - startX) / adjustedPatchSizeX), patchQuantityX);
                            unsigned int patchZIndex = std::min(static_cast<unsigned int>((zValue - startZ) / adjustedPatchSizeZ), patchQuantityZ);
                            unsigned int patchIndex = (patchZIndex * patchQuantityX) + patchXIndex;

                            leafGrassPatches[patchIndex]->addVertex(globalGrassVertex, grassNormal);
                        }
                    }
                }, beginX, endX));
            }
            std::for_each(threads.begin(), threads.end(), [](std::thread& x){x.join();});

            buildGrassQuadtree(leafGrassPatches, patchQuantityX, patchQuantityZ);
            createVBO(leafGrassPatches);
        }
    }

    unsigned int TerrainGrass::retrieveVertexIndex(const Point2<float> &localXzCoordinate) const
    {
        Point3<float> localCoordinate = Point3<float>(localXzCoordinate.X, 0.0f, localXzCoordinate.Y);
        Point3<float> farLeftCoordinate = localCoordinate - mesh->getVertices()[0];

        float xInterval = mesh->getVertices()[1].X - mesh->getVertices()[0].X;
        int xIndex = MathAlgorithm::clamp(static_cast<int>(std::round(farLeftCoordinate.X / xInterval)), 0, static_cast<int>(mesh->getXSize() - 1));

        float zInterval = mesh->getVertices()[mesh->getXSize()].Z - mesh->getVertices()[0].Z;
        int zIndex = MathAlgorithm::clamp(static_cast<int>(std::round(farLeftCoordinate.Z / zInterval)), 0, static_cast<int>(mesh->getZSize() - 1));

        return xIndex + zIndex*mesh->getXSize();
    }

    void TerrainGrass::buildGrassQuadtree(const std::vector<TerrainGrassQuadtree *> &leafGrassPatches, unsigned int leafQuantityX, unsigned int leafQuantityZ)
    {
        std::vector<TerrainGrassQuadtree *> childrenGrassQuadtree = leafGrassPatches;
        unsigned int childrenNbQuadtreeX = leafQuantityX;
        unsigned int childrenNbQuadtreeZ = leafQuantityZ;

        unsigned int depth = grassQuadtreeDepth;
        while(depth >= 1)
        {
            auto depthNbQuadtreeX = static_cast<unsigned int>(MathAlgorithm::pow(2, depth));
            unsigned int depthNbQuadtreeZ = depthNbQuadtreeX;
            unsigned int depthNbQuadtree = depthNbQuadtreeX * depthNbQuadtreeZ;
            if(std::sqrt(childrenGrassQuadtree.size()) >= std::sqrt(depthNbQuadtree)*2)
            {
                std::vector<TerrainGrassQuadtree *> depthGrassQuadtree;
                depthGrassQuadtree.reserve(depthNbQuadtree);
                for(unsigned int i=0; i<depthNbQuadtree; ++i)
                {
                    depthGrassQuadtree.push_back(new TerrainGrassQuadtree());
                }

                for (unsigned int childZ = 0; childZ < childrenNbQuadtreeZ; ++childZ)
                {
                    for (unsigned int childX = 0; childX < childrenNbQuadtreeX; ++childX)
                    {
                        int xQuadtreeIndex = (depthNbQuadtreeX/static_cast<float>(childrenNbQuadtreeX)) * (childX + 0.5f);
                        int zQuadtreeIndex = (depthNbQuadtreeZ/static_cast<float>(childrenNbQuadtreeZ)) * (childZ + 0.5f);

                        unsigned int quadtreeIndex = (zQuadtreeIndex * depthNbQuadtreeX) + xQuadtreeIndex;
                        unsigned int childQuadtreeIndex = (childZ * childrenNbQuadtreeZ) + childX;

                        depthGrassQuadtree[quadtreeIndex]->addChild(childrenGrassQuadtree[childQuadtreeIndex]);
                    }
                }

                childrenGrassQuadtree = depthGrassQuadtree;
                childrenNbQuadtreeX = depthNbQuadtreeX;
                childrenNbQuadtreeZ = depthNbQuadtreeZ;
            }

            depth--;
        }

        delete mainGrassQuadtree;
        mainGrassQuadtree = new TerrainGrassQuadtree(childrenGrassQuadtree);
    }

    void TerrainGrass::createVBO(const std::vector<TerrainGrassQuadtree *> &leafGrassPatches)
    {
        clearVBO();

        vertexArrayObjects.resize(leafGrassPatches.size());
        glGenVertexArrays(leafGrassPatches.size(), &vertexArrayObjects[0]);
        bufferIDs.resize(leafGrassPatches.size());
        glGenBuffers(leafGrassPatches.size() * 2, &bufferIDs[0][0]);

        unsigned int quadtreeId = 0;
        for(auto *grassQuadtree : leafGrassPatches)
        {
            glBindVertexArray(vertexArrayObjects[quadtreeId]);

            glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[quadtreeId][VAO_VERTEX_POSITION]);
            glEnableVertexAttribArray(SHADER_VERTEX_POSITION);
            glVertexAttribPointer(SHADER_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glBufferData(GL_ARRAY_BUFFER, grassQuadtree->getGrassVertices().size() * sizeof(float) * 3, &grassQuadtree->getGrassVertices()[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[quadtreeId][VAO_NORMAL]);
            glEnableVertexAttribArray(SHADER_NORMAL);
            glVertexAttribPointer(SHADER_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glBufferData(GL_ARRAY_BUFFER, grassQuadtree->getGrassNormals().size() * sizeof(float) * 3, &grassQuadtree->getGrassNormals()[0], GL_STATIC_DRAW);

            grassQuadtree->setVertexArrayObjectId(quadtreeId++);
        }
    }

    void TerrainGrass::clearVBO()
    {
        if(!vertexArrayObjects.empty())
        {
            glDeleteVertexArrays(vertexArrayObjects.size(), &vertexArrayObjects[0]);
            vertexArrayObjects.clear();
        }

        if(!bufferIDs.empty())
        {
            glDeleteBuffers(bufferIDs.size(), &bufferIDs[0][0]);
            bufferIDs.clear();
        }
    }

    const std::string &TerrainGrass::getGrassTexture() const
    {
        return grassTextureFilename;
    }

    void TerrainGrass::setGrassTexture(const std::string &grassTextureFilename)
    {
        this->grassTextureFilename = grassTextureFilename;

        if(grassTexture!=nullptr)
        {
            grassTexture->release();
        }

        if(grassTextureFilename.empty())
        {
            grassTexture = nullptr;
        }else
        {
            grassTexture = MediaManager::instance()->getMedia<Image>(grassTextureFilename, nullptr);
            grassTexture->toTexture(true, true, false);
        }
    }

    const std::string &TerrainGrass::getMaskTexture() const
    {
        return grassMaskFilename;
    }

    void TerrainGrass::setMaskTexture(const std::string &grassMaskFilename)
    {
        this->grassMaskFilename = grassMaskFilename;

        if(grassMaskTexture!=nullptr)
        {
            grassMaskTexture->release();
        }

        if(grassMaskFilename.empty())
        {
            grassMaskTexture = new Image(1, 1, Image::IMAGE_GRAYSCALE, std::vector<unsigned char>({0}));
            grassMaskTexture->toTexture(false, false, false);
        } else
        {
            grassMaskTexture = MediaManager::instance()->getMedia<Image>(grassMaskFilename, nullptr);
            grassMaskTexture->toTexture(false, false, false);
        }
    }

    float TerrainGrass::getGrassDisplayDistance() const
    {
        return grassDisplayDistance;
    }

    void TerrainGrass::setGrassDisplayDistance(float grassDisplayDistance)
    {
        this->grassDisplayDistance = grassDisplayDistance;

        ShaderManager::instance()->bind(terrainGrassShader);
        glUniform1f(grassDisplayDistanceLoc, grassDisplayDistance);
    }

    float TerrainGrass::getGrassHeight() const
    {
        return grassHeight;
    }

    void TerrainGrass::setGrassHeight(float grassHeight)
    {
        this->grassHeight = grassHeight;

        ShaderManager::instance()->bind(terrainGrassShader);
        glUniform1f(grassHeightLoc, grassHeight);
    }

    float TerrainGrass::getGrassLength() const
    {
        return grassLength;
    }

    void TerrainGrass::setGrassLength(float grassLength)
    {
        this->grassLength = grassLength;

        ShaderManager::instance()->bind(terrainGrassShader);
        glUniform1f(grassHalfLengthLoc, grassLength / 2.0f);
    }

    unsigned int TerrainGrass::getNumGrassInTexture() const
    {
        return numGrassInTex;
    }

    void TerrainGrass::setNumGrassInTexture(unsigned int numGrassInTex)
    {
        this->numGrassInTex = numGrassInTex;

        ShaderManager::instance()->bind(terrainGrassShader);
        glUniform1i(numGrassInTexLoc, numGrassInTex);
    }

    float TerrainGrass::getGrassQuantity() const
    {
        return grassQuantity;
    }

    void TerrainGrass::setGrassQuantity(float grassQuantity)
    {
        this->grassQuantity = grassQuantity;

        generateGrass(mesh, terrainPosition);
    }

    Vector3<float> TerrainGrass::getWindDirection() const
    {
        return windDirection;
    }

    void TerrainGrass::setWindDirection(const Vector3<float> &windDirection)
    {
        this->windDirection = windDirection.normalize();

        ShaderManager::instance()->bind(terrainGrassShader);
        glUniform3fv(windDirectionLoc, 1, (const float *)this->windDirection);
    }

    float TerrainGrass::getWindStrength() const
    {
        return windStrength;
    }

    void TerrainGrass::setWindStrength(float windStrength)
    {
        this->windStrength = windStrength;

        ShaderManager::instance()->bind(terrainGrassShader);
        glUniform1f(windStrengthLoc, windStrength);
    }

    void TerrainGrass::display(const Camera *camera, float invFrameRate)
    {
        if(grassTexture!=nullptr)
        {
            ScopeProfiler profiler("3d", "grassDisplay");

            #ifdef _DEBUG
                assert(grassDisplayDistance!=0.0f);
            #endif

            ShaderManager::instance()->bind(terrainGrassShader);

            glDisable(GL_CULL_FACE);
            glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, grassTexture->getTextureID());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, grassMaskTexture->getTextureID());

            sumTimeStep += invFrameRate;
            glUniform1f(sumTimeStepLoc, sumTimeStep);
            glUniformMatrix4fv(mViewLoc, 1, GL_FALSE, (const float *) camera->getViewMatrix());
            glUniform3fv(cameraPositionLoc, 1, (const float *)camera->getPosition());

            std::stack<const TerrainGrassQuadtree *> grassQuadtrees;
            grassQuadtrees.push(mainGrassQuadtree);
            while (!grassQuadtrees.empty())
            {
                const TerrainGrassQuadtree *grassQuadtree = grassQuadtrees.top();
                grassQuadtrees.pop();

                if (camera->getFrustum().cutFrustum(grassDisplayDistance).collideWithAABBox(*grassQuadtree->getBox()))
                {
                    if (grassQuadtree->isLeaf())
                    {
                        glBindVertexArray(vertexArrayObjects[grassQuadtree->getVertexArrayObjectId()]);
                        glDrawArrays(GL_POINTS, 0, grassQuadtree->getGrassVertices().size());
                    } else
                    {
                        for (const auto *child : grassQuadtree->getChildren())
                        {
                            grassQuadtrees.push(child);
                        }
                    }
                }
            }

            glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
            glEnable(GL_CULL_FACE);
        }
    }

}
