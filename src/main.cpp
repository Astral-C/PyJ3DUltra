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

static bool init = false;

bool InitJ3DUltra(){
    if(!init){
        if(gladLoadGL()){
            J3DUniformBufferObject::CreateUBO();
            init = true;
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

PYBIND11_MODULE(J3DUltra, m) {
    m.doc() = "J3DUltra";

    py::class_<J3DModelData, std::shared_ptr<J3DModelData>>(m, "J3DModelData")
        .def(py::init<>())
        .def("getInstance", &J3DModelData::GetInstance);

    py::class_<J3DModelInstance, std::shared_ptr<J3DModelInstance>>(m, "J3DModelInstance")
        .def(py::init<std::shared_ptr<J3DModelData>>())
        .def("render", &J3DModelInstance::Render)
        // These don't work yet
        .def("setTranslation", &J3DModelInstance::SetTranslation)
        .def("setRotation", &J3DModelInstance::SetRotation)
        .def("setScale", &J3DModelInstance::SetScale);

    m.def("loadModel", &LoadJ3DModel, "Load a BMD/BDL Model");

    m.def("init", &InitJ3DUltra, "Setup J3DUltra for Model Loading and Rendering");
    m.def("cleanup", &CleanupJ3DUltra, "Cleanup J3DUltra Library");
    m.def("setCamera", &SetCamera, "Set Projection and View Matrices to render with");
}