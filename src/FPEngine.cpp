#include "FPEngine.h"

#include <iostream>
#include <sstream>
#include <CSCI441/objects.hpp>
#include <stb_image.h>

#include <glm/gtc/constants.hpp> // for glm::pi()
#include <glm/gtc/type_ptr.hpp>  // for glm::value_ptr()

#include <iostream>

//*************************************************************************************
//
// Public Interface

FPEngine::FPEngine()
    : CSCI441::OpenGLEngine(4, 1, 640, 480, "FP: The Big Spooky"),
      _mousePosition({MOUSE_UNINITIALIZED, MOUSE_UNINITIALIZED}),
      _leftMouseButtonState(GLFW_RELEASE), _cam(nullptr),
      _cameraSpeed({0.0f, 0.0f}), _cameraPitch(glm::pi<float>() * 0.5f),
      _groundVAO(0), _numGroundPoints(0),
      _lightingShaderProgram(nullptr),
    _spriteShaderProgram(nullptr), 
    _spriteShaderUniformLocations( {-1, -1} ),
    _textureShaderProgram(nullptr),
    _textureShaderUniformLocations({-1, -1}),
    _textureShaderAttributeLocations({-1, -1, -1}),
      _lightingShaderUniformLocations({-1, -1, -1, -1, -1}),
      _lightingShaderAttributeLocations({-1, -1}), _pCharacter(nullptr), _pTympanius(nullptr),
      _pWilfred(nullptr), _pEnemyElster(nullptr), _pFarina(nullptr),
      _characterMoveSpeed(10.0f), _characterTurnSpeed(2.0f),
      _characterVerticalVelocity(0.0f), _characterOnGround(true),
      _characterDead(false), _particleSystem(nullptr), _coinsCollected(0){

  for (auto &_key : _keys)
    _key = GL_FALSE;
}

FPEngine::~FPEngine() {
  delete _arcBallCam;
  delete _firstPersonCam;
  delete _pCharacter;
  delete _pTympanius;
  delete _pWilfred;
  delete _pEnemyElster;
  delete _pFarina;
  delete _elsterShaderProgram;
  delete _groundTessShaderProgram;
    delete _textureShaderProgram;
  delete _pSkybox;
  delete _spriteShaderProgram;
  delete _particleSystem;

  for (auto enemy : _enemies) {
    delete enemy;
  }
  _enemies.clear();

  for (auto coin : _coins) {
    delete coin;
  }
  _coins.clear();
}

void FPEngine::handleKeyEvent(const GLint KEY, const GLint ACTION) {
  if (KEY != GLFW_KEY_UNKNOWN)
    _keys[KEY] = ((ACTION == GLFW_PRESS) || (ACTION == GLFW_REPEAT));

  if (ACTION == GLFW_PRESS) {
    switch (KEY) {
    // quit!
    case GLFW_KEY_Q:
    case GLFW_KEY_ESCAPE:
      setWindowShouldClose();
      break;

    case GLFW_KEY_R:
      mReloadShaders();
      _setLightingParameters();
      // Update Character shader references after reload
      _pCharacter->updateShaderReferences(
          _elsterShaderProgram->getShaderProgramHandle(),
          _elsterShaderUniformLocations.mvpMatrix,
          _elsterShaderUniformLocations.normalMatrix,
          _elsterShaderUniformLocations.modelMatrix,
          _elsterShaderUniformLocations.materialDiffuse,
          _elsterShaderUniformLocations.materialSpecular,
          _elsterShaderUniformLocations.materialShininess);
      _pWilfred = new Wilfred(_lightingShaderProgram->getShaderProgramHandle(),
                              _lightingShaderUniformLocations.mvpMatrix,
                              _lightingShaderUniformLocations.normalMatrix,
                              _lightingShaderUniformLocations.materialColor,
                              _lightingShaderUniformLocations.modelMatrix);
      // Reload ground tessellation shader attribute locations
      _groundTessShaderAttributeLocations.vPos =
          _groundTessShaderProgram->getAttributeLocation("vPos");
      _groundTessShaderAttributeLocations.vNormal =
          _groundTessShaderProgram->getAttributeLocation("vNormal");
      _groundTessShaderAttributeLocations.vTexCoord =
          _groundTessShaderProgram->getAttributeLocation("vTexCoord");
      break;

      // Camera switching delete
      // TODO: have a disable minimap once created
      // case GLFW_KEY_1:
      // case GLFW_KEY_2:

    default:
      break; // suppress CLion warning
    }
  }
}

void FPEngine::handleMouseButtonEvent(const GLint BUTTON, const GLint ACTION) {
  // if the event is for the left mouse button
  if (BUTTON == GLFW_MOUSE_BUTTON_LEFT) {
    // update the left mouse button's state
    _leftMouseButtonState = ACTION;
  }
}

void FPEngine::handleCursorPositionEvent(const glm::vec2 currMousePosition) {
  // if mouse hasn't moved in the window, prevent camera from flipping out
  if (_mousePosition.x == MOUSE_UNINITIALIZED) {
    _mousePosition = currMousePosition;
    return;
  }

  // For first-person camera, rotate the character's heading with mouse
  if (_cam == _firstPersonCam) {
    // Horizontal mouse movement controls character heading (yaw)
    float deltaX = (currMousePosition.x - _mousePosition.x) * 0.005f;

    // Turn character left/right based on mouse movement
    if (!_characterDead) {
      _pCharacter->turnRight(deltaX * 2.0f);
    }

    // Vertical mouse movement controls camera pitch
    float deltaPhi = -(currMousePosition.y - _mousePosition.y) * 0.005f;
    _cameraPitch += deltaPhi;

    // Clamp pitch to prevent flipping (look straight up to straight down)
    const float maxPitch = glm::pi<float>() * 0.95f;  // Almost straight up
    const float minPitch = glm::pi<float>() * 0.05f;  // Almost straight down
    _cameraPitch = glm::clamp(_cameraPitch, minPitch, maxPitch);
  }
  // For other cameras, require left mouse button to be held down
  else if (_leftMouseButtonState == GLFW_PRESS) {
    // Check if Shift is also pressed for zooming
    if (_keys[GLFW_KEY_LEFT_SHIFT] || _keys[GLFW_KEY_RIGHT_SHIFT]) {
      // zoom based on vertical mouse movement
      float deltaY = currMousePosition.y - _mousePosition.y;
      float zoomFactor = 0.1f; // Adjust sensitivity as needed

      if (deltaY < 0.0f) { // Dragging up
        _cam->moveForward(-deltaY * zoomFactor);
      } else if (deltaY > 0.0f) { // Dragging down
        _cam->moveBackward(deltaY * zoomFactor);
      }
    } else {
      // rotate the camera by the distance the mouse moved
      float theta = (currMousePosition.x - _mousePosition.x) * 0.005f;
      float phi = (currMousePosition.y - _mousePosition.y) * 0.005f;
      _cam->rotate(theta, phi);
    }
  }

  // update the mouse position
  _mousePosition = currMousePosition;
}

//*************************************************************************************
//
// Public Helpers

/// \desc Simple helper function to return a random number between 0.0f
/// and 1.0f.
GLfloat getRand() {
  return static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
}

//*************************************************************************************
//
// Engine Setup

void FPEngine::mSetupGLFW() {
  CSCI441::OpenGLEngine::mSetupGLFW();

  // set our callbacks
  glfwSetKeyCallback(mpWindow, mp_engine_keyboard_callback);
  glfwSetMouseButtonCallback(mpWindow, mp_engine_mouse_button_callback);
  glfwSetCursorPosCallback(mpWindow, mp_engine_cursor_callback);
}

void FPEngine::mSetupOpenGL() {
  glEnable(GL_DEPTH_TEST); // enable depth testing
  glDepthFunc(GL_LESS);    // use less than depth test

  glEnable(GL_BLEND); // enable blending
  glBlendFunc(GL_SRC_ALPHA,
              GL_ONE_MINUS_SRC_ALPHA); // use one minus blending equation

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // clear the frame buffer to gray
}

