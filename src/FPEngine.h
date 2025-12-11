#ifndef FP_ENGINE_H
#define FP_ENGINE_H

#include <CSCI441/Camera.hpp>
#include <CSCI441/FreeCam.hpp>
#include <CSCI441/OpenGLEngine.hpp>
#include <CSCI441/ShaderProgram.hpp>

#include "ArcballCam.hpp"
#include "Character.h"
#include "Coin.h"
#include "Enemy.h"
#include "ParticleSystem.h"
#include "Skybox.h"

#include <vector>

class FPEngine final : public CSCI441::OpenGLEngine {
public:
  FPEngine();
  ~FPEngine() override;

  void run() override;

  /// \desc handle any key events inside the engine
  /// \param KEY key as represented by GLFW_KEY_ macros
  /// \param ACTION key event action as represented by GLFW_ macros
  void handleKeyEvent(GLint KEY, GLint ACTION);

  /// \desc handle any mouse button events inside the engine
  /// \param BUTTON mouse button as represented by GLFW_MOUSE_BUTTON_ macros
  /// \param ACTION mouse event as represented by GLFW_ macros
  void handleMouseButtonEvent(GLint BUTTON, GLint ACTION);

  /// \desc handle any cursor movement events inside the engine
  /// \param currMousePosition the current cursor position
  void handleCursorPositionEvent(glm::vec2 currMousePosition);

  /// \desc value off-screen to represent mouse has not begun interacting with
  /// window yet
  static constexpr GLfloat MOUSE_UNINITIALIZED = -9999.0f;

private:
  void mSetupGLFW() override;
  void mSetupOpenGL() override;
  void mSetupShaders() override;
  void mSetupTextures() override;
  void mSetupBuffers() override;
  void mSetupScene() override;

  void mCleanupScene() override;
  void mCleanupBuffers() override;
  void mCleanupShaders() override;

  /// \desc draws everything to the scene from a particular point of view
  /// \param viewMtx the current view matrix for our camera
  /// \param projMtx the current projection matrix for our camera
  //  param cameraPos: the position of the camera for lighting shenanigans
  void _renderScene(const glm::mat4 &viewMtx, const glm::mat4 &projMtx,
                    const glm::vec3 &cameraPos) const;
  /// \desc handles moving our camera as determined by keyboard input
  void _updateScene();

  /// \desc tracks the number of different keys that can be present as
  /// determined by GLFW
  static constexpr GLuint NUM_KEYS = GLFW_KEY_LAST;
  /// \desc boolean array tracking each key state.  if true, then the key is in
  /// a pressed or held down state.  if false, then the key is in a released
  /// state and not being interacted with
  GLboolean _keys[NUM_KEYS] = {0};

  /// \desc last location of the mouse in window coordinates
  glm::vec2 _mousePosition;
  /// \desc current state of the left mouse button
  GLint _leftMouseButtonState;

  /// \desc total number of textures in our scene
  static constexpr GLuint NUM_TEXTURES = 4;
  /// \desc used to index through our texture array to give named access
  enum TEXTURE_ID {
    /// \desc ground texture
    GROUND = 0,
    // enemy texture
    ENEMY = 1,
    // coin texture
    COIN = 2,
    // particle texture
    PARTICLE = 3,
  };
  /// \desc texture handles for our textures
  GLuint _texHandles[NUM_TEXTURES] = {0};

  /// \desc the arcball camera in our world
  CSCI441::Camera *_cam;
  CSCI441::ArcballCam *_arcBallCam;
  CSCI441::FreeCam *_freeCam;
  CSCI441::FreeCam *_firstPersonCam;
  /// \desc pair of values to store the speed the camera can move/rotate.
  /// \brief x = forward/backward delta, y = rotational delta
  glm::vec2 _cameraSpeed;

  // i have eliminated the other characters, it is only elster left...
  Character *_pCharacter;
  float _characterMoveSpeed;
  float _characterTurnSpeed;
  float _characterVerticalVelocity;
  bool _characterOnGround;
  bool _characterDead;

  Skybox *_pSkybox;

  // game objects
  std::vector<Enemy *> _enemies;
  std::vector<Coin *> _coins;
  ParticleSystem *_particleSystem;
  int _coinsCollected;

  /// \desc the size of the world (controls the ground size and locations of
  /// buildings)
  static constexpr GLfloat WORLD_SIZE = 110.0f;
  /// \desc VAO for our ground
  GLuint _groundVAO;
  /// \desc the number of points that make up our ground object
  GLsizei _numGroundPoints;

  /// \desc smart container to store information specific to each tree we wish
  /// to draw
  struct TreeData {
    /// \desc transformations to position and size the tree
    glm::mat4 leavesModelMatrix;
    glm::mat4 trunkModelMatrix;
    /// \desc color to draw the tree
    glm::vec3 color;
    glm::vec3 barkColor;
    float frameOffset;
  };
  /// \desc information list of all the trees to draw
  std::vector<TreeData> _trees;

  struct BushData {
    glm::vec3 position;
    glm::vec3 color;
    GLfloat size;
  };
  std::vector<BushData> _bushes;

