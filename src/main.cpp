#include <filesystem>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <J3D/J3DModelLoader.hpp>
#include <J3D/Data/J3DModelData.hpp>
#include <J3D/Material/J3DUniformBufferObject.hpp>
#include <J3D/Animation/J3DColorAnimationInstance.hpp>
#include <J3D/Animation/J3DTexIndexAnimationInstance.hpp>
#include <J3D/Animation/J3DTexMatrixAnimationInstance.hpp>
#include <J3D/Animation/J3DJointAnimationInstance.hpp>
#include <J3D/Animation/J3DJointFullAnimationInstance.hpp>
#include <J3D/Animation/J3DVisibilityAnimationInstance.hpp>
#include <J3D/Animation/J3DAnimationLoader.hpp>
#include <J3D/Rendering/J3DRendering.hpp>
#include <J3D/Picking/J3DPicking.hpp>
#include <J3D/Rendering/J3DLight.hpp>
#include <J3D/Data/J3DModelInstance.hpp>
#include <bstream.h>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace py = pybind11;
using namespace py::literals;

static bool init = false;
static std::vector<std::shared_ptr<J3DModelInstance>> renderBatch = {};
static glm::mat4 viewMtx = {}, projMtx = {};

bool InitJ3DUltra(){
    if(!init){
        if(gladLoadGL()){
            init = true;
 
            J3DUniformBufferObject::CreateUBO();
 
            //set default sort
            J3D::Rendering::SetSortFunction([](J3D::Rendering::RenderPacketVector args){
                std::sort(args.begin(), args.end(), [](const J3DRenderPacket& a, const J3DRenderPacket& b) -> bool {
                    return a.SortKey > b.SortKey;
                });
            });

            return true;
        }
    }

    return false;
}

void SetCamera(std::vector<float> proj, std::vector<float> view){
    if(init){
        glm::mat4 projection, viewm4;

        projection = glm::make_mat4(proj.data());
        viewm4 = glm::make_mat4(view.data());

        viewMtx = viewm4;
        projMtx = projection;

        J3DUniformBufferObject::SetProjAndViewMatrices(projection, viewm4);
    }
}

void CleanupJ3DUltra(){
    if(init){
        J3DUniformBufferObject::DestroyUBO();
        renderBatch.clear();
        if(J3D::Picking::IsPickingEnabled()) J3D::Picking::DestroyFramebuffer();
    }
}

std::shared_ptr<J3DModelInstance> LoadJ3DModel(std::string path){
    if(!init) return nullptr;

    if(!std::filesystem::exists(path)){
        std::cout << "Couldn't load model " << path << std::endl;
        return nullptr;
    }

    J3DModelLoader Loader;
    bStream::CFileStream modelStream(path, bStream::Endianess::Big, bStream::OpenMode::In);
    
    std::shared_ptr<J3DModelData> data = std::make_shared<J3DModelData>();
    data = Loader.Load(&modelStream, NULL);

    return data->CreateInstance();
}

