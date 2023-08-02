#include <filesystem>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <J3D/J3DModelLoader.hpp>
#include <J3D/J3DModelData.hpp>
#include <J3D/J3DUniformBufferObject.hpp>
#include <J3D/J3DLight.hpp>
#include <J3D/J3DModelInstance.hpp>
#include <bstream.h>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace py = pybind11;
using namespace py::literals;

static bool init = false;
static J3DLight lights[8] = {};

bool InitJ3DUltra(){
    if(!init){
        if(gladLoadGL()){
            init = true;
 
            J3DUniformBufferObject::CreateUBO();
 
            for (int i = 0; i < 8; i++) lights[i].Color = glm::vec4(1, 1, 1, 1);
            J3DUniformBufferObject::SetLights(lights);

            return true;
        }
    }

    return false;
}

void SetCamera(std::vector<float> proj, std::vector<float> view){
    glm::mat4 projection, viewm4;

    projection = glm::make_mat4(proj.data());
    viewm4 = glm::make_mat4(view.data());

    J3DUniformBufferObject::SetProjAndViewMatrices(&projection, &viewm4);
}

void SetLight(J3DLight light, int lightIdx){
    lights[lightIdx] = light;
    J3DUniformBufferObject::SetLights(lights);
}

void CleanupJ3DUltra(){
    if(init){
        J3DUniformBufferObject::DestroyUBO();
    }
}

std::shared_ptr<J3DModelInstance> LoadJ3DModel(std::string filepath){
    if(!init) return nullptr;

    if(!std::filesystem::exists(filepath)){
        std::cout << "Couldn't load model " << filepath << std::endl;
        return nullptr;
    }

    J3DModelLoader Loader;
    bStream::CFileStream modelStream(filepath, bStream::Endianess::Big, bStream::OpenMode::In);
    
    std::shared_ptr<J3DModelData> data = std::make_shared<J3DModelData>();
    data = Loader.Load(&modelStream, NULL);

    return data->GetInstance();
}

void setTranslation(std::shared_ptr<J3DModelInstance> instance, float x, float y, float z){
    instance->SetTranslation(glm::vec3(x, y, z));
}

void setRotation(std::shared_ptr<J3DModelInstance> instance, float x, float y, float z){
    instance->SetRotation(glm::vec3(x, y, z));
}

void setScale(std::shared_ptr<J3DModelInstance> instance, float x, float y, float z){
    instance->SetScale(glm::vec3(x, y, z));
}

PYBIND11_MODULE(J3DUltra, m) {
    m.doc() = "J3DUltra";

    py::class_<J3DLight>(m, "J3DLight")
        .def(py::init<>())
        .def(py::init([](std::array<float, 3> position, std::array<float, 3> direction, std::array<float, 4> color, std::array<float, 3> angle_atten, std::array<float, 3> dist_atten){
            J3DLight light;

            light.Position = glm::vec4(position[0], position[1], position[2], 1);
            light.Direction = glm::vec4(direction[0], direction[1], direction[2], 1);
            light.Color = glm::vec4(color[0], color[1], color[2], color[3]);
            light.AngleAtten = glm::vec4(angle_atten[0], angle_atten[1], angle_atten[2], 1);;
            light.DistAtten = glm::vec4(dist_atten[0], dist_atten[1], dist_atten[2], 1);;

            return light;
        }));

    py::class_<J3DModelData, std::shared_ptr<J3DModelData>>(m, "J3DModelData")
        .def(py::init<>())
        .def("getInstance", &J3DModelData::GetInstance);

    py::class_<J3DModelInstance, std::shared_ptr<J3DModelInstance>>(m, "J3DModelInstance")
        .def(py::init<std::shared_ptr<J3DModelData>>())
        .def("render", &J3DModelInstance::Render)
        // These don't work yet
        .def("setTranslation", &setTranslation)
        .def("setRotation", &setRotation)
        .def("setScale", &setScale);

    m.def("loadModel", &LoadJ3DModel, "Load a BMD/BDL Model");

    m.def("init", &InitJ3DUltra, "Setup J3DUltra for Model Loading and Rendering");
    m.def("cleanup", &CleanupJ3DUltra, "Cleanup J3DUltra Library");
    m.def("setCamera", &SetCamera, "Set Projection and View Matrices to render with");
    m.def("setLight", &SetLight, "Set Scene Light for J3D Render Functions");
}