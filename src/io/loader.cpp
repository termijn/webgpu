#include "loader.h"

#include "renderable.h"

#include <iostream>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <tinygltf-2.9.3/stb_image.h>
#include <tinygltf-2.9.3/stb_image_write.h>
#include <tinygltf-2.9.3/json.hpp>

#define TINYGLTF_HEADER_ONLY
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_USE_CPP14
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#include <tinygltf-2.9.3/tiny_gltf.h>

using namespace glm;

std::map<int, Image>    imageCache;
std::map<int, Mesh>     meshCache;

Image getImage(int i, std::function<Image(void)> creator)
{
    if (imageCache.find(i) == imageCache.end())
        imageCache.emplace(std::make_pair(i, creator()));

    return imageCache[i];
}

glm::mat4 make_mat4(const double* matrixArray) {
    return glm::mat4(
        (float)matrixArray[0], (float)matrixArray[1], (float)matrixArray[2], (float)matrixArray[3],
        (float)matrixArray[4], (float)matrixArray[5], (float)matrixArray[6], (float)matrixArray[7],
        (float)matrixArray[8], (float)matrixArray[9], (float)matrixArray[10], (float)matrixArray[11],
        (float)matrixArray[12], (float)matrixArray[13], (float)matrixArray[14], (float)matrixArray[15]
    );
}

glm::vec3 make_vec3(const double* vecArray) 
{
    return glm::vec3(vecArray[0], vecArray[1], vecArray[2]);
}

glm::quat make_quat(const double* quatArray) {
    // GLM expects quaternions in the (w, x, y, z) format
    return glm::quat(static_cast<float>(quatArray[3]), static_cast<float>(quatArray[0]), 
                     static_cast<float>(quatArray[1]), static_cast<float>(quatArray[2]));
}

glm::mat4 getNodeTransformation(const tinygltf::Node& node) 
{
    glm::mat4 transform = glm::mat4(1.0);

    // If the node has a matrix, use it
    if (node.matrix.size() == 16) 
    {
        transform = make_mat4(node.matrix.data());
    } 
    else 
    {
        // Otherwise, apply translation, rotation, and scale (TRS)
        if (node.translation.size() == 3) {
            glm::vec3 translation = make_vec3(node.translation.data());
            transform = glm::translate(transform, translation);
        }

        if (node.rotation.size() == 4) {
            glm::quat rotation = make_quat(node.rotation.data());
            transform *= glm::toMat4(rotation);
        }

        if (node.scale.size() == 3) {
            glm::vec3 scale = make_vec3(node.scale.data());
            transform = glm::scale(transform, scale);
            std::cout <<  node.name << " has scale: " << scale.x << "," << scale.y << "," << scale.z << std::endl;
        }
    }
    return transform;
}

struct AttributeData
{
    const unsigned char* dataPtr        = nullptr;
    size_t               itemCount      = 0;
    int                  stride         = 0;
    int                  componentType  = 0;

    template <typename T>
    T get(int index) const
    {
        int strideBytes = stride == -1 || stride == 0 ? sizeof(T) : stride;
        const T* value = reinterpret_cast<const T*>(dataPtr + index * strideBytes);
        return *value;
    }
};

std::optional<AttributeData> read(const tinygltf::Model& model, tinygltf::Primitive& primitive, int accessorIndex)
{
	if (accessorIndex >= model.accessors.size()) return std::nullopt;

	const tinygltf::Accessor*   accessor   = &model.accessors[accessorIndex];
	const tinygltf::BufferView* bufferView = &model.bufferViews[accessor->bufferView];
	const tinygltf::Buffer*     buffer     = &model.buffers[bufferView->buffer];

	auto result = AttributeData {
		&buffer->data[bufferView->byteOffset + accessor->byteOffset],
		accessor->count,
		accessor->ByteStride(*bufferView),
        accessor->componentType
	};
	return result;
}