std::shared_ptr<J3DModelInstance> LoadJ3DModel(py::bytes data){
    if(!init) return nullptr;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DModelLoader Loader;
    bStream::CMemoryStream modelStream((uint8_t*)dataInfo.ptr, dataInfo.size, bStream::Endianess::Big, bStream::OpenMode::In);
    
    std::shared_ptr<J3DModelData> modelData = Loader.Load(&modelStream, NULL);

    return modelData->CreateInstance();
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

void renderModel(std::shared_ptr<J3DModelInstance> instance){
    renderBatch.push_back(instance);
}

void attachBrk(std::shared_ptr<J3DModelInstance> instance, py::bytes data){
    if(!init) return;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DColorAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DColorAnimationInstance>(animLoaded);

    instance->SetRegisterColorAnimation(animInstance);
}

void attachBrk(std::shared_ptr<J3DModelInstance> instance, std::string path){
    if(!init) return;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DColorAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DColorAnimationInstance>(path);

    instance->SetRegisterColorAnimation(animInstance);
}

void attachBrk(std::shared_ptr<J3DModelInstance> instance, std::shared_ptr<J3DAnimation::J3DColorAnimationInstance> anim){
    if(!init) return;
    instance->SetRegisterColorAnimation(anim);
}

std::shared_ptr<J3DAnimation::J3DColorAnimationInstance> LoadBrk(py::bytes data){
    if(!init) return nullptr;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DColorAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DColorAnimationInstance>(animLoaded);

    return animInstance;
}

std::shared_ptr<J3DAnimation::J3DColorAnimationInstance> LoadBrk(std::string path){
    if(!init) return nullptr;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DColorAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DColorAnimationInstance>(path);

    return animInstance;
}

void attachBtp(std::shared_ptr<J3DModelInstance> instance, py::bytes data){
    if(!init) return;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DTexIndexAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DTexIndexAnimationInstance>(animLoaded);

    instance->SetTexIndexAnimation(animInstance);
}

void attachBtp(std::shared_ptr<J3DModelInstance> instance, std::string path){
    if(!init) return;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DTexIndexAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DTexIndexAnimationInstance>(path);

    instance->SetTexIndexAnimation(animInstance);
}

void attachBtp(std::shared_ptr<J3DModelInstance> instance, std::shared_ptr<J3DAnimation::J3DTexIndexAnimationInstance> anim){
    if(!init) return;
    instance->SetTexIndexAnimation(anim);
}

std::shared_ptr<J3DAnimation::J3DTexIndexAnimationInstance> LoadBtp(py::bytes data){
    if(!init) return nullptr;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DTexIndexAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DTexIndexAnimationInstance>(animLoaded);

    return animInstance;
}

std::shared_ptr<J3DAnimation::J3DTexIndexAnimationInstance> LoadBtp(std::string path){
    if(!init) return nullptr;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DTexIndexAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DTexIndexAnimationInstance>(path);

    return animInstance;
}

void attachBtk(std::shared_ptr<J3DModelInstance> instance, py::bytes data){
    if(!init) return;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DTexMatrixAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DTexMatrixAnimationInstance>(animLoaded);

    instance->SetTexMatrixAnimation(animInstance);
}

void attachBtk(std::shared_ptr<J3DModelInstance> instance, std::string path){
    if(!init) return;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DTexMatrixAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DTexMatrixAnimationInstance>(path);

    instance->SetTexMatrixAnimation(animInstance);
}

void attachBtk(std::shared_ptr<J3DModelInstance> instance, std::shared_ptr<J3DAnimation::J3DTexMatrixAnimationInstance> anim){
    if(!init) return;
    instance->SetTexMatrixAnimation(anim);
}

std::shared_ptr<J3DAnimation::J3DTexMatrixAnimationInstance> LoadBtk(py::bytes data){
    if(!init) return nullptr;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DTexMatrixAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DTexMatrixAnimationInstance>(animLoaded);

    return animInstance;
}

std::shared_ptr<J3DAnimation::J3DTexMatrixAnimationInstance> LoadBtk(std::string path){
    if(!init) return nullptr;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DTexMatrixAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DTexMatrixAnimationInstance>(path);

    return animInstance;
}

void attachBck(std::shared_ptr<J3DModelInstance> instance, py::bytes data){
    if(!init) return;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DJointAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DJointAnimationInstance>(animLoaded);

    instance->SetJointAnimation(animInstance);
}

void attachBck(std::shared_ptr<J3DModelInstance> instance, std::string path){
    if(!init) return;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DJointAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DJointAnimationInstance>(path);

    instance->SetJointAnimation(animInstance);
}

void attachBck(std::shared_ptr<J3DModelInstance> instance, std::shared_ptr<J3DAnimation::J3DJointAnimationInstance> anim){
    if(!init) return;
    instance->SetJointAnimation(anim);
}

void attachBca(std::shared_ptr<J3DModelInstance> instance, py::bytes data){
    if(!init) return;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DJointFullAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DJointFullAnimationInstance>(animLoaded);

    instance->SetJointFullAnimation(animInstance);
}

void attachBca(std::shared_ptr<J3DModelInstance> instance, std::string path){
    if(!init) return;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DJointFullAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DJointFullAnimationInstance>(path);

    instance->SetJointFullAnimation(animInstance);
}

void attachBca(std::shared_ptr<J3DModelInstance> instance, std::shared_ptr<J3DAnimation::J3DJointFullAnimationInstance> anim){
    if(!init) return;
    instance->SetJointFullAnimation(anim);
}

std::shared_ptr<J3DAnimation::J3DJointFullAnimationInstance> LoadBca(py::bytes data){
    if(!init) return nullptr;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DJointFullAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DJointFullAnimationInstance>(animLoaded);

    return animInstance;
}

std::shared_ptr<J3DAnimation::J3DJointFullAnimationInstance> LoadBca(std::string path){
    if(!init) return nullptr;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DJointFullAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DJointFullAnimationInstance>(path);

    return animInstance;
}

void attachBva(std::shared_ptr<J3DModelInstance> instance, py::bytes data){
    if(!init) return;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DVisibilityAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DVisibilityAnimationInstance>(animLoaded);

    instance->SetVisibilityAnimation(animInstance);
}

void attachBva(std::shared_ptr<J3DModelInstance> instance, std::string path){
    if(!init) return;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DVisibilityAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DVisibilityAnimationInstance>(path);

    instance->SetVisibilityAnimation(animInstance);
}

void attachBva(std::shared_ptr<J3DModelInstance> instance, std::shared_ptr<J3DAnimation::J3DVisibilityAnimationInstance> anim){
    if(!init) return;
    instance->SetVisibilityAnimation(anim);
}

std::shared_ptr<J3DAnimation::J3DVisibilityAnimationInstance> LoadBva(py::bytes data){
    if(!init) return nullptr;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DVisibilityAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DVisibilityAnimationInstance>(animLoaded);

    return animInstance;
}

std::shared_ptr<J3DAnimation::J3DVisibilityAnimationInstance> LoadBva(std::string path){
    if(!init) return nullptr;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DVisibilityAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DVisibilityAnimationInstance>(path);

    return animInstance;
}

std::shared_ptr<J3DAnimation::J3DJointAnimationInstance> LoadBck(py::bytes data){
    if(!init) return nullptr;

    py::buffer_info dataInfo(py::buffer(data).request());

    J3DAnimation::J3DAnimationLoader Loader;
    std::shared_ptr<J3DAnimation::J3DAnimationInstance> animLoaded = Loader.LoadAnimation(dataInfo.ptr, dataInfo.size);
    std::shared_ptr<J3DAnimation::J3DJointAnimationInstance> animInstance = std::dynamic_pointer_cast<J3DAnimation::J3DJointAnimationInstance>(animLoaded);

    return animInstance;
}

std::shared_ptr<J3DAnimation::J3DJointAnimationInstance> LoadBck(std::string path){
    if(!init) return nullptr;

    J3DAnimation::J3DAnimationLoader Loader;
    
    std::shared_ptr<J3DAnimation::J3DJointAnimationInstance> animInstance = Loader.LoadAnimation<J3DAnimation::J3DJointAnimationInstance>(path);

    return animInstance;
}

void setLight(std::shared_ptr<J3DModelInstance> instance, J3DLight light, int lightIdx){
    instance->SetLight(light, lightIdx);
}

bool isClicked(std::shared_ptr<J3DModelInstance> instance, uint32_t x, uint32_t y){
    if(J3D::Picking::IsPickingEnabled()) return std::get<0>(J3D::Picking::Query(x,y)) == instance->GetModelId();
}

std::tuple<uint16_t, uint16_t> QueryPicking(uint32_t x, uint32_t y){
    if(J3D::Picking::IsPickingEnabled()) return J3D::Picking::Query(x,y);
}

void InitPicking(uint32_t w, uint32_t h){
    J3D::Picking::InitFramebuffer(w, h);
}

void ResizePickingFB(uint32_t w, uint32_t h){
    if(J3D::Picking::IsPickingEnabled()) J3D::Picking::ResizeFramebuffer(w, h);
}

void RenderScene(float dt, std::array<float, 3> cameraPos, bool renderPicking = false){
    if(init){
        auto sortedPackets = J3D::Rendering::SortPackets(renderBatch, glm::vec3(cameraPos.at(0), cameraPos.at(1), cameraPos.at(2))); 
        J3D::Rendering::Render(dt, viewMtx, projMtx, sortedPackets);
        
        if(J3D::Picking::IsPickingEnabled() && renderPicking) J3D::Picking::RenderPickingScene(viewMtx, projMtx, sortedPackets);

        renderBatch.clear();
    }
}

PYBIND11_MODULE(J3DUltra, m) {
    m.doc() = "J3DUltra";

    py::class_<glm::vec3>(m, "Vec3")
        .def(py::init([](){ return glm::vec3(0.0); }))
        .def(py::init([](float x, float y, float z){ return glm::vec3(x,y,z);} ))
        .def_readwrite("x", &glm::vec3::x)
        .def_readwrite("y", &glm::vec3::y)
        .def_readwrite("z", &glm::vec3::z)
        .def_readwrite("r", &glm::vec3::r)
        .def_readwrite("g", &glm::vec3::g)
        .def_readwrite("b", &glm::vec3::b);

    py::class_<glm::vec4>(m, "Vec4")
        .def(py::init([](){ return glm::vec4(0.0); }))
        .def(py::init([](float x, float y, float z, float w){ return glm::vec4(x,y,z,w);} ))
        .def_readwrite("x", &glm::vec4::x)
        .def_readwrite("y", &glm::vec4::y)
        .def_readwrite("z", &glm::vec4::z)
        .def_readwrite("w", &glm::vec4::w)
        .def_readwrite("r", &glm::vec4::r)
        .def_readwrite("g", &glm::vec4::g)
        .def_readwrite("b", &glm::vec4::b)
        .def_readwrite("a", &glm::vec4::a);

    py::class_<J3DLight>(m, "J3DLight")
        .def(py::init<>())
        .def(py::init([](std::array<float, 3> position, std::array<float, 3> direction, std::array<float, 4> color, std::array<float, 3> angle_atten, std::array<float, 3> dist_atten, bool followCamera){
            J3DLight light;

            light.Position = glm::vec4(position[0], position[1], position[2], followCamera ? 0 : 1);
            light.Direction = glm::vec4(direction[0], direction[1], direction[2], 1);
            light.Color = glm::vec4(color[0], color[1], color[2], color[3]);
            light.AngleAtten = glm::vec4(angle_atten[0], angle_atten[1], angle_atten[2], 1);;
            light.DistAtten = glm::vec4(dist_atten[0], dist_atten[1], dist_atten[2], 1);;

            return light;
        }))
        .def_readwrite("position", &J3DLight::Position)
        .def_readwrite("direction", &J3DLight::Direction)
        .def_readwrite("color", &J3DLight::Color)
        .def_readwrite("dist_atten", &J3DLight::DistAtten)
        .def_readwrite("angle_atten", &J3DLight::AngleAtten);

    py::class_<J3DModelData, std::shared_ptr<J3DModelData>>(m, "J3DModelData")
        .def(py::init<>())
        .def("createInstance", &J3DModelData::CreateInstance);

    py::class_<J3DAnimation::J3DAnimationInstance, std::shared_ptr<J3DAnimation::J3DAnimationInstance>>(m, "J3DAnimation")
        .def("setFrame", &J3DAnimation::J3DAnimationInstance::SetFrame)
        .def("getFrame", &J3DAnimation::J3DAnimationInstance::GetFrame)
        .def("setPaused", &J3DAnimation::J3DAnimationInstance::SetPaused)
        .def("tick", &J3DAnimation::J3DAnimationInstance::Tick);

    py::class_<J3DAnimation::J3DColorAnimationInstance, std::shared_ptr<J3DAnimation::J3DColorAnimationInstance>, J3DAnimation::J3DAnimationInstance>(m, "J3DColorAnimation")
        .def(py::init<>());

    py::class_<J3DAnimation::J3DTexIndexAnimationInstance, std::shared_ptr<J3DAnimation::J3DTexIndexAnimationInstance>, J3DAnimation::J3DAnimationInstance>(m, "J3DTexIndexAnimation")
        .def(py::init<>());

    py::class_<J3DAnimation::J3DTexMatrixAnimationInstance, std::shared_ptr<J3DAnimation::J3DTexMatrixAnimationInstance>, J3DAnimation::J3DAnimationInstance>(m, "J3DTexMatrixAnimation")
        .def(py::init<>());

    py::class_<J3DAnimation::J3DJointAnimationInstance, std::shared_ptr<J3DAnimation::J3DJointAnimationInstance>, J3DAnimation::J3DAnimationInstance>(m, "J3DJointAnimation")
        .def(py::init<>());

    py::class_<J3DAnimation::J3DJointFullAnimationInstance, std::shared_ptr<J3DAnimation::J3DJointFullAnimationInstance>, J3DAnimation::J3DAnimationInstance>(m, "J3DJointFullAnimation")
        .def(py::init<>());

    py::class_<J3DAnimation::J3DVisibilityAnimationInstance, std::shared_ptr<J3DAnimation::J3DVisibilityAnimationInstance>, J3DAnimation::J3DAnimationInstance>(m, "J3DVisibilityAnimation")
        .def(py::init<>());

    py::class_<J3DModelInstance, std::shared_ptr<J3DModelInstance>>(m, "J3DModelInstance")
        .def(py::init<std::shared_ptr<J3DModelData>, uint16_t>())
        .def("render", &renderModel)
        .def("setLight", &setLight, "Set Scene Light for J3D Render Functions")
        // These don't work yet
        .def("setTranslation", &setTranslation)
        .def("setRotation", &setRotation)
        .def("setScale", &setScale)
        .def("isClicked", &isClicked)
        .def("attachBrk", py::overload_cast<std::shared_ptr<J3DModelInstance>, py::bytes>(&attachBrk), py::kw_only(), py::arg("data"))
        .def("attachBrk", py::overload_cast<std::shared_ptr<J3DModelInstance>, std::string>(&attachBrk), py::kw_only(), py::arg("path"))
        .def("attachBrk", &J3DModelInstance::SetRegisterColorAnimation, py::kw_only(), py::arg("anim"))
        .def("getBrk", &J3DModelInstance::GetRegisterColorAnimation)
        .def("attachBtp", py::overload_cast<std::shared_ptr<J3DModelInstance>, py::bytes>(&attachBtp), py::kw_only(), py::arg("data"))
        .def("attachBtp", py::overload_cast<std::shared_ptr<J3DModelInstance>, std::string>(&attachBtp), py::kw_only(), py::arg("path"))
        .def("attachBtp", &J3DModelInstance::SetTexIndexAnimation, py::kw_only(), py::arg("anim"))
        .def("getBtp", &J3DModelInstance::GetTexIndexAnimation)
        .def("attachBtk", py::overload_cast<std::shared_ptr<J3DModelInstance>, py::bytes>(&attachBtk), py::kw_only(), py::arg("data"))
        .def("attachBtk", py::overload_cast<std::shared_ptr<J3DModelInstance>, std::string>(&attachBtk), py::kw_only(), py::arg("path"))
        .def("attachBtk", &J3DModelInstance::SetTexMatrixAnimation, py::kw_only(), py::arg("anim"))
        .def("getBtk", &J3DModelInstance::GetTexMatrixAnimation)
        .def("attachBck", py::overload_cast<std::shared_ptr<J3DModelInstance>, py::bytes>(&attachBck), py::kw_only(), py::arg("data"))
        .def("attachBck", py::overload_cast<std::shared_ptr<J3DModelInstance>, std::string>(&attachBck), py::kw_only(), py::arg("path"))
        .def("attachBck", &J3DModelInstance::SetJointAnimation, py::kw_only(), py::arg("anim"))
        .def("getBck", &J3DModelInstance::GetJointAnimation)
        .def("attachBca", py::overload_cast<std::shared_ptr<J3DModelInstance>, py::bytes>(&attachBca), py::kw_only(), py::arg("data"))
        .def("attachBca", py::overload_cast<std::shared_ptr<J3DModelInstance>, std::string>(&attachBca), py::kw_only(), py::arg("path"))
        .def("attachBca", &J3DModelInstance::SetJointFullAnimation, py::kw_only(), py::arg("anim"))
        .def("getBca", &J3DModelInstance::GetJointFullAnimation)
        .def("attachBva", py::overload_cast<std::shared_ptr<J3DModelInstance>, py::bytes>(&attachBva), py::kw_only(), py::arg("data"))
        .def("attachBva", py::overload_cast<std::shared_ptr<J3DModelInstance>, std::string>(&attachBva), py::kw_only(), py::arg("path"))
        .def("attachBva", &J3DModelInstance::SetVisibilityAnimation, py::kw_only(), py::arg("anim"))
        .def("getBva", &J3DModelInstance::GetVisibilityAnimation)
        ;
    
    m.def("loadModel", py::overload_cast<std::string>(&LoadJ3DModel), "Load BMD/BDL from filepath", py::kw_only(), py::arg("path"));
    m.def("loadModel", py::overload_cast<py::bytes>(&LoadJ3DModel), "Load BMD/BDL from bytes object", py::kw_only(), py::arg("data"));
    
    m.def("loadBrk", py::overload_cast<std::string>(&LoadBrk), "Load BRK from filepath", py::kw_only(), py::arg("path"));
    m.def("loadBrk", py::overload_cast<py::bytes>(&LoadBrk), "Load BRK from bytes object", py::kw_only(), py::arg("data"));
    m.def("loadBtp", py::overload_cast<std::string>(&LoadBtp), "Load BTP from filepath", py::kw_only(), py::arg("path"));
    m.def("loadBtp", py::overload_cast<py::bytes>(&LoadBtp), "Load BTP from bytes object", py::kw_only(), py::arg("data"));
    m.def("loadBtk", py::overload_cast<std::string>(&LoadBtk), "Load BTK from filepath", py::kw_only(), py::arg("path"));
    m.def("loadBtk", py::overload_cast<py::bytes>(&LoadBtk), "Load BTK from bytes object", py::kw_only(), py::arg("data"));
    m.def("loadBck", py::overload_cast<std::string>(&LoadBck), "Load BCK from filepath", py::kw_only(), py::arg("path"));
    m.def("loadBck", py::overload_cast<py::bytes>(&LoadBck), "Load BCK from bytes object", py::kw_only(), py::arg("data"));
    m.def("loadBca", py::overload_cast<std::string>(&LoadBca), "Load BCA from filepath", py::kw_only(), py::arg("path"));
    m.def("loadBca", py::overload_cast<py::bytes>(&LoadBca), "Load BCA from bytes object", py::kw_only(), py::arg("data"));
    m.def("loadBva", py::overload_cast<std::string>(&LoadBva), "Load BVA from filepath", py::kw_only(), py::arg("path"));
    m.def("loadBva", py::overload_cast<py::bytes>(&LoadBva), "Load BVA from bytes object", py::kw_only(), py::arg("data"));
    
    m.def("init", &InitJ3DUltra, "Setup J3DUltra for Model Loading and Rendering");
    m.def("cleanup", &CleanupJ3DUltra, "Cleanup J3DUltra Library");
    m.def("setCamera", &SetCamera, "Set Projection and View Matrices to render with");
    
    m.def("render", &RenderScene, "Execute all pending model renders");

    m.def("resizePicking", &ResizePickingFB, "");
    m.def("queryPicking", &QueryPicking, "");
    m.def("initPicking", &InitPicking, "");
}