void FPEngine::mSetupShaders() {
  _lightingShaderProgram =
      new CSCI441::ShaderProgram("shaders/mp.v.glsl", "shaders/mp.f.glsl");
  _lightingShaderUniformLocations.mvpMatrix =
      _lightingShaderProgram->getUniformLocation("mvpMatrix");
  _lightingShaderUniformLocations.materialColor =
      _lightingShaderProgram->getUniformLocation("materialColor");
  // TODO #3A: assign uniforms
  _lightingShaderUniformLocations.lightDirection =
      _lightingShaderProgram->getUniformLocation("lightDirection");
  _lightingShaderUniformLocations.lightPosition =
      _lightingShaderProgram->getUniformLocation("lightPosition");
  _lightingShaderUniformLocations.spotLightPosition =
      _lightingShaderProgram->getUniformLocation("spotLightPosition");
  _lightingShaderUniformLocations.spotLightDirection =
      _lightingShaderProgram->getUniformLocation("spotLightDirection");
  _lightingShaderUniformLocations.lightColor =
      _lightingShaderProgram->getUniformLocation("lightColor");
  _lightingShaderUniformLocations.spotLightColor =
      _lightingShaderProgram->getUniformLocation("spotLightColor");
  _lightingShaderUniformLocations.pointLightColor =
      _lightingShaderProgram->getUniformLocation("pointLightColor");
  _lightingShaderUniformLocations.normalMatrix =
      _lightingShaderProgram->getUniformLocation("normalMatrix");
  _lightingShaderUniformLocations.modelMatrix =
      _lightingShaderProgram->getUniformLocation("modelMatrix");
  _lightingShaderUniformLocations.cameraPosition =
      _lightingShaderProgram->getUniformLocation("cameraPosition");

  _lightingShaderAttributeLocations.vPos =
      _lightingShaderProgram->getAttributeLocation("vPos");
  // TODO #3B: assign attributes
  _lightingShaderAttributeLocations.vNormal =
      _lightingShaderProgram->getAttributeLocation("vNormal");

  _elsterShaderProgram = new CSCI441::ShaderProgram("shaders/elster.v.glsl",
                                                    "shaders/elster.f.glsl");

  // get uniform locations
  _elsterShaderUniformLocations.mvpMatrix =
      _elsterShaderProgram->getUniformLocation("mvpMatrix");
  _elsterShaderUniformLocations.normalMatrix =
      _elsterShaderProgram->getUniformLocation("normalMatrix");
  _elsterShaderUniformLocations.modelMatrix =
      _elsterShaderProgram->getUniformLocation("modelMatrix");
  _elsterShaderUniformLocations.viewMatrix =
      _elsterShaderProgram->getUniformLocation("viewMatrix");
  _elsterShaderUniformLocations.materialDiffuse =
      _elsterShaderProgram->getUniformLocation("materialDiffuse");
  _elsterShaderUniformLocations.materialSpecular =
      _elsterShaderProgram->getUniformLocation("materialSpecular");
  _elsterShaderUniformLocations.materialShininess =
      _elsterShaderProgram->getUniformLocation("materialShininess");
  _elsterShaderUniformLocations.lightDirection =
      _elsterShaderProgram->getUniformLocation("lightDirection");
  _elsterShaderUniformLocations.lightPosition =
      _elsterShaderProgram->getUniformLocation("lightPosition");
  _elsterShaderUniformLocations.spotLightPosition =
      _elsterShaderProgram->getUniformLocation("spotLightPosition");
  _elsterShaderUniformLocations.spotLightDirection =
      _elsterShaderProgram->getUniformLocation("spotLightDirection");
  _elsterShaderUniformLocations.spotLightColor =
      _elsterShaderProgram->getUniformLocation("spotLightColor");
  _elsterShaderUniformLocations.pointLightColor =
      _elsterShaderProgram->getUniformLocation("pointLightColor");
  _elsterShaderUniformLocations.lightColor =
      _elsterShaderProgram->getUniformLocation("lightColor");
  _elsterShaderUniformLocations.cameraPosition =
      _elsterShaderProgram->getUniformLocation("cameraPosition");
  _elsterShaderUniformLocations.ambientLight =
      _elsterShaderProgram->getUniformLocation("ambientLight");
  _elsterShaderUniformLocations.useSkinning =
      _elsterShaderProgram->getUniformLocation("useSkinning");
  _elsterShaderUniformLocations.jointMatrices =
      _elsterShaderProgram->getUniformLocation("jointMatrices");

  // get attribute locations
  _elsterShaderAttributeLocations.vPos =
      _elsterShaderProgram->getAttributeLocation("vPos");
  _elsterShaderAttributeLocations.vNormal =
      _elsterShaderProgram->getAttributeLocation("vNormal");
  _elsterShaderAttributeLocations.vJoints =
      _elsterShaderProgram->getAttributeLocation("vJoints");
  _elsterShaderAttributeLocations.vWeights =
      _elsterShaderProgram->getAttributeLocation("vWeights");

  _tympaniusShaderProgram = new CSCI441::ShaderProgram("shaders/tympMP.v.glsl",
                                                       "shaders/tympMP.f.glsl");

  // Set Uniform Locations
  _tympaniusShaderUniformLocations.mvpMatrix =
      _tympaniusShaderProgram->getUniformLocation("mvpMatrix");
  _tympaniusShaderUniformLocations.normalMatrix =
      _tympaniusShaderProgram->getUniformLocation("normalMatrix");
  _tympaniusShaderUniformLocations.modelMatrix =
      _tympaniusShaderProgram->getUniformLocation("modelMatrix");

  _tympaniusShaderUniformLocations.lightDir =
      _tympaniusShaderProgram->getUniformLocation("lightDir");
    _tympaniusShaderUniformLocations.lightPosition =
      _tympaniusShaderProgram->getUniformLocation("lightPosition");
    _tympaniusShaderUniformLocations.spotLightPosition =
        _tympaniusShaderProgram->getUniformLocation("spotLightPosition");
    _tympaniusShaderUniformLocations.spotLightDirection =
        _tympaniusShaderProgram->getUniformLocation("spotLightDirection");
    _tympaniusShaderUniformLocations.spotLightColor =
        _tympaniusShaderProgram->getUniformLocation("spotLightColor");
    _tympaniusShaderUniformLocations.pointLightColor =
        _tympaniusShaderProgram->getUniformLocation("pointLightColor");
  _tympaniusShaderUniformLocations.lightColor =
      _tympaniusShaderProgram->getUniformLocation("lightColor");
  _tympaniusShaderUniformLocations.ambientLightColor =
      _tympaniusShaderProgram->getUniformLocation("ambientLightColor");

  _tympaniusShaderUniformLocations.cameraPos =
      _tympaniusShaderProgram->getUniformLocation("cameraPos");
  _tympaniusShaderUniformLocations.shininessAlpha =
      _tympaniusShaderProgram->getUniformLocation("shininessAlpha");
  _tympaniusShaderUniformLocations.materialColor =
      _tympaniusShaderProgram->getUniformLocation("materialColor");

  _tympaniusShaderUniformLocations.textureMap =
      _tympaniusShaderProgram->getUniformLocation("textureMap");
  _tympaniusShaderUniformLocations.isTextured =
      _tympaniusShaderProgram->getUniformLocation("isTextured");

  // Set Attribute Locations
  _tympaniusShaderAttributeLocations.vPos =
      _tympaniusShaderProgram->getAttributeLocation("vPos");
  _tympaniusShaderAttributeLocations.vNormal =
      _tympaniusShaderProgram->getAttributeLocation("vNormal");
  _tympaniusShaderAttributeLocations.texCoord =
      _tympaniusShaderProgram->getAttributeLocation("vTexPos");

  // load tess shader for ground
  _groundTessShaderProgram = new CSCI441::ShaderProgram(
      "shaders/ground.v.glsl", "shaders/ground.tcs.glsl",
      "shaders/ground.tes.glsl", "shaders/ground.f.glsl");

  // get uniform locations for ground tess shader
  _groundTessShaderUniformLocations.mvpMatrix =
      _groundTessShaderProgram->getUniformLocation("mvpMatrix");
  _groundTessShaderUniformLocations.modelMatrix =
      _groundTessShaderProgram->getUniformLocation("modelMatrix");
  _groundTessShaderUniformLocations.normalMatrix =
      _groundTessShaderProgram->getUniformLocation("normalMatrix");
  _groundTessShaderUniformLocations.groundTexture =
      _groundTessShaderProgram->getUniformLocation("groundTexture");
  _groundTessShaderUniformLocations.tessLevel =
      _groundTessShaderProgram->getUniformLocation("tessLevel");
  _groundTessShaderUniformLocations.hillHeight =
      _groundTessShaderProgram->getUniformLocation("hillHeight");
  _groundTessShaderUniformLocations.lightDirection =
      _groundTessShaderProgram->getUniformLocation("lightDirection");
  _groundTessShaderUniformLocations.lightColor =
      _groundTessShaderProgram->getUniformLocation("lightColor");
  _groundTessShaderUniformLocations.lightPosition =
      _groundTessShaderProgram->getUniformLocation("lightPosition");
  _groundTessShaderUniformLocations.pointLightColor =
      _groundTessShaderProgram->getUniformLocation("pointLightColor");
  _groundTessShaderUniformLocations.spotLightPosition =
      _groundTessShaderProgram->getUniformLocation("spotLightPosition");
  _groundTessShaderUniformLocations.spotLightDirection =
      _groundTessShaderProgram->getUniformLocation("spotLightDirection");
  _groundTessShaderUniformLocations.spotLightColor =
      _groundTessShaderProgram->getUniformLocation("spotLightColor");
  _groundTessShaderUniformLocations.cameraPosition =
      _groundTessShaderProgram->getUniformLocation("cameraPosition");

  // get attribute locations for ground tess shader
  _groundTessShaderAttributeLocations.vPos =
      _groundTessShaderProgram->getAttributeLocation("vPos");
  _groundTessShaderAttributeLocations.vNormal =
      _groundTessShaderProgram->getAttributeLocation("vNormal");
  _groundTessShaderAttributeLocations.vTexCoord =
      _groundTessShaderProgram->getAttributeLocation("vTexCoord");

  // load sprite shader for enemies, coins, and particles
  _spriteShaderProgram = new CSCI441::ShaderProgram("shaders/sprite.v.glsl",
                                                    "shaders/sprite.f.glsl");

  // get uniform locations for sprite shader
  _spriteShaderUniformLocations.mvpMatrix =
      _spriteShaderProgram->getUniformLocation("mvpMatrix");
  _spriteShaderUniformLocations.spriteTexture =
      _spriteShaderProgram->getUniformLocation("spriteTexture");

    // texture shader
    _textureShaderProgram = new CSCI441::ShaderProgram("shaders/lab06.v.glsl", "shaders/lab06.f.glsl" );
    // query uniform locations
    _textureShaderUniformLocations.mvpMatrix      = _textureShaderProgram->getUniformLocation("mvpMatrix");
    _textureShaderUniformLocations.modelMatrix      = _textureShaderProgram->getUniformLocation("modelMatrix");
    _textureShaderUniformLocations.normalMatrix      = _textureShaderProgram->getUniformLocation("normalMatrix");
    
    // TODO #12A - texture map
    _textureShaderUniformLocations.texMap      = _textureShaderProgram->getUniformLocation("textureMap");

    
  _textureShaderUniformLocations.spotLightPosition =
      _textureShaderProgram->getUniformLocation("spotLightPosition");
  _textureShaderUniformLocations.spotLightDirection =
      _textureShaderProgram->getUniformLocation("spotLightDirection");
  _textureShaderUniformLocations.spotLightColor =
      _textureShaderProgram->getUniformLocation("spotLightColor");

   _textureShaderUniformLocations.cameraPosition =
      _textureShaderProgram->getUniformLocation("cameraPosition");

    // set static uniforms
    // TODO #13 - set uniform
    _textureShaderProgram->setProgramUniform(_textureShaderUniformLocations.texMap, 0);

    _textureShaderAttributeLocations.vPos =
    _textureShaderProgram->getAttributeLocation("vPos");
  _textureShaderAttributeLocations.vNormal =
      _textureShaderProgram->getAttributeLocation("vNormal");
  _textureShaderAttributeLocations.texCoord =
      _textureShaderProgram->getAttributeLocation("vTexCoord");

    CSCI441::setVertexAttributeLocations(_textureShaderAttributeLocations.vPos,_textureShaderAttributeLocations.vNormal,
                                         _textureShaderAttributeLocations.texCoord);
}