std::optional<AttributeData> read(const tinygltf::Model& model, tinygltf::Primitive& primitive, const std::string& attribute)
{
    std::optional<AttributeData> result = std::nullopt;

    if (primitive.attributes.find(attribute) != primitive.attributes.end()) 
    {
        int accessorIndex = primitive.attributes.at(attribute);
		result = read(model, primitive, accessorIndex);
    }
    return result;
}

void fillVertices(const tinygltf::Model& model, tinygltf::Primitive& primitive, Mesh& mesh, glm::mat4 nodeTransform)
{
    vec4 max = vec4(std::numeric_limits<float>::min());
    vec4 min = vec4(std::numeric_limits<float>::max());

    auto normalData        = read(model, primitive, "NORMAL");
    auto texCoordsData     = read(model, primitive, "TEXCOORD_0");
    auto positionData      = read(model, primitive, "POSITION");

    mesh.vertices().reserve(positionData->itemCount);
    for (size_t i = 0; i < positionData->itemCount; ++i) 
    {
        glm::vec3 vertexPosition        = positionData->get<vec3>(i);
        glm::vec4 transformedPosition   = nodeTransform * glm::vec4(vertexPosition, 1.0f);

        glm::vec3 vertexNormal(0.0f);
        if (normalData) 
        {
            vertexNormal = normalData->get<vec3>(i);
            glm::vec4 transformedNormal = transpose(inverse(nodeTransform)) * glm::vec4(vertexNormal, 0.0f);
            vertexNormal = glm::normalize(glm::vec3(transformedNormal));
        }

        glm::vec2 vertexTexCoord(0.0f);
        if (texCoordsData)
            vertexTexCoord = texCoordsData->get<vec2>(i);

        mesh.vertices().push_back(Vertex {.position = transformedPosition, .normal = vec4(vertexNormal, 0.0f), .uv = vertexTexCoord  });

        min = glm::min(min, transformedPosition);
        max = glm::max(max, transformedPosition);
    }
    mesh.boundingBox = Box { min, max };

    std::cout << "min=" << min.x << "," << min.y << "," << min.z << " max=" << max.x << "," << max.y << "," << max.z << std::endl;
}

void fillIndices(const tinygltf::Model& model, tinygltf::Primitive& primitive, Mesh& mesh, uint vertexOffset)
{
	if (primitive.indices < 0)
	{
		std::cout << "Primitive has no indices; might be a non-indexed model." << std::endl;
		return;
	}
    
	auto indexData = read(model, primitive, primitive.indices);
	if (indexData == std::nullopt) return;

	mesh.indices().reserve(indexData->itemCount / 3);

	for (size_t i = 0; i < indexData->itemCount; i += 3) 
	{
		u32vec3 triangle;

		if (indexData->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) 
		{
			triangle.x = indexData->get<uint16>(i);
			triangle.y = indexData->get<uint16>(i + 1);
			triangle.z = indexData->get<uint16>(i + 2);;

		} 
		else if (indexData->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) 
		{
			triangle.x = indexData->get<uint32>(i);
			triangle.y = indexData->get<uint32>(i + 1);
			triangle.z = indexData->get<uint32>(i + 2);
		}

		mesh.indices().emplace_back(triangle + u32vec3(vertexOffset));
	}
}

std::vector<vec2> getUVSet(int uvIndex, const tinygltf::Model& model)
{
    std::vector<vec2> result;

    const tinygltf::Accessor* texCoordAccessor        = nullptr;
    const tinygltf::BufferView* texCoordBufferView    = nullptr;
    const tinygltf::Buffer* texCoordBuffer            = nullptr;
    const unsigned char* texCoordData                 = nullptr;
    size_t texCoordStride = 0;

    int texCoordAccessorIndex   = uvIndex;
    texCoordAccessor            = &model.accessors[texCoordAccessorIndex];
    texCoordBufferView          = &model.bufferViews[texCoordAccessor->bufferView];
    texCoordBuffer              = &model.buffers[texCoordBufferView->buffer];
    texCoordData                = &texCoordBuffer->data[texCoordBufferView->byteOffset + texCoordAccessor->byteOffset];
    texCoordStride              = texCoordAccessor->ByteStride(*texCoordBufferView);
    if (texCoordStride == 0) texCoordStride = sizeof(vec2);

    int nrCoords = texCoordBufferView->byteLength / texCoordStride;
    std::cout << "nrUVCoords = " << nrCoords << std::endl;
    for (size_t i = 0; i < nrCoords; ++i) 
    {
        glm::vec2 vertexTexCoord(0.0f);
        if (texCoordAccessor)
        {
            const float* texCoord = reinterpret_cast<const float*>(texCoordData + i * texCoordStride);
            result.emplace_back(texCoord[0], texCoord[1]);
        }
    }
    return result;
}