  /// \desc generates tree information to make up our scene
  void _generateEnvironment();

  /// \desc loads an image into CPU memory and registers it with the GPU
  /// \note sets the texture parameters and sends the data to the GPU
  /// \param FILENAME external image filename to load
  static GLuint _loadAndRegisterTexture(const char *FILENAME);

  /// \desc creates the ground VAO
  void _createGroundBuffers();

  /// \desc shader program that performs lighting
  CSCI441::ShaderProgram
      *_lightingShaderProgram; // the wrapper for our shader program
  /// \desc stores the locations of all of our shader uniforms
  struct LightingShaderUniformLocations {
    /// \desc precomputed MVP matrix location
    GLint mvpMatrix;
    /// \desc material diffuse color location
    GLint materialColor;
    // TODO #1: add new uniforms
    GLint lightDirection;
    GLint lightPosition;
    GLint pointLightColor;
    GLint spotLightPosition;
    GLint spotLightDirection;
    GLint spotLightColor;
    GLint lightColor;
    GLint normalMatrix;
    GLint modelMatrix;
    GLint cameraPosition;
  } _lightingShaderUniformLocations;
  /// \desc stores the locations of all of our shader attributes
  struct LightingShaderAttributeLocations {
    /// \desc vertex position location
    GLint vPos;
    // TODO #2: add new attributes
    GLint vNormal;

  } _lightingShaderAttributeLocations;

  // Shaders for elster
  CSCI441::ShaderProgram *_elsterShaderProgram;
  struct ElsterShaderUniformLocations {
    GLint mvpMatrix;
    GLint normalMatrix;
    GLint modelMatrix;
    GLint viewMatrix;
    GLint materialDiffuse;
    GLint materialSpecular;
    GLint materialShininess;
    GLint lightDirection;
    GLint lightPosition;
    GLint pointLightColor;
    GLint spotLightPosition;
    GLint spotLightDirection;
    GLint spotLightColor;
    GLint lightColor;
    GLint cameraPosition;
    GLint ambientLight;
    GLint useSkinning;
    GLint jointMatrices;
  } _elsterShaderUniformLocations;

  struct ElsterShaderAttributeLocations {
    GLint vPos;
    GLint vNormal;
    GLint vJoints;
    GLint vWeights;
  } _elsterShaderAttributeLocations;

  // tess shaders for ground
  CSCI441::ShaderProgram *_groundTessShaderProgram;
  struct GroundTessShaderUniformLocations {
    GLint mvpMatrix;
    GLint modelMatrix;
    GLint normalMatrix;
    GLint groundTexture;
    GLint tessLevel;
    GLint hillHeight;
    GLint lightDirection;
    GLint lightColor;
    GLint lightPosition;
    GLint pointLightColor;
    GLint spotLightPosition;
    GLint spotLightDirection;
    GLint spotLightColor;
    GLint cameraPosition;
  } _groundTessShaderUniformLocations;

  struct GroundTessShaderAttributeLocations {
    GLint vPos;
    GLint vNormal;
    GLint vTexCoord;
  } _groundTessShaderAttributeLocations;

  // sprite shader for enemies, coins, and particles
  CSCI441::ShaderProgram *_spriteShaderProgram;
  struct SpriteShaderUniformLocations {
    GLint mvpMatrix;
    GLint spriteTexture;
  } _spriteShaderUniformLocations;

  /// \desc set the lighting parameters to the shader
  void _setLightingParameters();

  // spawn enemies around the world
  void _spawnEnemies(int numEnemies);

  // spawn coins at corners of the map
  void _spawnCoins();

  // check collision between enemies
  void _checkEnemyCollisions();

  // check collision between player and enemies
  void _checkPlayerEnemyCollision();

  // check collision between player and coins
  void _checkCoinCollection();

  // calculates the height of the Bezier terrain at a given position
  float _getTerrainHeight(float x, float z) const;

  // checks collision between character and vegetation and returns corrected
  // position
  glm::vec3 _checkAndResolveCollisions(const glm::vec3 &position,
                                       float characterRadius) const;

  // gets the height of the tallest object at a given position
  float _getObjectHeightAt(float x, float z) const;

  /// \desc precomputes the matrix uniforms CPU-side and then sends them
  /// to the GPU to be used in the shader for each vertex.  It is more efficient
  /// to calculate these once and then use the resultant product in the shader.
  /// \param modelMtx model transformation matrix
  /// \param viewMtx camera view matrix
  /// \param projMtx camera projection matrix
  void _computeAndSendMatrixUniforms(const glm::mat4 &modelMtx,
                                     const glm::mat4 &viewMtx,
                                     const glm::mat4 &projMtx) const;
};

void mp_engine_keyboard_callback(GLFWwindow *window, int key, int scancode,
                                 int action, int mods);
void mp_engine_cursor_callback(GLFWwindow *window, double x, double y);
void mp_engine_mouse_button_callback(GLFWwindow *window, int button, int action,
                                     int mods);

#endif