void FPEngine::mSetupTextures() {
  // TODO #09 - load textures
  _texHandles[TEXTURE_ID::GROUND] =
      _loadAndRegisterTexture("./assets/textures/floor.png");
  _texHandles[TEXTURE_ID::WALL] = 
      _loadAndRegisterTexture("./assets/textures/bricks.png");
  _texHandles[TEXTURE_ID::ENEMY] =
      _loadAndRegisterTexture("assets/textures/goomba.png");
  _texHandles[TEXTURE_ID::COIN] =
      _loadAndRegisterTexture("assets/textures/coin.png");
  _texHandles[TEXTURE_ID::PARTICLE] =
      _loadAndRegisterTexture("assets/textures/sonic_coin.png");
}

void FPEngine::mSetupBuffers() {
  // TODO #4: need to connect our 3D Object Library to our shader
  CSCI441::setVertexAttributeLocations(
      _lightingShaderAttributeLocations.vPos,
      _lightingShaderAttributeLocations.vNormal);

  _createGroundBuffers();
  _generateEnvironment();
}

void FPEngine::_createGroundBuffers() {
  struct VertexNormalTextured {
    glm::vec3 position;
    glm::vec3 vNormal;
    glm::vec2 texCoord;
  };

  // asingle patch covering the world
  // scale by WORLD_SIZE for proper coverage
  const float size = WORLD_SIZE;
  constexpr VertexNormalTextured groundPatch[4] = {
      {{-1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
      {{1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{-1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
      {{1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}};

  // scale positions to world size
  VertexNormalTextured scaledPatch[4];
  for (int i = 0; i < 4; i++) {
    scaledPatch[i].position = groundPatch[i].position * size;
    scaledPatch[i].vNormal = groundPatch[i].vNormal;
    scaledPatch[i].texCoord = groundPatch[i].texCoord * 10.0f;
  }

  _numGroundPoints = 4;

  glGenVertexArrays(1, &_groundVAO);
  glBindVertexArray(_groundVAO);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(scaledPatch), scaledPatch,
               GL_STATIC_DRAW);

  // vertex attribs for tess shader
  // pos
  glEnableVertexAttribArray(_groundTessShaderAttributeLocations.vPos);
  glVertexAttribPointer(_groundTessShaderAttributeLocations.vPos, 3, GL_FLOAT,
                        GL_FALSE, sizeof(VertexNormalTextured),
                        (void *)offsetof(VertexNormalTextured, position));

  // norms
  glEnableVertexAttribArray(_groundTessShaderAttributeLocations.vNormal);
  glVertexAttribPointer(_groundTessShaderAttributeLocations.vNormal, 3,
                        GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextured),
                        (void *)offsetof(VertexNormalTextured, vNormal));

  // tex coords
  glEnableVertexAttribArray(_groundTessShaderAttributeLocations.vTexCoord);
  glVertexAttribPointer(_groundTessShaderAttributeLocations.vTexCoord, 2,
                        GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextured),
                        (void *)offsetof(VertexNormalTextured, texCoord));

  // Set patch size for tess
  glPatchParameteri(GL_PATCH_VERTICES, 4);

  fprintf(stdout,
          "[INFO]: ground tessellation patch created with VAO/VBO %d/%d & %d "
          "control points\n",
          _groundVAO, vbo, _numGroundPoints);
}

void FPEngine::mSetupScene() {
  // Create and position the arcball camera - at character height
  _arcBallCam = new CSCI441::ArcballCam();
  _arcBallCam->setPosition(glm::vec3(0.0f, 40.0f, 30.0f));
  _arcBallCam->setLookAtPoint(glm::vec3(0.0f, 35.0f, 0.0f));
  _arcBallCam->recomputeOrientation();

  _pSkybox = new Skybox();

  _pCharacter = new Character(_elsterShaderProgram->getShaderProgramHandle(),
                              _elsterShaderUniformLocations.mvpMatrix,
                              _elsterShaderUniformLocations.normalMatrix,
                              _elsterShaderUniformLocations.modelMatrix,
                              _elsterShaderUniformLocations.materialDiffuse,
                              _elsterShaderUniformLocations.materialSpecular,
                              _elsterShaderUniformLocations.materialShininess);

  if (!_pCharacter->loadFromFile("assets/models/heroes/Elster/elster.glb")) {
    fprintf(stderr, "Failed to load character model\n");
  }

  // Position character at center of mountain, slightly above terrain
  // The terrain height at (0, 0) is approximately 33.75 units (0.6 *
  // hillHeight)
  _pCharacter->setPosition(glm::vec3(70.0f, _getTerrainHeight(20.0f, 0.0f), 0.0f));

  _pTympanius = new Tympanius(_tympaniusShaderProgram, &_tympaniusShaderUniformLocations, &_tympaniusShaderAttributeLocations);
  _pTympanius->setPosition(glm::vec3(4.0f, _getTerrainHeight(4.0f, 4.0f), 4.0f));
  _pTympanius->setFloorHeight(_getTerrainHeight(_pTympanius->getPosition().x, _pTympanius->getPosition().z)); // Because setPosition also adds the floating height to the position

  // Create and position the first-person camera at character position
  _firstPersonCam = new CSCI441::FreeCam();
  const float EYE_HEIGHT = 5.0f;  // Increased height for better first-person view
  _firstPersonCam->setPosition(glm::vec3(0.0f, 36.0f + EYE_HEIGHT, 0.0f));
  _firstPersonCam->setTheta(-_pCharacter->getHeading());  // Face same direction as character
  _firstPersonCam->setPhi(_cameraPitch);  // Use stored pitch (initialized to look horizontally)
  _firstPersonCam->recomputeOrientation();

  // Set the active camera to first-person (this is now the only camera)
  _cam = _firstPersonCam;

  _cameraSpeed = glm::vec2(0.25f, 0.02f);

  _pWilfred = new Wilfred(_lightingShaderProgram->getShaderProgramHandle(),
                          _lightingShaderUniformLocations.mvpMatrix,
                          _lightingShaderUniformLocations.normalMatrix,
                          _lightingShaderUniformLocations.materialColor,
                          _lightingShaderUniformLocations.modelMatrix);
    _pWilfred->setPosition(glm::vec3(10.0f, _getTerrainHeight(10.0f, 10.0f), 10.0f));

  // enemy Elster
  _pEnemyElster =
      new Character(_elsterShaderProgram->getShaderProgramHandle(),
                    _elsterShaderUniformLocations.mvpMatrix,
                    _elsterShaderUniformLocations.normalMatrix,
                    _elsterShaderUniformLocations.modelMatrix,
                    _elsterShaderUniformLocations.materialDiffuse,
                    _elsterShaderUniformLocations.materialSpecular,
                    _elsterShaderUniformLocations.materialShininess);

  // Farina
  _pFarina = new Farina(_lightingShaderProgram->getShaderProgramHandle(),
                        _lightingShaderUniformLocations.mvpMatrix,
                        _lightingShaderUniformLocations.materialColor,
                        _lightingShaderUniformLocations.normalMatrix,
                        _lightingShaderUniformLocations.modelMatrix);

  _pFarina->setPosition(glm::vec3(-15.0f, 25.0f, -15.0f));

  if (!_pEnemyElster->loadFromFile(
          "./assets/models/heroes/Elster/elster.glb")) {
    fprintf(stderr, "Failed to load enemy Elster model\n");
  }

  // starting pos for enemy elster
  float elsterStartX = -10.0f;
  float elsterStartZ = -10.0f;
  float elsterStartY = _getTerrainHeight(elsterStartX, elsterStartZ) + 1.0f;
  _pEnemyElster->setPosition(
      glm::vec3(elsterStartX, elsterStartY, elsterStartZ));

  // enemy elster to use walking animation
  _pEnemyElster->playAnimation("elsterWalking");

  // Set lighting parameters
  _setLightingParameters();

  // Initialize particle system
  _particleSystem = new ParticleSystem();

  // Spawn enemies
  _spawnEnemies(0); // Spawn 10 enemies

  // Spawn coins at corners
  _spawnCoins();
}

void FPEngine::_setLightingParameters() {
  // TODO #6: set lighting uniforms
  const glm::vec3 lightPosition = glm::vec3(1.0f, 0.0f, 1.0f);
  const glm::vec3 spotLightPosition = _firstPersonCam->getPosition();
  const glm::vec3 spotLightDirection =
      glm::normalize(_firstPersonCam->getLookAtPoint() - spotLightPosition);
  _lightingShaderProgram->useProgram();
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.lightDirection, lightDirection);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.lightPosition, lightPosition);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.spotLightPosition, spotLightPosition);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.spotLightDirection, spotLightDirection);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.lightColor, lightColor);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.spotLightColor, spotLightColor);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.pointLightColor, pointLightColor);

  const glm::vec3 ambientLightColor = glm::vec3(0.71, 0.54, 0.7);

  _elsterShaderProgram->useProgram();
  _elsterShaderProgram->setProgramUniform(
      _elsterShaderUniformLocations.lightDirection, lightDirection);
  _elsterShaderProgram->setProgramUniform(
      _elsterShaderUniformLocations.lightColor, lightColor);
  _elsterShaderProgram->setProgramUniform(
      _elsterShaderUniformLocations.lightPosition, lightPosition);
  _elsterShaderProgram->setProgramUniform(
      _elsterShaderUniformLocations.spotLightPosition, spotLightPosition);
  _elsterShaderProgram->setProgramUniform(
      _elsterShaderUniformLocations.spotLightDirection, spotLightDirection);
  _elsterShaderProgram->setProgramUniform(
      _elsterShaderUniformLocations.spotLightColor, spotLightColor);
  _elsterShaderProgram->setProgramUniform(
      _elsterShaderUniformLocations.pointLightColor, pointLightColor);
  _elsterShaderProgram->setProgramUniform(
      _elsterShaderUniformLocations.ambientLight, ambientLightColor);

  // Tympanius lighting
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.lightDir, lightDirection);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.lightPosition, lightPosition);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.spotLightPosition, spotLightPosition);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.spotLightDirection, spotLightDirection);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.lightColor, lightColor);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.spotLightColor, spotLightColor);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.pointLightColor, pointLightColor);

  // set lighting for ground tess shader
  _groundTessShaderProgram->useProgram();
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.lightDirection, lightDirection);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.lightColor, lightColor);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.lightPosition, lightPosition);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.spotLightPosition, spotLightPosition);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.spotLightDirection, spotLightDirection);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.spotLightColor, spotLightColor);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.pointLightColor, pointLightColor);

  _textureShaderProgram->useProgram();
  _textureShaderProgram->setProgramUniform(
      _textureShaderUniformLocations.spotLightPosition, spotLightPosition);
  _textureShaderProgram->setProgramUniform(
      _textureShaderUniformLocations.spotLightDirection, spotLightDirection);
  _textureShaderProgram->setProgramUniform(
      _textureShaderUniformLocations.spotLightColor, spotLightColor);
}

