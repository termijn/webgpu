#pragma once

#include "object.h"
#include "scene.h"
#include "image.h"

#include <memory>
#include <string>
#include <map>

// Loads an entire gltf model in a single mesh
Mesh loadModel(const std::string& filePath);

// Loads the gltf model in a scene graph under parent, respecting the individual components in the model
std::unique_ptr<Scene> loadModelObjects(const std::string &filePath, Object &parent);

// Loads an image from the given file path. Can be one of: jpg, png, tga, bmp, psd, gif, hdr radiance rgbE
Image loadImage(const std::string& filePath);