TextureTransforms getTextureTransforms(const tinygltf::TextureInfo& textureInfo)
{
    TextureTransforms result;
    if (textureInfo.extensions.contains("KHR_texture_transform"))
    {
        auto extension = textureInfo.extensions.find("KHR_texture_transform");

        std::cout << "Extension found: " << extension->first << std::endl;

        const auto& transform = extension->second;

        if (transform.Has("offset")) 
        {
            std::cout << "it has offset" << std::endl;
            const auto& offsetArray = transform.Get("offset");
            if (offsetArray.IsArray() && offsetArray.ArrayLen() == 2) 
            {
                glm::vec2 offset(
                    offsetArray.Get(0).Get<double>(),
                    offsetArray.Get(1).Get<double>()
                );
                std::cout << "Texture offset: x = " << offset.x << ", y = " << offset.y << std::endl;
                result.offset = offset;
            }
        }
        
        if (transform.Has("scale")) 
        {
            std::cout << "it has scale" << std::endl;
            const auto& scaleArray = transform.Get("scale");
            if (scaleArray.IsArray() && scaleArray.ArrayLen() == 2) 
            {
                glm::vec2 scale(
                    scaleArray.Get(0).Get<double>(),
                    scaleArray.Get(1).Get<double>()
                );
                std::cout << "Texture scale: x = " << scale.x << ", y = " << scale.y << std::endl;
                result.scale = scale;
            }
        }

        if (transform.Has("texCoord"))
        {
             const auto& texCoord = transform.Get("texCoord");
             int uvIndex = texCoord.GetNumberAsInt();

             std::cout << "it overrides texCoord with " << uvIndex << std::endl;
             result.uvSet = uvIndex;
        }
    }
    return result;
}

Image convertTo8bits(Image& image)
{
    std::vector<uint8_t> newPixels(image.bytes() / 2);

    int nrBytes = newPixels.size();
    for (int i = 0; i < nrBytes; i++)
        newPixels[i] = image.getPixels()[i * 2];

    Image result(newPixels);
    return result;
}

template <typename T>
Image getImage(const T& textureInfo, const tinygltf::Model& model)
{
    Image img;
    if (textureInfo.index >= 0)
    {
        img = getImage(textureInfo.index, [&]
        {
            
            const tinygltf::Texture& texture    = model.textures[textureInfo.index];
            const tinygltf::Image& image        = model.images[texture.source];

            Image newImage(image.image);
            newImage.width           = image.width;
            newImage.height          = image.height;
            newImage.bytesPerPixel   = image.component * (image.bits / 8);

            std::cout << "component: " << image.component << std::endl;
            std::cout << "bits: " << image.bits << std::endl;

            if (image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE && image.component == 4)
            {
                newImage.bytesPerPixel   = 4;
                newImage.type            = Image::Type::RGBA;
                std::cout << "TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE" << std::endl;
            }

            if (image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT && image.component == 4)
            {
                newImage = convertTo8bits(newImage);
                newImage.bytesPerPixel   = 4;
                newImage.type            = Image::Type::RGBA;
                std::cout << "TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT" << std::endl;
            }

            if (image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE && image.component == 2)
            {
                newImage.type = Image::Type::RG8;
                std::cout << "TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE" << std::endl;
            }

            if (image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE && image.component == 1)
            {
                newImage.type = Image::Type::R8;
                std::cout << "TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE" << std::endl;
            }

            if (image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE && image.component == 3)
            {
                newImage.type = Image::Type::RGB;
                std::cout << "TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE" << std::endl;
            }

            if (image.pixel_type == TINYGLTF_COMPONENT_TYPE_FLOAT)
                std::cout << "TINYGLTF_COMPONENT_TYPE_FLOAT" << std::endl;
            if (image.pixel_type == TINYGLTF_COMPONENT_TYPE_DOUBLE)
                std::cout << "TINYGLTF_COMPONENT_TYPE_DOUBLE" << std::endl;

            std::cout << "image width "     << newImage.width << std::endl;
            std::cout << "image height "    << newImage.height << std::endl;
            std::cout << "image bpp "       << newImage.bytesPerPixel << std::endl;
            return newImage;
        });
    }

    return img;
}