//*************************************************************************************
//
// Engine Cleanup

void FPEngine::mCleanupShaders() {
  fprintf(stdout, "[INFO]: ...deleting Shaders.\n");
  delete _lightingShaderProgram;
  _lightingShaderProgram = nullptr;
  delete _elsterShaderProgram;
  _elsterShaderProgram = nullptr;
  delete _groundTessShaderProgram;
  _groundTessShaderProgram = nullptr;
  delete _spriteShaderProgram;
  _spriteShaderProgram = nullptr;
    delete _textureShaderProgram;
    _textureShaderProgram = nullptr;
}

void FPEngine::mCleanupBuffers() {
  fprintf(stdout, "[INFO]: ...deleting VAOs....\n");
  CSCI441::deleteObjectVAOs();
  glDeleteVertexArrays(1, &_groundVAO);
  _groundVAO = 0;

  fprintf(stdout, "[INFO]: ...deleting VBOs....\n");
  CSCI441::deleteObjectVBOs();

  delete _pWilfred;
  _pWilfred = nullptr;
}

void FPEngine::mCleanupScene() {
  fprintf(stdout, "[INFO]: ...deleting cameras..\n");
  // Set _cam to nullptr to avoid using a dangling pointer
  // The actual cameras are deleted in the destructor
  _cam = nullptr;
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

void FPEngine::_generateEnvironment() {
  //******************************************************************
  // parameters to make up our grid size and spacing, feel free to
  // play around with this
  constexpr GLfloat GRID_WIDTH = WORLD_SIZE * 2.0f;
  constexpr GLfloat GRID_LENGTH = WORLD_SIZE * 2.0f;
  constexpr GLfloat GRID_SPACING_WIDTH = 2.0f;
  constexpr GLfloat GRID_SPACING_LENGTH = 2.0f;
  // precomputed parameters based on above
  constexpr GLfloat LEFT_END_POINT = -GRID_WIDTH / 2.0f + 4.0f;
  constexpr GLfloat RIGHT_END_POINT = GRID_WIDTH / 2.0f - 2.0f;
  constexpr GLfloat BOTTOM_END_POINT = -GRID_LENGTH / 2.0f + 4.0f;
  constexpr GLfloat TOP_END_POINT = GRID_LENGTH / 2.0f - 2.0f;
  //******************************************************************

  srand(time(0)); // seed our RNG

  // coin corner positions
  const float coinOffset = WORLD_SIZE * 0.8f;
  const glm::vec2 coinCorners[4] = {
      glm::vec2(-coinOffset, -coinOffset), glm::vec2(coinOffset, -coinOffset),
      glm::vec2(-coinOffset, coinOffset), glm::vec2(coinOffset, coinOffset)};
    // read in map file for bushes
    std::string line;
    std::string square;
    // open file
    std::ifstream inputFile("map.txt");

    // Check if the file was opened successfully
    if (inputFile.is_open()) {
        int row = 0, col = 0;
        // Use a while loop with std::getline to read the file line by line
        float width = (RIGHT_END_POINT - LEFT_END_POINT)/28;
        float height = (TOP_END_POINT - BOTTOM_END_POINT)/28;
        while (std::getline(inputFile, line)) {
            col=0;
            std::istringstream lineStream(line);
            while (lineStream >> square) {
                if (square == "x") {
                    // put a wall here
                    BushData bush;
                    bush.size = width/2;
                    float bushX = LEFT_END_POINT+(col*width);
                    float bushZ = BOTTOM_END_POINT+(row*height);
                    float terrainY = _getTerrainHeight(bushX, bushZ);
                    // bush sits on the terrain
                    bush.position = glm::vec3(bushX, terrainY + bush.size, bushZ);
                    bush.color = glm::vec3(0.086 + (getRand() - 2) * 0.15,
                                           0.588 + (getRand() - 2) * 0.15,
                                           0.455 + (getRand() - 2) * 0.15);
                    _bushes.push_back(bush);
                }
                col++;
            }
            row++;
        }
        inputFile.close(); // Close the file stream
    } else {
        std::cerr << "Unable to open file" << std::endl;
    }

  // psych! everything's on a grid.
  for (int i = LEFT_END_POINT; i < RIGHT_END_POINT; i += GRID_SPACING_WIDTH) {
    for (int j = BOTTOM_END_POINT; j < TOP_END_POINT;
         j += GRID_SPACING_LENGTH) {
      // don't just draw a tree ANYWHERE.
      if (getRand() < 0.1f) {
        // Calculate distance from center
        float distFromCenterX = abs(i);
        float distFromCenterZ = abs(j);

        // Skip too close to spawn
        if (distFromCenterX < 10.0f && distFromCenterZ < 10.0f) {
          continue;
        }

        // Skip too close to coin
        bool nearCoin = false;
        for (const auto &corner : coinCorners) {
          float distToCoin = glm::length(glm::vec2(i, j) - corner);
          if (distToCoin < 10.0f) {
            nearCoin = true;
            break;
          }
        }
        if (nearCoin) {
          continue;
        }

        // if (getRand() < 0.5f) {
        //   BushData bush;
        //   bush.size = 2.0f;
        //   float bushX = i + getRand() - 2;
        //   float bushZ = j + getRand() - 2;
        //   float terrainY = _getTerrainHeight(bushX, bushZ);
        //   // bush sits on the terrain
        //   bush.position = glm::vec3(bushX, terrainY + bush.size, bushZ);
        //   bush.color = glm::vec3(0.086 + (getRand() - 2) * 0.15,
        //                          0.588 + (getRand() - 2) * 0.15,
        //                          0.455 + (getRand() - 2) * 0.15);
        //   _bushes.push_back(bush);
        //   continue;
        // }

        // translate to spot
        float treeX = i + getRand() - 2;
        float treeZ = j + getRand() - 2;
        float terrainY = _getTerrainHeight(treeX, treeZ);

        // compute random height
        GLdouble height = powf(getRand(), 2.5) * 15 + 10;

        // need to place it at terrain height
        // and scale it to the desired height
        glm::mat4 transToSpotMtx =
            glm::translate(glm::mat4(1.0), glm::vec3(treeX, terrainY, treeZ));
        glm::mat4 scaleToHeightMtx =
            glm::scale(glm::mat4(1.0), glm::vec3(1, height, 1));

        // translate up to position leaves at top of trunk
        glm::mat4 transToHeight =
            glm::translate(glm::mat4(1.0), glm::vec3(0, height * 0.2f, 0));

        // compute full model matrix
        glm::mat4 leavesModelMatrix =
            transToHeight * scaleToHeightMtx * transToSpotMtx;
        glm::mat4 trunkModelMatrix = scaleToHeightMtx * transToSpotMtx;

        // compute random colors
        glm::vec3 color(0.086 + (getRand() - 2) * 0.15,
                        0.588 + (getRand() - 2) * 0.15,
                        0.455 + (getRand() - 2) * 0.15);
        glm::vec3 barkColor(0.49 + (getRand() - 2) * 0.1,
                            0.439 + (getRand() - 2) * 0.1,
                            0.251 + (getRand() - 2) * 0.1);

        // get random offset for the swaying
        float frameOffset(getRand() * M_PI);

        // store tree properties
        TreeData currentTree = {leavesModelMatrix, trunkModelMatrix, color,
                                barkColor, frameOffset};
        _trees.emplace_back(currentTree);
      }
    }
  }
}

void FPEngine::_renderScene(const glm::mat4 &viewMtx, const glm::mat4 &projMtx,
                            const glm::vec3 &cameraPos) const {
  _pSkybox->draw(viewMtx, projMtx);
  

  const glm::vec3 lightPosition = glm::vec3(1.0f, 0.0f, 1.0f);
  const glm::vec3 spotLightPosition = _firstPersonCam->getPosition();
  const glm::vec3 spotLightDirection = glm::normalize(_firstPersonCam->getLookAtPoint() - spotLightPosition);

  // tess ground
  _groundTessShaderProgram->useProgram();

  // transformation matrices
  glm::mat4 groundModelMtx = glm::mat4(1.0f);
  glm::mat4 mvpMtx = projMtx * viewMtx * groundModelMtx;
  glm::mat3 normalMtx = glm::transpose(glm::inverse(glm::mat3(groundModelMtx)));

  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.mvpMatrix, mvpMtx);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.modelMatrix, groundModelMtx);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.normalMatrix, normalMtx);

  // tess parameters
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.tessLevel, 32.0f);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.hillHeight, 56.25f);

  // Set lighting uniforms
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.lightDirection,
      lightDirection);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.lightColor,
      lightColor);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.lightPosition,
      lightPosition);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.pointLightColor,
      pointLightColor);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.spotLightPosition,
      spotLightPosition);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.spotLightDirection,
     spotLightDirection);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.spotLightColor,
      spotLightColor);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.cameraPosition, cameraPos);

  // Bind ground texture
  glBindTexture(GL_TEXTURE_2D, _texHandles[TEXTURE_ID::GROUND]);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.groundTexture, 0);

  // Draw ground patches
  glBindVertexArray(_groundVAO);
  glDrawArrays(GL_PATCHES, 0, _numGroundPoints);

  // to character shader
  _elsterShaderProgram->useProgram();

  // camera position for lighting calculations
  _elsterShaderProgram->setProgramUniform(
      _elsterShaderUniformLocations.cameraPosition, cameraPos);

  // In first-person view no need for the chraracter to get rendered
  // TODO: set up the hands :o
  if (!_characterDead && _cam != _firstPersonCam) {
    glUniform1i(_elsterShaderUniformLocations.useSkinning, true);
    _pCharacter->draw(glm::mat4(1.0f), viewMtx, projMtx);
  }

  // draw enemy Elster
  glUniform1i(_elsterShaderUniformLocations.useSkinning, true);
  _pEnemyElster->draw(glm::mat4(1.0f), viewMtx, projMtx);

  // lighting shader
  _lightingShaderProgram->useProgram();

  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.lightDirection, lightDirection);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.lightPosition, lightPosition);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.spotLightPosition, spotLightPosition);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.spotLightDirection, spotLightDirection);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.lightColor, lightColor);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.spotLightColor, spotLightColor);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.pointLightColor, pointLightColor);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.cameraPosition, cameraPos);

  /// OLD MAN TIME
  glm::mat4 wilfredModelMtx(1.0f);
  _pWilfred->_animateBro(); // get this man an animation
  _pWilfred->drawWilfred(wilfredModelMtx, viewMtx, projMtx);
  /// OLD MAN NO MORE

  // Farina
  _pFarina->draw(glm::mat4(1.0f), viewMtx, projMtx);

  _textureShaderProgram->useProgram();

  _textureShaderProgram->setProgramUniform(
      _textureShaderUniformLocations.spotLightPosition, spotLightPosition);
  _textureShaderProgram->setProgramUniform(
      _textureShaderUniformLocations.spotLightDirection, spotLightDirection);
  _textureShaderProgram->setProgramUniform(
      _textureShaderUniformLocations.spotLightColor, spotLightColor);
  _lightingShaderProgram->setProgramUniform(
      _textureShaderUniformLocations.cameraPosition, cameraPos);
  
    CSCI441::setVertexAttributeLocations(_textureShaderAttributeLocations.vPos, _textureShaderAttributeLocations.vNormal, _textureShaderAttributeLocations.texCoord);

    glBindTexture(GL_TEXTURE_2D, _texHandles[TEXTURE_ID::WALL]);

  for (const auto &bush : _bushes) {
    glm::mat4 bushModelMtx = glm::translate(glm::mat4(1.0f), bush.position);
    bushModelMtx = glm::scale(bushModelMtx, glm::vec3(bush.size, bush.size*4.0f, bush.size));
    glm::mat4 TmvpMtx = projMtx * viewMtx * bushModelMtx;
    const glm::mat3 normalMtx = glm::transpose(glm::inverse(glm::mat3(bushModelMtx)));

    _textureShaderProgram->setProgramUniform(_textureShaderUniformLocations.mvpMatrix, TmvpMtx);
    _textureShaderProgram->setProgramUniform(_textureShaderUniformLocations.modelMatrix, bushModelMtx);
    _textureShaderProgram->setProgramUniform(_textureShaderUniformLocations.normalMatrix, normalMtx);
    
    
    //_lightingShaderProgram->setProgramUniform(
        //_lightingShaderUniformLocations.materialColor, bush.color);

      CSCI441::drawSolidCubeTextured(2.0f);
  }
  
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.lightDir, lightDirection);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.lightPosition, lightPosition);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.spotLightPosition, spotLightPosition);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.spotLightDirection, spotLightDirection);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.spotLightColor, spotLightColor);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.lightColor, lightColor);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.pointLightColor, pointLightColor);
  _tympaniusShaderProgram->setProgramUniform(
      _tympaniusShaderUniformLocations.cameraPos, cameraPos);
  
    if (_pTympanius->isAlive()) { 
      _pTympanius->draw(glm::mat4(1.0f), viewMtx, projMtx);
    }

  // coins
  for (auto coin : _coins) {
    if (!coin->isCollected()) {
      coin->draw(_spriteShaderProgram->getShaderProgramHandle(),
                 _spriteShaderUniformLocations.mvpMatrix,
                 _spriteShaderUniformLocations.spriteTexture, viewMtx, projMtx,
                 _texHandles[TEXTURE_ID::COIN]);
    }
  }

  // particles
  _particleSystem->draw(_spriteShaderProgram->getShaderProgramHandle(),
                        _spriteShaderUniformLocations.mvpMatrix,
                        _spriteShaderUniformLocations.spriteTexture, viewMtx,
                        projMtx, _texHandles[TEXTURE_ID::PARTICLE]);

}

