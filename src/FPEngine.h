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
#include "Tympanius.h"
#include "TympaniusShaderAttributeLocations.hpp"
#include "TympaniusShaderUniformLocations.hpp"
#include "Farina.h"
#include "ParticleSystem.h"
#include "Wilfred.h"

#include "Skybox.h"
#include <vector>

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
  /// renders the minimap with flat shading
  void _renderMinimap(const glm::mat4 &viewMtx, const glm::mat4 &projMtx) const;
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
  static constexpr GLuint NUM_TEXTURES = 5;
  /// \desc used to index through our texture array to give named access
  enum TEXTURE_ID {
    /// \desc ground texture
    GROUND = 0,
    /// wall texture
    WALL = 1,
    // particle texture
    PARTICLE = 2,
    // Hands texture
    PLAYER = 3,
  };
  /// \desc texture handles for our textures
  GLuint _texHandles[NUM_TEXTURES] = {0};

  /// \desc the arcball camera in our world
  CSCI441::Camera *_cam;
  CSCI441::ArcballCam *_arcBallCam;
  CSCI441::FreeCam *_firstPersonCam;
  CSCI441::FreeCam *_minimapCam; // Top-down camera for minimap
  float _minimapHeight; // Height of the minimap camera
  /// \desc pair of values to store the speed the camera can move/rotate.
  /// \brief x = forward/backward delta, y = rotational delta
  glm::vec2 _cameraSpeed;
  float _cameraPitch;

  // i have eliminated the other characters, it is only elster left...
  // ^ nuh uh - mari
  Character *_pCharacter;
  Tympanius *_pTympanius;
  Wilfred *_pWilfred;
  Character *_pEnemyElster;
  Farina *_pFarina;
  std::vector<Enemy*> _enemies;

  float aberrationStrength = 0.02f;
  float rNoise = 0;
  float gNoise = 0;
  float bNoise = 0;

  float _characterMoveSpeed;
  float _characterTurnSpeed;
  float _characterVerticalVelocity;
  bool _characterOnGround;
  bool _characterDead;
  int _enemyThatKilled = -1; // 0 = Tympanius, 1 = Wilfred, 2 = Elster, 3 = Farina
  float _deathEasingParam = 0;

  Skybox *_pSkybox;

  const glm::vec3 spotLightColor = {0.95f, 1.0f, 0.9f};
  const glm::vec3 pointLightColor = {1.0f, 0.0f, 0.0f};
  const glm::vec3 lightDirection = {-1.0f, 0.1f, -0.2f};
  const glm::vec3 lightColor = {0, 0, 0};

  // game objects
  ParticleSystem *_particleSystem;

  /// \desc the size of the world (controls the ground size and locations of
  /// buildings)
  static constexpr GLfloat WORLD_SIZE = 110.0f;
  /// \desc VAO for our ground
  GLuint _groundVAO;
  /// \desc the number of points that make up our ground object
  GLsizei _numGroundPoints;

  struct WallVertexData {
    glm::vec3 position;
    glm::vec2 texCoords;
  };
  struct WallData {
    glm::vec3 position;
    GLfloat size;
  };
  std::vector<WallData> _walls;

  GLuint _postFBO;
  GLuint _postTextureID;
  GLuint _quadVAO;
  GLuint _quadVBO;
  GLuint _quadIBO;
  GLsizei _numQuadVAOPoints = 0;

  void _createQuad();

  /// \desc generates tree information to make up our scene
  void _generateEnvironment();

  /// \desc loads an image into CPU memory and registers it with the GPU
  /// \note sets the texture parameters and sends the data to the GPU
  /// \param FILENAME external image filename to load
  static GLuint _loadAndRegisterTexture(const char *FILENAME);

  /// \desc creates the ground VAO
  void _createGroundBuffers();

  CSCI441::ShaderProgram
      *_textureShaderProgram; // the wrapper for our shader program
  struct TextureShaderUniformLocations {
    /// \desc precomputed MVP matrix location
    GLint mvpMatrix;
    // TODO #11 - texture map
    GLint texMap;
    GLint spotLightPosition;
    GLint spotLightDirection;
    GLint spotLightColor;
    GLint normalMatrix;
    GLint modelMatrix;
    GLint cameraPosition;
    GLint materialColor;
    GLint isTextured;
  } _textureShaderUniformLocations;

  struct TextureShaderAttributeLocations {
        /// \desc vertex position location
        GLint vPos;
        /// \desc vertex normal location
        /// \note not used in this lab
        GLint vNormal;
        // TODO #10 - texture coordinate
        GLint texCoord;

    } _textureShaderAttributeLocations;

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

  /// \desc shader program that performs lighting
    CSCI441::ShaderProgram* _tympaniusShaderProgram ;   // the wrapper for our shader program
    /// \desc stores the locations of all of our shader uniforms
    TympaniusShaderUniformLocations _tympaniusShaderUniformLocations;
    /// \desc stores the locations of all of our shader attributes
    TympaniusShaderAttributeLocations _tympaniusShaderAttributeLocations;

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

  // flat shader for minimap
  CSCI441::ShaderProgram *_flatShaderProgram;
  struct FlatShaderUniformLocations {
    GLint mvpMatrix;
    GLint modelMatrix;
    GLint normalMatrix;
    GLint materialColor;
    GLint lightDirection;
  } _flatShaderUniformLocations;

  struct FlatShaderAttributeLocations {
    GLint vPos;
    GLint vNormal;
  } _flatShaderAttributeLocations;

  CSCI441::ShaderProgram *_postShaderProgram;
  struct PostShaderUniformLocations {
    GLint mvpMatrix;
    GLint sceneTexture;
    GLint rOffset;
    GLint gOffset;
    GLint bOffset;
    GLint rNoise;
    GLint gNoise;
    GLint bNoise;

  } _postShaderUniformLocations;
  struct PostShaderAttributeLocations {
    GLint vPos;
    GLint texCoord;
  } _postShaderAttributeLocations;

  /// \desc set the lighting parameters to the shader
  void _setLightingParameters();

  // check collision between enemies
  void _checkEnemyCollisions();

  // get closest enemy's distance to player
  float _getEnemyDistance();

  // check collision between player and enemies
  void _checkPlayerEnemyCollision();

  // calculates the height of the Bezier terrain at a given position
  float _getTerrainHeight(float x, float z) const;

  // checks collision between character and vegetation and returns corrected
  // position
  glm::vec3 _checkAndResolveCollisions(const glm::vec3 &position,
                                       float characterRadius) const;

  // gets the height of the tallest object at a given position
  float _getObjectHeightAt(float x, float z) const;

  
  void doDeath(const char* killerName, glm::vec3 playerPos);

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
