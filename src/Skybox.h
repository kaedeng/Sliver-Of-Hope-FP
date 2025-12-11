#ifndef SKYBOX_H
#define SKYBOX_H

#include <CSCI441/ShaderProgram.hpp>

#include <string>
#include <vector>

class Skybox {
public:
  Skybox();
  void draw(const glm::mat4 &view, const glm::mat4 &projection);

private:
  CSCI441::ShaderProgram *mShaderProgram;
  GLuint mTextureId;
  GLuint mVAO;
  GLuint mVBO;

  void loadCubemap(const std::vector<std::string> &faces);
};

#endif // SKYBOX_H