void FPEngine::_updateScene() {
  // Calculate delta time for character animations
  static float lastTime = 0.0f;
  float currentTime = static_cast<float>(glfwGetTime());
  float deltaTime = currentTime - lastTime;
  lastTime = currentTime;

  bool moved = false;

  // Handle character movement (player is alive)
  if (!_characterDead) {
    // animation management
    static bool isWalking = false;

    // Forward/backward movement
    if (_keys[GLFW_KEY_W]) {
      _pCharacter->moveForward(_characterMoveSpeed * deltaTime);
      moved = true;
    }
    if (_keys[GLFW_KEY_S]) {
      _pCharacter->moveBackward(_characterMoveSpeed * deltaTime);
      moved = true;
    }

    if (_cam == _firstPersonCam) {
      glm::vec3 charPos = _pCharacter->getPosition();
      float heading = _pCharacter->getHeading();

      if (_keys[GLFW_KEY_A]) {
        glm::vec3 strafeDir(-cos(heading), 0.0f, sin(heading));
        charPos += strafeDir * _characterMoveSpeed * deltaTime;
        _pCharacter->setPosition(charPos);
        moved = true;
      }
      if (_keys[GLFW_KEY_D]) {
        glm::vec3 strafeDir(cos(heading), 0.0f, -sin(heading));
        charPos += strafeDir * _characterMoveSpeed * deltaTime;
        _pCharacter->setPosition(charPos);
        moved = true;
      }
    } else {
      // In other camera modes, A and D turn the character
      if (_keys[GLFW_KEY_A]) {
        _pCharacter->turnLeft(_characterTurnSpeed * deltaTime);
      }
      if (_keys[GLFW_KEY_D]) {
        _pCharacter->turnRight(_characterTurnSpeed * deltaTime);
      }
    }

    // Handle jumping with spacebar
    if (_keys[GLFW_KEY_SPACE] && _characterOnGround) {
      const float jumpVelocity = 15.0f; // Initial upward velocity for jump
      _characterVerticalVelocity = jumpVelocity;
      _characterOnGround = false;
    }

    if (moved) {
      if (!isWalking) {
        _pCharacter->playAnimation("elsterWalking");
        isWalking = true;
      }
    } else {
      if (isWalking) {
        _pCharacter->playAnimation("elsterIdle");
        isWalking = false;
      }
    }

    // gravity and follow terrain
    glm::vec3 charPos = _pCharacter->getPosition();

    // collision detection to current position
    const float CHARACTER_RADIUS = 0.5f;
    charPos = _checkAndResolveCollisions(charPos, CHARACTER_RADIUS);

    // terrain height at the characters current pos
    float terrainHeight = _getTerrainHeight(charPos.x, charPos.z);

    // if character can land on an object
    float objectHeight = _getObjectHeightAt(charPos.x, charPos.z);

    // gravity
    const float gravity = -20.0f; // m/s^2
    const float groundOffset =
        0.5f; // Offset to keep character slightly above ground

    // Use object height as the landing surface
    float surfaceHeight = terrainHeight;
    if (objectHeight > -500.0f && objectHeight > terrainHeight) {
      //surfaceHeight = objectHeight;
    }

    // if character should be on the ground
    if (surfaceHeight > -500.0f) {
      float targetHeight = surfaceHeight + groundOffset;
      const float groundTolerance = 0.2f;

      if (charPos.y > targetHeight + groundTolerance) {
        // apply gravity cause they're in the air
        _characterVerticalVelocity += gravity * deltaTime;
        charPos.y += _characterVerticalVelocity * deltaTime;

        // Check if landed
        if (charPos.y <= targetHeight) {
          charPos.y = targetHeight;
          _characterVerticalVelocity = 0.0f;
          _characterOnGround = true;
        } else {
          _characterOnGround = false;
        }
      } else if (_characterVerticalVelocity > 0.0f) {
        // Character is jumping, apply physics
        _characterVerticalVelocity += gravity * deltaTime;
        charPos.y += _characterVerticalVelocity * deltaTime;
        _characterOnGround = false;
      } else {
        // Character is on ground, snap to terrain
        charPos.y = targetHeight;
        _characterVerticalVelocity = 0.0f;
        _characterOnGround = true;
      }
    } else {
      // Character is out of bounds - falling to their death </3
      _characterOnGround = false;
      _characterVerticalVelocity += gravity * deltaTime;
      charPos.y += _characterVerticalVelocity * deltaTime;

      // die when fallen far enough below the world
      if (charPos.y < -50.0f && !_characterDead) {
        _characterDead = true;
        _particleSystem->spawnBurst(charPos, 30);
        fprintf(stdout, "[INFO]: Player fell off the edge! RIP!\n");
        fprintf(stdout, "[INFO]: Coins collected: %d / 4\n", _coinsCollected);
      }
    }

    // update character position
    _pCharacter->setPosition(charPos);
  }

  // Update character animations
  _pCharacter->update(deltaTime);

  // update enemies
  const float enemyTurnSpeed = 1.5f; // Radians per second

  // update enemy tympanius
  _pTympanius->update(deltaTime, _pCharacter->getPosition(), enemyTurnSpeed);
  glm::vec3 tympaniusPos = _pTympanius->getPosition();
  glm::vec3 newTympaniusPos = _checkAndResolveCollisions(tympaniusPos, _pTympanius->getRadius());
  _pTympanius->setPosition(newTympaniusPos);

    _pWilfred->update(deltaTime, _pCharacter->getPosition(), enemyTurnSpeed);
    glm::vec3 wilfPos = _pWilfred->getPosition();
    glm::vec3 newwilfPos = _checkAndResolveCollisions(glm::vec3(wilfPos.x, _getTerrainHeight(wilfPos.x, wilfPos.z) + 1.0f, wilfPos.z), 0.5f);
    _pWilfred->setPosition(glm::vec3(newwilfPos.x, wilfPos.y, newwilfPos.z));

  // update enemy elster
  _pEnemyElster->update(deltaTime, _pCharacter->getPosition(), enemyTurnSpeed);
  glm::vec3 elsterPos = _pEnemyElster->getPosition();
  float elsterTerrainHeight =
      _getTerrainHeight(elsterPos.x, elsterPos.z) + 1.0f;
  glm::vec3 newElsterPos = _checkAndResolveCollisions(
      glm::vec3(elsterPos.x, elsterTerrainHeight, elsterPos.z), 0.5f);
  _pEnemyElster->setPosition(
      glm::vec3(newElsterPos.x, elsterTerrainHeight, newElsterPos.z));

  // Farina
  _pFarina->update(deltaTime, _pCharacter->getPosition(), enemyTurnSpeed);

  // Terrain Collision / Gravity Resolution
  glm::vec3 farinaPos = _pFarina->getPosition();
  float terrainHeight = _getTerrainHeight(farinaPos.x, farinaPos.z);

  // Simple collision: Keep on the ground
  glm::vec3 newFarinaPos = _checkAndResolveCollisions(
      glm::vec3(farinaPos.x, terrainHeight + 1.0f, farinaPos.z),
      _pFarina->getRadius());

  _pFarina->setPosition(newFarinaPos);

  for (auto enemy : _enemies) {
    if (enemy->isAlive() && !enemy->isFalling()) {
      enemy->update(deltaTime, _pCharacter->getPosition(), enemyTurnSpeed);

      // Check if enemy has fallen off the world
      glm::vec3 enemyPos = enemy->getPosition();

      // Apply collision detection with bushes
      const float ENEMY_RADIUS = enemy->getRadius();
      enemyPos = _checkAndResolveCollisions(enemyPos, ENEMY_RADIUS);

      float terrainHeight = _getTerrainHeight(enemyPos.x, enemyPos.z);

      if (terrainHeight < -500.0f) {
        // Enemy is off the edge, start falling and spawn particles
        enemy->setFalling(true);
        _particleSystem->spawnBurst(enemyPos, 15);
        fprintf(stdout, "[INFO]: Enemy fell off the edge!\n");
      } else if (!enemy->isFalling()) {
        // Keep enemy on terrain
        enemyPos.y = terrainHeight + 1.0f;
        enemy->setPosition(enemyPos);
      }
    } else if (enemy->isFalling()) {
      // Update falling enemy
      enemy->update(deltaTime, _pCharacter->getPosition(), enemyTurnSpeed);

      // if it has fallen too far spawn final rings and kill the thing
      if (enemy->getPosition().y < -50.0f && enemy->isAlive()) {
        enemy->setAlive(false);
        _particleSystem->spawnBurst(enemy->getPosition(), 10);
      }
    }
  }

  // Update coins
  for (auto coin : _coins) {
    coin->update(deltaTime);
  }

  // Update particle system
  _particleSystem->update(deltaTime);

  // Check collisions
  _checkEnemyCollisions();
  _checkPlayerEnemyCollision();
  _checkCoinCollection();

  // Update camera to follow character
  if (_cam == _arcBallCam) {
    _arcBallCam->setLookAtPoint(_pCharacter->getPosition() +
                                glm::vec3(0.0f, 5.0f, 0.0f));
    _arcBallCam->recomputeOrientation();
  } else if (_cam == _firstPersonCam) {
    const float EYE_HEIGHT = 5.0f; // TODO: find a better height i just placed it
    glm::vec3 characterPos = _pCharacter->getPosition();
    glm::vec3 cameraPos = characterPos + glm::vec3(0.0f, EYE_HEIGHT, 0.0f);

    // update camera position to match character
    _firstPersonCam->setPosition(cameraPos);

    // Camera should look in the direction the character is facing
    float characterHeading = _pCharacter->getHeading();
    _firstPersonCam->setTheta(-characterHeading);
    _firstPersonCam->setPhi(_cameraPitch);

    _firstPersonCam->recomputeOrientation();
      const glm::vec3 spotLightPosition = _firstPersonCam->getPosition();
      const glm::vec3 spotLightDirection =
          glm::normalize(_firstPersonCam->getLookAtPoint() - spotLightPosition);
      _lightingShaderProgram->useProgram();
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.spotLightPosition, spotLightPosition);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.spotLightDirection, spotLightDirection);

  _elsterShaderProgram->useProgram();
  _elsterShaderProgram->setProgramUniform(
      _elsterShaderUniformLocations.spotLightPosition, spotLightPosition);
  _elsterShaderProgram->setProgramUniform(
      _elsterShaderUniformLocations.spotLightDirection, spotLightDirection);

  // set lighting for ground tess shader
  _groundTessShaderProgram->useProgram();
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.spotLightPosition, spotLightPosition);
  _groundTessShaderProgram->setProgramUniform(
      _groundTessShaderUniformLocations.spotLightDirection, spotLightDirection);
  }
}