void fillMaterial(const tinygltf::Model& model, int materialIndex, Material& material)
{
    if (materialIndex < 0 || materialIndex >= model.materials.size()) {
        std::cerr << "Invalid material index." << std::endl;
        return;
    }

    const tinygltf::Material& gltfMaterial = model.materials[materialIndex];
    std::cout << "Material: " << gltfMaterial.name << std::endl;

    auto baseColorFactor = gltfMaterial.pbrMetallicRoughness.baseColorFactor;
    if (!baseColorFactor.empty()) 
        material.albedo = glm::vec4(baseColorFactor[0], baseColorFactor[1], baseColorFactor[2], baseColorFactor[3]);

    auto& pbr = gltfMaterial.pbrMetallicRoughness;

    material.metallic = pbr.metallicFactor;
    material.roughness = pbr.roughnessFactor;

    // Load Base Color Texture
    if (pbr.baseColorTexture.index >= 0)
    {
        material.baseColorTransforms = getTextureTransforms(pbr.baseColorTexture);
        material.baseColorTexture = getImage(pbr.baseColorTexture, model);

        if (material.baseColorTransforms.uvSet >= 0)
        {
            std::cout << "material.baseColorTransforms.uvSet >= 0" << std::endl;
            material.uvSet = getUVSet(material.baseColorTransforms.uvSet, model);
        }
    }

    if (pbr.metallicRoughnessTexture.index >= 0)
    {     
        material.metallicRoughnessTransforms = getTextureTransforms(pbr.metallicRoughnessTexture);
        material.metallicRoughness = getImage(pbr.metallicRoughnessTexture, model);
    }

    if (gltfMaterial.normalTexture.index >= 0)
    {
        std::cout << "Loading normal map" << std::endl;
        material.normalMap = getImage(gltfMaterial.normalTexture, model);
        material.normalMapScale = gltfMaterial.normalTexture.scale;
    }

    if (gltfMaterial.occlusionTexture.index >= 0)
    {
        std::cout << "Loading occlusion map" << std::endl;
        material.occlusion = getImage(gltfMaterial.occlusionTexture, model);
    }

    if (gltfMaterial.emissiveTexture.index >= 0)
    {
        std::cout << "Loading emmisive texture" << std::endl;
        material.emissive = getImage(gltfMaterial.emissiveTexture, model);
    }
}

void traverseNodes(tinygltf::Model& model, const tinygltf::Node& node, Mesh& mesh, mat4 nodeTransform)
{
    std::cout << "Node: " << node.name << std::endl;
    
    nodeTransform = nodeTransform * getNodeTransformation(node);

    if (node.mesh >= 0)
    {
        tinygltf::Mesh& nodeMesh = model.meshes[node.mesh];

        for (auto& primitive: nodeMesh.primitives)
        {
            int offset = mesh.vertices().size();
            fillVertices(model, primitive, mesh, nodeTransform);
            fillIndices(model, primitive, mesh, offset);

            mesh.generateTangentVectors();
        }
    }
    
    for (const auto& childIndex : node.children)
        traverseNodes(model, model.nodes[childIndex], mesh, nodeTransform);
}