void FPEngine::run() {
  //  This is our draw loop - all rendering is done here.  We use a loop to keep
  //  the window open
  //	until the user decides to close the window and quit the program. Without
  // a loop, the 	window will display once and then the program exits.

  // Initialize delta time tracking
  static float lastTime = 0.0f;

  while (!glfwWindowShouldClose(
      mpWindow)) {         // check if the window was instructed to be closed
    glDrawBuffer(GL_BACK); // work with our back frame buffer
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT); // clear the current color contents and depth
                                  // buffer in the window

    // Calculate delta time
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // Get the size of our framebuffer.  Ideally this should be the same
    // dimensions as our window, but when using a Retina display the actual
    // window can be larger than the requested window.  Therefore, query what
    // the actual size of the window we are rendering to is.
    GLint framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(mpWindow, &framebufferWidth, &framebufferHeight);

    float mainAspectRatio = static_cast<float>(framebufferWidth) /
                            static_cast<float>(framebufferHeight);

    // Create projection matrix for main viewport
    glm::mat4 mainProjectionMatrix =
        glm::perspective(45.0f, mainAspectRatio, 0.1f, 1000.0f);

    // Render main camera view (full screen)
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    _renderScene(_cam->getViewMatrix(), mainProjectionMatrix,
                 _cam->getPosition());

    _updateScene();

    glfwSwapBuffers(
        mpWindow); // flush the OpenGL commands and make sure they get rendered!
    glfwPollEvents(); // check for any events and signal to redraw screen
  }
}

//*************************************************************************************
//
// Private Helper Functions

void FPEngine::_computeAndSendMatrixUniforms(const glm::mat4 &modelMtx,
                                             const glm::mat4 &viewMtx,
                                             const glm::mat4 &projMtx) const {
  // precompute the Model-View-Projection matrix on the CPU
  const glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
  // then send it to the shader on the GPU to apply to every vertex
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.mvpMatrix, mvpMtx);

  // TODO #7: compute and send the normal matrix
  const glm::mat3 normalMtx = glm::transpose(glm::inverse(glm::mat3(modelMtx)));
  // then send it to the shader on the GPU to apply to every vertex
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.normalMatrix, normalMtx);
  _lightingShaderProgram->setProgramUniform(
      _lightingShaderUniformLocations.modelMatrix, modelMtx);
}

float FPEngine::_getTerrainHeight(float x, float z) const {
  // Check if position is within terrain bounds
  if (x < -WORLD_SIZE || x > WORLD_SIZE || z < -WORLD_SIZE || z > WORLD_SIZE) {
    return -1000.0f; // Return very low height if out of bounds (for falling)
  }

  // Hill height parameter (matching shader)
  const float hillHeight = 56.25f;
  return hillHeight;
}

glm::vec3 FPEngine::_checkAndResolveCollisions(const glm::vec3 &position,
                                               float characterRadius) const {
  glm::vec3 correctedPos = position;
  const float characterHeight = 1.0f;

  // Check collision with bushes
  for (const auto &bush : _bushes) {
    glm::vec3 bushCenter = bush.position;
    float bushRadius = bush.size;

    // Check vertical overlap first
    float bushBottom = bushCenter.y - bushRadius;
    float bushTop = bushCenter.y + bushRadius;
    float charBottom = correctedPos.y;
    float charTop = correctedPos.y + characterHeight;

    // Only check horizontal collision if theres vertical overlap
    bool verticalOverlap = (charBottom < bushTop) && (charTop > bushBottom);

    if (verticalOverlap) {
      // Calculate 2D distance
      glm::vec2 charPosXZ(correctedPos.x, correctedPos.z);
      glm::vec2 bushPosXZ(bushCenter.x, bushCenter.z);
      float distance2D = glm::length(charPosXZ - bushPosXZ);
      float minDistance = characterRadius + bushRadius;

      // Check if collision occurs
      if (distance2D < minDistance && distance2D > 0.001f) {
        // Push character away from bush in XZ plane
        glm::vec2 pushDirection = glm::normalize(charPosXZ - bushPosXZ);
        glm::vec2 correction = pushDirection * (minDistance - distance2D);
        correctedPos.x += correction.x;
        correctedPos.z += correction.y;
      }
    }
  }

  return correctedPos;
}

float FPEngine::_getObjectHeightAt(float x, float z) const {
  const float CHARACTER_RADIUS =
      0.5f; // Match this with character collision radius

  // treat bushes as having a flat top for landing
  for (const auto &bush : _bushes) {
    glm::vec2 charPosXZ(x, z);
    glm::vec2 bushPosXZ(bush.position.x, bush.position.z);
    float distance = glm::length(charPosXZ - bushPosXZ);
    float bushRadius = bush.size;

    if (distance < bushRadius * 1.2f) {
      // Return the top of the bush
      return bush.position.y + bushRadius;
    }
  }

  return -1000.0f; // No object at this position
}

GLuint FPEngine::_loadAndRegisterTexture(const char *FILENAME) {
  // our handle to the GPU
  GLuint textureHandle = 0;

  // enable setting to prevent image from being upside down
  stbi_flip_vertically_on_write(true);

  // will hold image parameters after load
  GLint imageWidth, imageHeight, imageChannels;
  // load image from file
  GLubyte *data =
      stbi_load(FILENAME, &imageWidth, &imageHeight, &imageChannels, 0);

  // if data was read from file
  if (data) {
    const GLint STORAGE_TYPE = (imageChannels == 4 ? GL_RGBA : GL_RGB);

    // TODO #01 - generate a texture handle
    glGenTextures(1, &textureHandle);
    // TODO #02 - bind it to be active
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    // set texture parameters
    // TODO #03 - mag filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // TODO #04 - min filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // TODO #05 - wrap s
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // TODO #06 - wrap t
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // TODO #07 - transfer image data to the GPU
    glTexImage2D(GL_TEXTURE_2D, 0, STORAGE_TYPE, imageWidth, imageHeight, 0,
                 STORAGE_TYPE, GL_UNSIGNED_BYTE, data);

    fprintf(stdout, "[INFO]: %s texture map read in with handle %d\n", FILENAME,
            textureHandle);

    // release image memory from CPU - it now lives on the GPU
    stbi_image_free(data);
  } else {
    // load failed
    fprintf(stderr, "[ERROR]: Could not load texture map \"%s\"\n", FILENAME);
  }

  // return generated texture handle
  return textureHandle;
}