void traverseNodes(tinygltf::Model& model, const tinygltf::Node& node, Scene& scene, Object& parent, mat4 nodeTransform)
{
    std::cout << "Node: " << node.name << std::endl;

    mat4 transform                          = getNodeTransformation(node);
    RenderableObject& item = scene.newObject(parent);
    
    item.getObject().setTransform(getNodeTransformation(node));

    if (node.mesh >= 0)
    {
        tinygltf::Mesh& nodeMesh    = model.meshes[node.mesh];

        if (meshCache.contains(node.mesh))
        {            
            item.getRenderable().mesh = meshCache[node.mesh];
        } 
        else 
        {
            for (auto& primitive: nodeMesh.primitives)
            {
                Mesh& mesh = item.getRenderable().mesh;
                fillVertices (model, primitive, mesh, mat4(1.0));
                fillIndices  (model, primitive, mesh, 0);
                mesh.generateTangentVectors();
                meshCache[node.mesh] = mesh;
            }
        }

        Mesh& mesh = item.getRenderable().mesh;
        for (auto& primitive: nodeMesh.primitives)
        {
            if (primitive.material >= 0)
            {
                Material& material = item.getRenderable().material;
                fillMaterial(model, primitive.material, material);
                if (material.uvSet.has_value())
                {
                    std::cout << "overriding uvset " << std::endl;
                    int i = 0;
                    for (Vertex& vertex: mesh.vertices())
                        vertex.uv = material.uvSet.value()[i++];
                }
            }
        }
    }
    Object& object = item.getObject();
    for (const auto& childIndex : node.children)
        traverseNodes(model, model.nodes[childIndex], scene, object, nodeTransform);
}

std::optional<tinygltf::Model> loadGlTFModel(const std::string& filePath)
{
    std::cout << "Loading glTF model from " << filePath << std::endl;

    tinygltf::TinyGLTF  loader;
    tinygltf::Model     model;
    std::string         errors;
    std::string         warnings;
    
    bool loaded = loader.LoadBinaryFromFile(&model, &errors, &warnings, filePath);
    
    std::cout << "errors:"      << errors   << std::endl;
    std::cout << "warnings:"    << warnings << std::endl;

    return loaded ? std::optional<tinygltf::Model>(model) : std::nullopt;
}

Mesh loadModel(const std::string& filePath)
{
    imageCache.clear();
    std::optional<tinygltf::Model>  loadResult = loadGlTFModel(filePath);

    if (!loadResult.has_value()) return Mesh();

    tinygltf::Model& model = loadResult.value();

    Mesh result;
    mat4 nodeTransform = mat4(1.0);

    for (tinygltf::Scene& scene: model.scenes)
    {
        for (int nodeIndex : scene.nodes)
        {
            tinygltf::Node& node = model.nodes[nodeIndex];
            std::cout << "traversing node " << node.name << std::endl;
            traverseNodes(model, node, result, nodeTransform);
        }
    }

    return result;
}

std::unique_ptr<Scene> loadModelObjects(const std::string &filePath, Object &parent)
{
    imageCache.clear();
    auto result = std::make_unique<Scene>();

    std::optional<tinygltf::Model>  loadResult = loadGlTFModel(filePath);
    if (!loadResult.has_value()) return result;

    tinygltf::Model& model = loadResult.value();
    mat4 transform = mat4(1.0);

    for (tinygltf::Scene& scene: model.scenes)
    {
        for (int nodeIndex : scene.nodes)
        {
            tinygltf::Node& node = model.nodes[nodeIndex];
            std::cout << "traversing node " << node.name << std::endl;
            traverseNodes(model, node, *result, parent, transform);
        }
    }

    return result;
}

Image loadImage(const std::string &filePath)
{
    std::cout << "loadImage() - " << filePath << std::endl;
    int x, y, n = 0;
    unsigned char *data = stbi_load(filePath.c_str(), &x, &y, &n, STBI_rgb_alpha);

    Image image;
    image.bytesPerPixel = n;
    image.width         = x;
    image.height        = y;

    image.pixels->assign(data, data + x * y * n);

    image.type = n == 4 ? Image::Type::RGBA : Image::Type::RGB;
    stbi_image_free(data);
    return image;
}