void FPEngine::_spawnEnemies(int numEnemies) {
  srand(time(0));

  for (int i = 0; i < numEnemies; ++i) {
    // random position around the world
    float x = (getRand() - 0.5f) * WORLD_SIZE * 1.5f;
    float z = (getRand() - 0.5f) * WORLD_SIZE * 1.5f;

    // dont spawn too close to center
    if (abs(x) < 15.0f && abs(z) < 15.0f) {
      x += (x > 0 ? 20.0f : -20.0f);
      z += (z > 0 ? 20.0f : -20.0f);
    }

    float y = _getTerrainHeight(x, z) + 1.0f;

    // rand heading
    float heading = getRand() * 2.0f * M_PI;

    Tympanius *enemy = new Tympanius(_tympaniusShaderProgram, &_tympaniusShaderUniformLocations, &_tympaniusShaderAttributeLocations);

    _enemies.push_back(enemy);
  }

  fprintf(stdout, "[INFO]: Spawned %d enemies\n", numEnemies);
}

void FPEngine::_spawnCoins() {
  // spawn 4 coins at the corners of the map
  const float offset = WORLD_SIZE * 0.8f;

  glm::vec3 corners[4] = {
      glm::vec3(-offset, 0.0f, -offset), // Bottom left
      glm::vec3(offset, 0.0f, -offset),  // Bottom right
      glm::vec3(-offset, 0.0f, offset),  // Top left
      glm::vec3(offset, 0.0f, offset)    // Top right
  };

  for (int i = 0; i < 4; ++i) {
    float terrainY = _getTerrainHeight(corners[i].x, corners[i].z);
    corners[i].y = terrainY + 2.0f;

    Coin *coin = new Coin(corners[i]);
    _coins.push_back(coin);
  }

  fprintf(stdout, "[INFO]: Spawned 4 coins at corners\n");
}

void FPEngine::_checkEnemyCollisions() {
  // collisions between all enemy pairs
  for (size_t i = 0; i < _enemies.size(); ++i) {
    if (!_enemies[i]->isAlive() || _enemies[i]->isFalling())
      continue;

    for (size_t j = i + 1; j < _enemies.size(); ++j) {
      if (!_enemies[j]->isAlive() || _enemies[j]->isFalling())
        continue;

      glm::vec3 pos1 = _enemies[i]->getPosition();
      glm::vec3 pos2 = _enemies[j]->getPosition();

      float distance = glm::length(glm::vec2(pos1.x - pos2.x, pos1.z - pos2.z));
      float minDistance = _enemies[i]->getRadius() + _enemies[j]->getRadius();

      if (distance < minDistance) {
        // make them bounce off each other
        _enemies[i]->bounceOff(pos2);
        _enemies[j]->bounceOff(pos1);

        // Push them apart slightly
        glm::vec3 pushDir = glm::normalize(pos1 - pos2);
        pushDir.y = 0.0f;
        float pushAmount = (minDistance - distance) * 0.5f;

        _enemies[i]->setPosition(pos1 + pushDir * pushAmount);
        _enemies[j]->setPosition(pos2 - pushDir * pushAmount);
      }
    }
  }
}

void FPEngine::_checkPlayerEnemyCollision() {
  if (_characterDead)
    return;

  glm::vec3 playerPos = _pCharacter->getPosition();
  const float playerRadius = 0.5f;
  const float maxVerticalDistance = 2.0f;

  // Check collision with Tympanius
  glm::vec3 tympaniusPos = _pTympanius->getPosition();
  float tympaniusDistance = glm::length(glm::vec2(playerPos.x - tympaniusPos.x, playerPos.z - tympaniusPos.z));
  float tympaniusMinDistance = playerRadius + _pTympanius->getRadius(); // Tympanius's radius
  float tympaniusVerticalDistance = abs(playerPos.y - tympaniusPos.y);

  if (tympaniusDistance < tympaniusMinDistance &&
      tympaniusVerticalDistance < maxVerticalDistance) {
    _characterDead = true;
    _particleSystem->spawnBurst(playerPos, 30);
    fprintf(stdout, "[INFO]: Player hit by Tympanius! Game Over!\n");
    fprintf(stdout, "[INFO]: Coins collected: %d / 4\n", _coinsCollected);
    return;
  }

  // Check collision with Wilfred
  glm::vec3 wilfredPos = _pWilfred->getPosition();
  float wilfredDistance = glm::length(
      glm::vec2(playerPos.x - wilfredPos.x, playerPos.z - wilfredPos.z));
  float wilfredMinDistance = playerRadius + _pWilfred->getRadius();
  float wilfredVerticalDistance = abs(playerPos.y - wilfredPos.y);

  if (wilfredDistance < wilfredMinDistance &&
      wilfredVerticalDistance < maxVerticalDistance) {
    _characterDead = true;
    _particleSystem->spawnBurst(playerPos, 30);
    fprintf(stdout, "[INFO]: Player hit by Wilfred! Game Over!\n");
    fprintf(stdout, "[INFO]: Coins collected: %d / 4\n", _coinsCollected);
    return;
  }

  // Check collision with enemy Elster
  glm::vec3 elsterPos = _pEnemyElster->getPosition();
  float elsterDistance = glm::length(
      glm::vec2(playerPos.x - elsterPos.x, playerPos.z - elsterPos.z));
  float elsterMinDistance = playerRadius + 0.5f; // Elster's radius
  float elsterVerticalDistance = abs(playerPos.y - elsterPos.y);

  if (elsterDistance < elsterMinDistance &&
      elsterVerticalDistance < maxVerticalDistance) {
    _characterDead = true;
    _particleSystem->spawnBurst(playerPos, 30);
    fprintf(stdout, "[INFO]: Player hit by enemy Elster! Game Over!\n");
    fprintf(stdout, "[INFO]: Coins collected: %d / 4\n", _coinsCollected);
    return;
  }

  glm::vec3 farinaPos = _pFarina->getPosition();
  float farinaDist = glm::length(
      glm::vec2(playerPos.x - farinaPos.x, playerPos.z - farinaPos.z));
  float farinaMinDist = playerRadius + _pFarina->getRadius();
  float farinaVertDist = abs(playerPos.y - farinaPos.y);

  if (farinaDist < farinaMinDist && farinaVertDist < 2.0f) {
    _characterDead = true;
    _particleSystem->spawnBurst(playerPos, 30);
    return;
  }

  for (auto enemy : _enemies) {
    if (!enemy->isAlive() || enemy->isFalling())
      continue;

    glm::vec3 enemyPos = enemy->getPosition();

    // Check horizontal distance
    float distance = glm::length(
        glm::vec2(playerPos.x - enemyPos.x, playerPos.z - enemyPos.z));
    float minDistance = playerRadius + enemy->getRadius();

    // player must be at roughly same height as enemy
    float verticalDistance = abs(playerPos.y - enemyPos.y);

    if (distance < minDistance && verticalDistance < maxVerticalDistance) {
      // player die </3 rip
      _characterDead = true;
      _particleSystem->spawnBurst(playerPos, 30);
      fprintf(stdout, "[INFO]: Player hit by enemy! Game Over!\n");
      fprintf(stdout, "[INFO]: Coins collected: %d / 4\n", _coinsCollected);
      return;
    }
  }
}

void FPEngine::_checkCoinCollection() {
  if (_characterDead)
    return;

  glm::vec3 playerPos = _pCharacter->getPosition();

  for (auto coin : _coins) {
    if (coin->isCollected())
      continue;

    // distance to coin
    float distance = glm::length(playerPos - coin->getPosition());

    // collection radius
    const float collectionRadius = 2.5f; // Larger collection radius

    if (distance < collectionRadius) {
      // Coin collected! hooray! yippee!
      coin->setCollected(true);
      _coinsCollected++;
      _particleSystem->spawnBurst(coin->getPosition(), 15);
      fprintf(stdout, "[INFO]: Coin collected! (%d / 4)\n", _coinsCollected);

      if (_coinsCollected >= 4) {
        fprintf(stdout,
                "[INFO]: All coins collected! You win!\n"); // you get nothing
                                                            // for winning lol
      }
    }
  }
}

//*************************************************************************************
//
// Callbacks

void mp_engine_keyboard_callback(GLFWwindow *window, const int key,
                                 const int scancode, const int action,
                                 const int mods) {
  const auto engine = static_cast<FPEngine *>(glfwGetWindowUserPointer(window));

  // pass the key and action through to the engine
  engine->handleKeyEvent(key, action);
}

void mp_engine_cursor_callback(GLFWwindow *window, const double x,
                               const double y) {
  const auto engine = static_cast<FPEngine *>(glfwGetWindowUserPointer(window));

  // pass the cursor position through to the engine
  engine->handleCursorPositionEvent(glm::vec2(x, y));
}

void mp_engine_mouse_button_callback(GLFWwindow *window, const int button,
                                     const int action, const int mods) {
  const auto engine = static_cast<FPEngine *>(glfwGetWindowUserPointer(window));

  // pass the mouse button and action through to the engine
  engine->handleMouseButtonEvent(button, action);
}
